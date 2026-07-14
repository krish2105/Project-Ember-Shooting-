using UnrealBuildTool;

public class EmberUI : ModuleRules
{
    public EmberUI(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] {
            "Core", "CoreUObject", "Engine", "UMG", "Slate", "SlateCore",
            "EnhancedInput", "EmberCore", "EmberGameplay"
        });
    }
}
