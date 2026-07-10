using UnrealBuildTool;

public class EmberEditor : ModuleRules
{
    public EmberEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new[] {
            "Core", "CoreUObject", "Engine", "UnrealEd", "AssetRegistry", "Projects", "AddContentDialog", "GameplayTags",
            "EmberCore", "EmberGameplay", "EmberAI", "EmberMission"
        });
    }
}
