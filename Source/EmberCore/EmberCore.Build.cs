using UnrealBuildTool;

public class EmberCore : ModuleRules
{
    public EmberCore(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "GameplayTags", "DeveloperSettings" });
    }
}

