// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

using UnrealBuildTool;

public class TPCEUncooked : ModuleRules
{
	public TPCEUncooked(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnforceIWYU = true;

		PrivateIncludePaths.AddRange(
			new string[]
			{
				"TPCEUncooked/Private"
			});

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"UnrealEd",
			});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore",
				"EditorStyle",
				"UMG",
				"AnimGraph",
				"BlueprintGraph",
				"TPCE",
			});
	}
}
