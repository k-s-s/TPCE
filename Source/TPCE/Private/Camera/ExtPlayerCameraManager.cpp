// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "Camera/ExtPlayerCameraManager.h"

float AExtPlayerCameraManager::GetBlendTimeToGo() const
{
	return BlendTimeToGo;
}

float AExtPlayerCameraManager::GetBlendTimePercent() const
{
	if (BlendParams.BlendTime > 0.0f)
	{
		return (BlendParams.BlendTime - BlendTimeToGo) / BlendParams.BlendTime;
	}
	return 1.0f;
}

FVector AExtPlayerCameraManager::GetFocalPointOnActor(const AActor* Actor) const
{
	return Actor != nullptr ? Actor->GetTargetLocation() : FVector::ZeroVector;
}
