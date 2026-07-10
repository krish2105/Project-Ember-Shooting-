using UnrealBuildTool;
using System.Collections.Generic;

public class ProjectEmberTarget : TargetRules
{
    public ProjectEmberTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.AddRange(new[] { "EmberCore", "EmberGameplay", "EmberAI", "EmberMission", "EmberUI" });
    }
}
