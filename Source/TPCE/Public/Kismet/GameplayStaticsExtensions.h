// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "Kismet/GameplayStatics.h"
#include "GenericTeamAgentInterface.h"

#include "GameplayStaticsExtensions.generated.h"


UCLASS()
class TPCE_API UGameplayStaticsEx : public UGameplayStatics
{
	GENERATED_UCLASS_BODY()

public:

	/**
	 * Extracts additional data from a HitResult.
	 *
	 * @param MyBoneName Name of _my_ bone which took part in hit event (in case of two skeletal meshes colliding).
	 * @param PenetrationDepth Distance along Normal that will result in moving out of penetration or zero if it cannot be computed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Collision")
	static void BreakHitResultEx(const struct FHitResult& Hit, FName& MyBoneName, float& PenetrationDepth);

	/**
	 * Get axis-aligned cylinder around this actor, used for simple collision checks (ie Pawns reaching a destination).
	 * If IsRootComponentCollisionRegistered() returns true, just returns its bounding cylinder, otherwise falls back to GetComponentsBoundingCylinder.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Collision")
	static void GetSimpleCollisionCylinder(class AActor* Actor, float& CollisionRadius, float& CollisionHalfHeight);

	/**
	* Return the tag associated with the socket if it exists.
	* @param InSocketName Name of the socket to get the tag from.
	* @return The socket's tag if the socket with the given name exists. Otherwise, an empty string.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Utilities|Transformation", meta=(Keywords="Bone"))
	static FString GetSocketTag(class USceneComponent* Target, FName InSocketName);

	/** Return team attitude between two actors. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="AI", meta=(DefaultToSelf="ControlledObject"))
	static ETeamAttitude::Type GetTeamAttitudeTowards(class AActor* ThisActor, class AActor* OtherActor);

	/** Set the Source Cubemap Angle of a Skylight Component. */
	UFUNCTION(BlueprintCallable, Category="Light")
	static void SetSourceCubemapAngle(class USkyLightComponent* SkyLightComponent, float NewSourceCubemapAngle);

	/**
	 * Draws a connected sequence of miter joined line segments.
	 * Draws a set of miter-joined segments on the Canvas.
	 *
	 * @param RenderMaterial			Material to use when rendering. Material should be Masked Unlit with Opacity Mask Clip 0.001f and flagged for Editor Compositing usage.
	 * @param Points					Array of vertices.
	 * @param Thickness					Screen space width of the line.
	 * @param UTiling					Scale of the U coordinates generated along the line.
	 */
	UFUNCTION(BlueprintCallable, Category="Canvas")
	static void DrawPolyline(class UCanvas* InCanvas, class UMaterialInterface* RenderMaterial, const TArray<FVector2D>& Points, float Thickness = 1.f, float UTiling = 100.f);

	/**
	 * Draws a connected sequence of chamfer joined line segments.
	 *
	 * @param RenderMaterial			Material to use when rendering. Material should be Masked Unlit with Opacity Mask Clip 0.001f and flagged for Editor Compositing usage.
	 * @param Points					Array of vertices.
	 * @param Thickness					Screen space width of the line.
	 * @param UTiling					Scale of the U coordinates generated along the line.
	 * @param Radius					Chamfer radius.
	 * @param Step						Chamfer resolution in degrees. With a Step of 10.0, a 90 degree corner would be divided into 9 segments.
	 */
	UFUNCTION(BlueprintCallable, Category="Canvas")
	static void DrawChamferedPolyline(class UCanvas* InCanvas, class UMaterialInterface* RenderMaterial, const TArray<FVector2D>& Points, float Thickness = 1.f, float UTiling = 100.f, float Radius = 0.f, float Step = 10.f);
};
