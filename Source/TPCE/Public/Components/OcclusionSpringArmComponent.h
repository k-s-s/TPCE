// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "Components/ReboundSpringArmComponent.h"
#include "UObject/ObjectMacros.h"
#include "Engine/EngineTypes.h"
#include "Components/SceneComponent.h"

#include "OcclusionSpringArmComponent.generated.h"

class UMaterialInterface;
class UPrimitiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOcclusionSpringArmOcclude, UPrimitiveComponent*, OccludingComponent);
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FOcclusionSpringArmTest, UPrimitiveComponent*, OccludingComponent);

/**
 * This component tries to maintain its children at a fixed distance from the parent,
 * but will retract the children if there is a collision, and spring back when there is no collision.
 * Additionally, it can detect and react to meshes occluding the view.
 *
 * Example: Use as a 'camera boom' to keep the follow camera for a player from passing through the world.
 */
UCLASS(ClassGroup=Camera, meta=(BlueprintSpawnableComponent))
class TPCE_API UOcclusionSpringArmComponent : public UReboundSpringArmComponent
{
	GENERATED_BODY()

public:

	UOcclusionSpringArmComponent();

	// Begin UActorComponent Interface
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// End UActorComponent Interface

	/** Whether occlusion checks should be performed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Occlusion")
	bool bEnableOcclusion;

	/** Collision profile of the occlusion query probe. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Occlusion", meta=(editcondition="bEnableOcclusion"))
	FName OcclusionProfileName;

	/** Half extents of the occlusion query probe. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Occlusion", meta=(editcondition="bEnableOcclusion"))
	FVector OcclusionExtents;

	/** Occlusion query probe offset in space of the arm component. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Occlusion", meta=(editcondition="bEnableOcclusion"))
	FVector OcclusionOffset;

	/** Occlusion query probe roll offset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Occlusion", meta=(editcondition="bEnableOcclusion"))
	float OcclusionRoll;

	/** Only consider unwalkable primitives for occlusion. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Occlusion", meta=(editcondition="bEnableOcclusion"))
	bool bFilterWalkable;

	/** Whether occlusion should test for complex geometry. */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Occlusion", meta=(editcondition="bEnableOcclusion"))
	//bool bOcclusionTraceComplex;

	/** Material that meshes should switch to while occluded. */
	UPROPERTY(EditDefaultsOnly, Category="Camera Occlusion", meta=(editcondition="bEnableOcclusion"))
	UMaterialInterface* MaskedMaterial;

	/** Index of the custom primitive data slot that holds the time when occlusion started. */
	UPROPERTY(EditDefaultsOnly, AdvancedDisplay, Category="Camera Occlusion", meta=(editcondition="bEnableOcclusion"))
	int32 MaskedMaterialTimeDataIndex;

	/** Shows debug helpers for camera occlusion. */
	UPROPERTY(EditDefaultsOnly, AdvancedDisplay, Category="Camera Occlusion", meta=(editcondition="bEnableOcclusion"))
	uint32 bDrawDebugOcclusionBox : 1;

	/** Event called a mesh comes between the target and camera. */
	UPROPERTY(BlueprintAssignable, Category="Camera Occlusion")
	FOcclusionSpringArmOcclude OnBeginOcclude;

	/** Event called a mesh is no longer between the target and camera. */
	UPROPERTY(BlueprintAssignable, Category="Camera Occlusion")
	FOcclusionSpringArmOcclude OnEndOcclude;

	/** Called for every mesh that is found to be occluding the view. Return True to ignore the mesh. */
	virtual bool TestOcclude(UPrimitiveComponent* OccludingComponent);
	virtual void BeginOcclude(UPrimitiveComponent* OccludingComponent);
	virtual void EndOcclude(UPrimitiveComponent* OccludingComponent);

private:

	void UpdateOccludingComponents();
	TSet<TWeakObjectPtr<UPrimitiveComponent>> OccludingComps;
	TSet<TWeakObjectPtr<UPrimitiveComponent>> NewOccludingComps;
};
