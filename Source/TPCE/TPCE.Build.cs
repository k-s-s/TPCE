// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

using UnrealBuildTool;

public class TPCE: ModuleRules
{
	public TPCE(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnforceIWYU = true;

		PrivateIncludePaths.AddRange(
			new string[]
			{
                "TPCE/Private"
            }
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"AnimationCore",
				"AnimGraphRuntime",
				"InputCore",
				"AIModule",
				"GameplayTasks",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// UI
				"Slate",
				"SlateCore",
				"UMG",
				"RHI", // Needed for GetMax2DTextureDimension
            }
		);
	}
}
