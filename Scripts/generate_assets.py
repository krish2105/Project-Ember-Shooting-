#!/usr/bin/env python3
"""Generate Project Ember environment props with Meshy Text-to-3D v2.

The command is deliberately dry-run by default. Real API calls require both
``--execute`` and a ``MESHY_API_KEY`` environment variable so an accidental
invocation cannot spend credits. Task identifiers are persisted after every
request, allowing an interrupted run to resume without creating duplicate
jobs.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import sys
import tempfile
import time
import urllib.error
import urllib.request
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict, Iterable, Mapping, Optional


API_ROOT = "https://api.meshy.ai/openapi/v2/text-to-3d"
TERMINAL_STATES = {"SUCCEEDED", "FAILED", "CANCELED"}
REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_OUTPUT = REPO_ROOT / "Assets" / "Imported"


@dataclass(frozen=True)
class AssetRequest:
    name: str
    prompt: str
    texture_prompt: str
    target_polycount: int


ASSETS = (
    AssetRequest(
        name="ruined_concrete_pillar",
        prompt=(
            "Single isolated game-ready decayed neo-classical waterfront pillar, "
            "fractured weathered limestone and reinforced concrete, chipped fluted "
            "shaft, broken capital and base, deep cracks, exposed bent rebar, salt "
            "staining and impact damage. Mid-poly realistic proportions, watertight "
            "geometry, clean silhouette, flat underside, no ground, no scene, no text."
        ),
        texture_prompt=(
            "Photoreal cold gray weathered limestone and concrete, dark cavity grime, "
            "rust stains around exposed rebar, chipped pale aggregate, subtle dampness; "
            "neutral baked-free PBR response without dramatic lighting."
        ),
        target_polycount=12000,
    ),
    AssetRequest(
        name="industrial_crane_ruin",
        prompt=(
            "Single isolated ruined dockside construction crane, corroded steel lattice "
            "mast and broken truss jib, bent structural members, damaged machinery base, "
            "hanging hook and short heavy cable, war-torn harbor salvage. Game-ready "
            "mid-poly topology, readable silhouette, stable base, no ground, no scene, no text."
        ),
        texture_prompt=(
            "Photoreal blackened painted steel with flaking desaturated yellow paint, "
            "orange-brown rust, soot, salt corrosion, oily machinery grime and scratched "
            "edges; neutral baked-free PBR response."
        ),
        target_polycount=30000,
    ),
    AssetRequest(
        name="rubble_pile_debris",
        prompt=(
            "Single isolated low rubble mound for a war-damaged neo-classical city and "
            "harbor, scattered fractured limestone blocks, concrete chunks, red bricks, "
            "small asphalt pieces and several bent steel rebar rods. Game-ready mid-poly "
            "cluster, irregular silhouette, flat underside, no ground plane, no scene, no text."
        ),
        texture_prompt=(
            "Photoreal dusty gray concrete and pale limestone mixed with muted red brick, "
            "dark soot, rusted rebar, damp dirt in crevices and chipped aggregate; neutral "
            "baked-free PBR response."
        ),
        target_polycount=16000,
    ),
)


class MeshyError(RuntimeError):
    """A recoverable or terminal Meshy API error."""


def atomic_write_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle, temporary_name = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    try:
        with os.fdopen(handle, "w", encoding="utf-8") as stream:
            json.dump(value, stream, indent=2, sort_keys=True)
            stream.write("\n")
        os.replace(temporary_name, path)
    finally:
        if os.path.exists(temporary_name):
            os.unlink(temporary_name)


def load_json(path: Path, fallback: Any) -> Any:
    if not path.exists():
        return fallback
    with path.open("r", encoding="utf-8") as stream:
        return json.load(stream)


class MeshyClient:
    def __init__(self, api_key: str, poll_seconds: float, timeout_minutes: float) -> None:
        self._api_key = api_key
        self._poll_seconds = poll_seconds
        self._timeout_seconds = timeout_minutes * 60.0

    def request_json(
        self,
        method: str,
        url: str,
        payload: Optional[Mapping[str, Any]] = None,
        attempts: int = 5,
    ) -> Dict[str, Any]:
        body = None if payload is None else json.dumps(payload).encode("utf-8")
        headers = {
            "Authorization": f"Bearer {self._api_key}",
            "Accept": "application/json",
        }
        if body is not None:
            headers["Content-Type"] = "application/json"

        for attempt in range(attempts):
            request = urllib.request.Request(url, data=body, headers=headers, method=method)
            try:
                with urllib.request.urlopen(request, timeout=90) as response:
                    result = json.loads(response.read().decode("utf-8"))
                    if not isinstance(result, dict):
                        raise MeshyError(f"Unexpected non-object response from {url}")
                    return result
            except urllib.error.HTTPError as error:
                detail = error.read().decode("utf-8", errors="replace")[:1000]
                if error.code in {401, 402, 403}:
                    raise MeshyError(
                        f"Meshy rejected the request with HTTP {error.code}. "
                        "Check authentication, permissions and account credits."
                    ) from error
                if error.code == 429 or 500 <= error.code < 600:
                    if attempt + 1 < attempts:
                        retry_after = error.headers.get("Retry-After")
                        delay = float(retry_after) if retry_after else min(30.0, 2.0 ** attempt)
                        time.sleep(delay)
                        continue
                raise MeshyError(f"Meshy HTTP {error.code}: {detail}") from error
            except (urllib.error.URLError, TimeoutError) as error:
                if attempt + 1 < attempts:
                    time.sleep(min(30.0, 2.0 ** attempt))
                    continue
                raise MeshyError(f"Meshy network request failed: {error}") from error
        raise MeshyError("Meshy request exhausted all retry attempts")

    def create_task(self, payload: Mapping[str, Any]) -> str:
        result = self.request_json("POST", API_ROOT, payload)
        task_id = result.get("result")
        if not isinstance(task_id, str) or not task_id:
            raise MeshyError(f"Meshy create response did not contain a task id: {result}")
        return task_id

    def wait_for_task(self, task_id: str, label: str) -> Dict[str, Any]:
        deadline = time.monotonic() + self._timeout_seconds
        last_status = ""
        while time.monotonic() < deadline:
            result = self.request_json("GET", f"{API_ROOT}/{task_id}")
            status = str(result.get("status", "UNKNOWN")).upper()
            if status != last_status:
                progress = result.get("progress", "?")
                print(f"[{label}] {status} ({progress}%)", flush=True)
                last_status = status
            if status in TERMINAL_STATES:
                if status != "SUCCEEDED":
                    message = result.get("task_error") or result.get("message") or "no details"
                    raise MeshyError(f"{label} ended as {status}: {message}")
                return result
            time.sleep(self._poll_seconds)
        raise MeshyError(f"Timed out waiting for {label} after {self._timeout_seconds / 60:.1f} minutes")


def preview_payload(asset: AssetRequest) -> Dict[str, Any]:
    return {
        "mode": "preview",
        "prompt": asset.prompt,
        "ai_model": "latest",
        "model_type": "standard",
        "should_remesh": True,
        "topology": "triangle",
        "target_polycount": asset.target_polycount,
        "target_formats": ["fbx"],
        "auto_size": True,
        "origin_at": "bottom",
    }


def refine_payload(asset: AssetRequest, preview_task_id: str) -> Dict[str, Any]:
    return {
        "mode": "refine",
        "preview_task_id": preview_task_id,
        "ai_model": "latest",
        "enable_pbr": True,
        "hd_texture": True,
        "remove_lighting": True,
        "texture_prompt": asset.texture_prompt,
        "target_formats": ["fbx"],
    }


def download_file(url: str, destination: Path) -> Dict[str, Any]:
    destination.parent.mkdir(parents=True, exist_ok=True)
    digest = hashlib.sha256()
    handle, temporary_name = tempfile.mkstemp(prefix=f".{destination.name}.", dir=destination.parent)
    size = 0
    try:
        request = urllib.request.Request(url, headers={"User-Agent": "ProjectEmberAssetPipeline/1.0"})
        with os.fdopen(handle, "wb") as stream:
            with urllib.request.urlopen(request, timeout=180) as response:
                while True:
                    chunk = response.read(1024 * 1024)
                    if not chunk:
                        break
                    stream.write(chunk)
                    digest.update(chunk)
                    size += len(chunk)
        os.replace(temporary_name, destination)
    finally:
        if os.path.exists(temporary_name):
            os.unlink(temporary_name)
    return {"path": destination.name, "bytes": size, "sha256": digest.hexdigest()}


def first_texture_set(result: Mapping[str, Any]) -> Mapping[str, Any]:
    texture_urls = result.get("texture_urls", [])
    if not isinstance(texture_urls, list) or not texture_urls or not isinstance(texture_urls[0], dict):
        raise MeshyError("Refine task succeeded without a PBR texture set")
    return texture_urls[0]


def download_asset(asset: AssetRequest, result: Mapping[str, Any], output: Path) -> Dict[str, Any]:
    model_urls = result.get("model_urls")
    if not isinstance(model_urls, dict) or not model_urls.get("fbx"):
        raise MeshyError(f"{asset.name} refine task succeeded without an FBX URL")

    textures = first_texture_set(result)
    required = {"base_color": "albedo", "normal": "normal", "roughness": "roughness"}
    missing = [key for key in required if not textures.get(key)]
    if missing:
        raise MeshyError(f"{asset.name} is missing required PBR maps: {', '.join(missing)}")

    sources = {f"{asset.name}.fbx": model_urls["fbx"]}
    for source_key, output_key in required.items():
        sources[f"{asset.name}_{output_key}.png"] = textures[source_key]
    if textures.get("metallic"):
        sources[f"{asset.name}_metallic.png"] = textures["metallic"]
    if result.get("thumbnail_url"):
        sources[f"{asset.name}_preview.png"] = result["thumbnail_url"]

    files = []
    for filename, url in sources.items():
        print(f"[{asset.name}] downloading {filename}", flush=True)
        files.append(download_file(str(url), output / filename))
    return {
        "name": asset.name,
        "preview_task_id": result.get("preview_task_id"),
        "refine_task_id": result.get("id"),
        "files": files,
    }


def select_assets(names: Iterable[str]) -> tuple[AssetRequest, ...]:
    requested = set(names)
    if not requested:
        return ASSETS
    known = {asset.name: asset for asset in ASSETS}
    unknown = sorted(requested - known.keys())
    if unknown:
        raise MeshyError(f"Unknown assets: {', '.join(unknown)}")
    return tuple(known[name] for name in names)


def print_dry_run(assets: Iterable[AssetRequest], output: Path) -> None:
    plan = {
        "mode": "dry-run",
        "api": API_ROOT,
        "output": str(output),
        "assets": [
            {
                "name": asset.name,
                "preview": preview_payload(asset),
                "refine_after_preview": refine_payload(asset, "<preview_task_id>"),
                "expected_files": [
                    f"{asset.name}.fbx",
                    f"{asset.name}_albedo.png",
                    f"{asset.name}_normal.png",
                    f"{asset.name}_roughness.png",
                ],
            }
            for asset in assets
        ],
    }
    print(json.dumps(plan, indent=2))


def execute(assets: Iterable[AssetRequest], output: Path, args: argparse.Namespace) -> None:
    api_key = os.environ.get("MESHY_API_KEY", "").strip()
    if not api_key:
        raise MeshyError("--execute requires MESHY_API_KEY in the environment; the key is never stored by this script")

    output.mkdir(parents=True, exist_ok=True)
    state_path = output / "meshy_tasks.json"
    manifest_path = output / "asset_manifest.json"
    state = load_json(state_path, {"schema_version": 1, "assets": {}})
    manifest = load_json(manifest_path, {"schema_version": 1, "generator": "Meshy Text-to-3D v2", "assets": {}})
    client = MeshyClient(api_key, args.poll_seconds, args.timeout_minutes)

    for asset in assets:
        asset_state = state["assets"].setdefault(asset.name, {})
        preview_id = asset_state.get("preview_task_id")
        if not preview_id:
            print(f"[{asset.name}] creating preview task", flush=True)
            preview_id = client.create_task(preview_payload(asset))
            asset_state["preview_task_id"] = preview_id
            atomic_write_json(state_path, state)
        preview = client.wait_for_task(preview_id, f"{asset.name}:preview")

        refine_id = asset_state.get("refine_task_id")
        if not refine_id:
            print(f"[{asset.name}] creating PBR refine task", flush=True)
            refine_id = client.create_task(refine_payload(asset, preview_id))
            asset_state["refine_task_id"] = refine_id
            atomic_write_json(state_path, state)
        refined = client.wait_for_task(refine_id, f"{asset.name}:refine")
        refined = dict(refined)
        refined.setdefault("preview_task_id", preview.get("id", preview_id))

        manifest["assets"][asset.name] = download_asset(asset, refined, output)
        asset_state["downloaded"] = True
        atomic_write_json(manifest_path, manifest)
        atomic_write_json(state_path, state)

    print(f"Generated asset manifest: {manifest_path}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--dry-run", action="store_true", help="Print requests without network calls (default)")
    mode.add_argument("--execute", action="store_true", help="Submit real Meshy jobs and download their results")
    parser.add_argument("--asset", action="append", default=[], help="Generate one named asset; may be repeated")
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT, help="Output directory")
    parser.add_argument("--poll-seconds", type=float, default=10.0, help="Seconds between task status checks")
    parser.add_argument("--timeout-minutes", type=float, default=45.0, help="Timeout for each Meshy stage")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    try:
        assets = select_assets(args.asset)
        output = args.output.expanduser().resolve()
        if args.execute:
            execute(assets, output, args)
        else:
            print_dry_run(assets, output)
        return 0
    except (MeshyError, json.JSONDecodeError, OSError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
