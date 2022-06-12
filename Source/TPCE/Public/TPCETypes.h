// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/EngineTypes.h"
#include "Math/Bounds.h"

#include "TPCETypes.generated.h"

// Epic Skeleton Bone Names

extern TPCE_API const FName NAME_RootBone;
extern TPCE_API const FName NAME_Pelvis;
extern TPCE_API const FName NAME_Spine_01;
extern TPCE_API const FName NAME_Spine_02;
extern TPCE_API const FName NAME_Spine_03;

extern TPCE_API const FName NAME_Clavicle_L;
extern TPCE_API const FName NAME_UpperArm_L;
extern TPCE_API const FName NAME_LowerArm_L;
extern TPCE_API const FName NAME_UpperArmTwist_L;
extern TPCE_API const FName NAME_LowerArmTwist_L;
extern TPCE_API const FName NAME_Hand_L;

extern TPCE_API const FName NAME_IndexFinger_01_L;
extern TPCE_API const FName NAME_IndexFinger_02_L;
extern TPCE_API const FName NAME_IndexFinger_03_L;
extern TPCE_API const FName NAME_MiddleFinger_01_L;
extern TPCE_API const FName NAME_MiddleFinger_02_L;
extern TPCE_API const FName NAME_MiddleFinger_03_L;
extern TPCE_API const FName NAME_PinkyFinger_01_L;
extern TPCE_API const FName NAME_PinkyFinger_02_L;
extern TPCE_API const FName NAME_PinkyFinger_03_L;
extern TPCE_API const FName NAME_RingFinger_01_L;
extern TPCE_API const FName NAME_RingFinger_02_L;
extern TPCE_API const FName NAME_RingFinger_03_L;
extern TPCE_API const FName NAME_Thumb_01_L;
extern TPCE_API const FName NAME_Thumb_02_L;
extern TPCE_API const FName NAME_Thumb_03_L;

extern TPCE_API const FName NAME_Clavicle_R;
extern TPCE_API const FName NAME_UpperArm_R;
extern TPCE_API const FName NAME_LowerArm_R;
extern TPCE_API const FName NAME_UpperArmTwist_R;
extern TPCE_API const FName NAME_LowerArmTwist_R;
extern TPCE_API const FName NAME_Hand_R;

extern TPCE_API const FName NAME_IndexFinger_01_R;
extern TPCE_API const FName NAME_IndexFinger_02_R;
extern TPCE_API const FName NAME_IndexFinger_03_R;
extern TPCE_API const FName NAME_MiddleFinger_01_R;
extern TPCE_API const FName NAME_MiddleFinger_02_R;
extern TPCE_API const FName NAME_MiddleFinger_03_R;
extern TPCE_API const FName NAME_PinkyFinger_01_R;
extern TPCE_API const FName NAME_PinkyFinger_02_R;
extern TPCE_API const FName NAME_PinkyFinger_03_R;
extern TPCE_API const FName NAME_RingFinger_01_R;
extern TPCE_API const FName NAME_RingFinger_02_R;
extern TPCE_API const FName NAME_RingFinger_03_R;
extern TPCE_API const FName NAME_Thumb_01_R;
extern TPCE_API const FName NAME_Thumb_02_R;
extern TPCE_API const FName NAME_Thumb_03_R;

extern TPCE_API const FName NAME_Neck_01;
extern TPCE_API const FName NAME_Head;

extern TPCE_API const FName NAME_Thigh_L;
extern TPCE_API const FName NAME_Calf_L;
extern TPCE_API const FName NAME_ThighTwist_L;
extern TPCE_API const FName NAME_CalfTwist_L;
extern TPCE_API const FName NAME_Foot_L;
extern TPCE_API const FName NAME_Ball_L;

extern TPCE_API const FName NAME_Thigh_R;
extern TPCE_API const FName NAME_Calf_R;
extern TPCE_API const FName NAME_ThighTwist_R;
extern TPCE_API const FName NAME_CalfTwist_R;
extern TPCE_API const FName NAME_Foot_R;
extern TPCE_API const FName NAME_Ball_R;

extern TPCE_API const FName NAME_IKFootRoot;
extern TPCE_API const FName NAME_IKFoot_L;
extern TPCE_API const FName NAME_IKFoot_R;

extern TPCE_API const FName NAME_IKHandRoot;
extern TPCE_API const FName NAME_IKHand_Gun;
extern TPCE_API const FName NAME_IKHand_L;
extern TPCE_API const FName NAME_IKHand_R;

extern TPCE_API const FName NAME_Weapon_L;
extern TPCE_API const FName NAME_Weapon_R;

/** */
UENUM(BlueprintType)
enum class ELongitudinalDirection : uint8
{
	Forward,
	Backward
};

/** */
UENUM(BlueprintType)
enum class ECardinalDirection : uint8
{
	North,
	South,
	East,
	West
};
