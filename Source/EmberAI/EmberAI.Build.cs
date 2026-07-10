using UnrealBuildTool;

public class EmberAI : ModuleRules
{
    public EmberAI(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] {
            "Core", "CoreUObject", "Engine", "AIModule", "GameplayTasks",
            "NavigationSystem", "GameplayTags", "EmberCore", "EmberGameplay"
        });
    }
}

