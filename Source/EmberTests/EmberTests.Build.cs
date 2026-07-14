using UnrealBuildTool;

public class EmberTests : ModuleRules
{
    public EmberTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new[] {
            "Core", "CoreUObject", "Engine", "InputCore", "EmberCore", "EmberGameplay", "EmberAI", "EmberMission", "EmberUI"
        });
    }
}
