using UnrealBuildTool;

public class EmberGameplay : ModuleRules
{
    public EmberGameplay(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] {
            "Core", "CoreUObject", "Engine", "AudioExtensions", "InputCore", "EnhancedInput",
            "GameplayTags", "Niagara", "ChaosModularVehicleEngine", "EmberCore"
        });
    }
}
