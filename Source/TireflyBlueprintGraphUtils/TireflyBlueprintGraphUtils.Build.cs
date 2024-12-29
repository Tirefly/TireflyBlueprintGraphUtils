// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TireflyBlueprintGraphUtils : ModuleRules
{
	public TireflyBlueprintGraphUtils(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
		
		
		// Public Dependency Modules
		PublicDependencyModuleNames.Add("Core");
		
		
		// Private Dependency Modules
		PrivateDependencyModuleNames.Add("CoreUObject");
		PrivateDependencyModuleNames.Add("Engine");
		PrivateDependencyModuleNames.Add("Slate");
		PrivateDependencyModuleNames.Add("SlateCore");
		PrivateDependencyModuleNames.Add("BlueprintGraph");
		PrivateDependencyModuleNames.Add("UnrealEd");
		PrivateDependencyModuleNames.Add("GraphEditor");
		PrivateDependencyModuleNames.Add("InputCore");
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
