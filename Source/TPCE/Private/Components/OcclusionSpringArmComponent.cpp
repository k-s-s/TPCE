// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "Components/OcclusionSpringArmComponent.h"

#include "CollisionQueryParams.h"
#include "WorldCollision.h"
#include "Engine/World.h"
#include "Engine/CollisionProfile.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

UOcclusionSpringArmComponent::UOcclusionSpringArmComponent()
	: bEnableOcclusion(false)
	, OcclusionProfileName(UCollisionProfile::Pawn_ProfileName)
	, OcclusionExtents(200.f, 100.f, 100.f)
	, OcclusionOffset(-300.f, 0.f, -50.f)
	, OcclusionRoll(45.f)
	, MaskedMaterialTimeDataIndex(0)
{
}

void UOcclusionSpringArmComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bEnableOcclusion)
	{
		// Trace before updating the transform to give the spring arm a one tick advantage
		UpdateOccludingComponents();
	}
	else if (OccludingComps.Num() > 0)
	{
		// No longer performing occlusion checks, clear components
		for (TWeakObjectPtr<UPrimitiveComponent> PrimCompPtr : OccludingComps.Difference(NewOccludingComps))
		{
			if (UPrimitiveComponent* PrimComp = PrimCompPtr.Get())
			{
				EndOcclude(PrimComp);
				OnEndOcclude.Broadcast(PrimComp);
			}
		}
		OccludingComps.Empty();
	}
}

void UOcclusionSpringArmComponent::UpdateOccludingComponents()
{
	auto TestAndAddComponentsFromHitResults = [this](const TArray<FOverlapResult>& Hits)
	{
		for (const FOverlapResult& Hit : Hits)
		{
			if (Hit.Component.IsValid() && !TestOcclude(Hit.Component.Get()))
			{
				NewOccludingComps.Add(Hit.Component);
			}
		}
	};

	// Find meshes between the camera and target
	const FRotator ArmRotation = GetTargetRotation();
	//const FVector ArmVector = FMath::IsNearlyZero(ArmComponent->TargetArmLength) ? FVector::ZeroVector : TraceRot.Vector() * ArmComponent->TargetArmLength;
	const FVector BoxPos = GetTargetLocation() + ArmRotation.RotateVector(OcclusionOffset);
	const FRotator BoxRot = FRotator(ArmRotation.Pitch, ArmRotation.Yaw, ArmRotation.Roll + OcclusionRoll);

#if ENABLE_DRAW_DEBUG
	if (bDrawDebugOcclusionBox)
	{
		UKismetSystemLibrary::DrawDebugBox(this, BoxPos, OcclusionExtents, FColor::Yellow, BoxRot);
	}
#endif

	TArray<FOverlapResult> Hits;
	UWorld* ThisWorld = GetWorld();
	if (ThisWorld)
	{
		static const FName NAME_CameraOcclusionTraceTag(TEXT("Camera Occlusion Trace"));
		const FCollisionShape TraceBox = FCollisionShape::MakeBox(OcclusionExtents);
		const bool bOcclusionTraceComplex = false;
		const FCollisionQueryParams Params(NAME_CameraOcclusionTraceTag, bOcclusionTraceComplex);
		if (ThisWorld->OverlapMultiByProfile(Hits, BoxPos, BoxRot.Quaternion(), OcclusionProfileName, TraceBox, Params))
		{
			TestAndAddComponentsFromHitResults(Hits);
		}
	}

	for (TWeakObjectPtr<UPrimitiveComponent> PrimCompPtr : NewOccludingComps.Difference(OccludingComps))
	{
		if (UPrimitiveComponent* PrimComp = PrimCompPtr.Get())
		{
			BeginOcclude(PrimComp);
			OnBeginOcclude.Broadcast(PrimComp);
		}
	}

	for (TWeakObjectPtr<UPrimitiveComponent> PrimCompPtr : OccludingComps.Difference(NewOccludingComps))
	{
		if (UPrimitiveComponent* PrimComp = PrimCompPtr.Get())
		{
			EndOcclude(PrimComp);
			OnEndOcclude.Broadcast(PrimComp);
		}
	}

	Swap(OccludingComps, NewOccludingComps);
	NewOccludingComps.Empty();
}

bool UOcclusionSpringArmComponent::TestOcclude(UPrimitiveComponent* OccludingComponent)
{
	if (bFilterWalkable && OccludingComponent->GetWalkableSlopeOverride().GetWalkableSlopeBehavior() != EWalkableSlopeBehavior::WalkableSlope_Unwalkable)
	{
		return true;
	}

	return false;
}

void UOcclusionSpringArmComponent::BeginOcclude(UPrimitiveComponent* OccludingComponent)
{
	//Interaction->IgnoreActor(Mesh->GetOwner(), true);

	const int32 NumMaterials = OccludingComponent->GetNumMaterials();
	for (int32 MaterialIdx = 0; MaterialIdx < NumMaterials; ++MaterialIdx)
	{
		if (MaskedMaterial)
		{
			UMaterialInterface* Material = OccludingComponent->GetMaterial(MaterialIdx);

			if (Material->GetBaseMaterial() != MaskedMaterial)
			{
				UMaterialInstanceDynamic* MID = OccludingComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(MaterialIdx, MaskedMaterial);
				MID->CopyMaterialUniformParameters(Material);
			}
		}

		OccludingComponent->SetCustomPrimitiveDataFloat(MaskedMaterialTimeDataIndex, GetWorld()->TimeSeconds);
	}
}

void UOcclusionSpringArmComponent::EndOcclude(UPrimitiveComponent* OccludingComponent)
{
	//Interaction->IgnoreActor(Mesh->GetOwner(), false);

	const int32 NumMaterials = OccludingComponent->GetNumMaterials();
	for (int32 MaterialIdx = 0; MaterialIdx < NumMaterials; ++MaterialIdx)
	{
		if (UMaterialInstance* MI = Cast<UMaterialInstance>(OccludingComponent->GetMaterial(MaterialIdx)))
		{
			if (MI->Parent == MaskedMaterial)
			{
				// We set this override, so remove it
				OccludingComponent->SetMaterial(MaterialIdx, nullptr);
			}
		}
	}

	//OccludingComponent->CleanUpOverrideMaterials();
}
