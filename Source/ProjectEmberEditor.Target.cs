using UnrealBuildTool;
using System.Collections.Generic;

public class ProjectEmberEditorTarget : TargetRules
{
    public ProjectEmberEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.AddRange(new[] { "EmberCore", "EmberGameplay", "EmberAI", "EmberMission", "EmberUI", "EmberEditor", "EmberTests" });
    }
}
