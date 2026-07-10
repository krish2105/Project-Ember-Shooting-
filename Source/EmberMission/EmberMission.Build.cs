using UnrealBuildTool;

public class EmberMission : ModuleRules
{
    public EmberMission(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] {
            "Core", "CoreUObject", "Engine", "GameplayTags", "EmberCore", "EmberGameplay"
        });
    }
}

