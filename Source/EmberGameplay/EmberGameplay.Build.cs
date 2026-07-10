using UnrealBuildTool;

public class EmberGameplay : ModuleRules
{
    public EmberGameplay(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] {
            "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
            "GameplayTags", "Niagara", "EmberCore"
        });
    }
}

