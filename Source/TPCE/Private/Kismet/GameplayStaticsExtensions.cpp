// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "Kismet/GameplayStaticsExtensions.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/StaticMeshSocket.h"
#include "Engine/Canvas.h"

UGameplayStaticsEx::UGameplayStaticsEx(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGameplayStaticsEx::BreakHitResultEx(const FHitResult& Hit, FName& MyBoneName, float& PenetrationDepth)
{
	MyBoneName = Hit.MyBoneName;
	PenetrationDepth = Hit.PenetrationDepth;
}

void UGameplayStaticsEx::GetSimpleCollisionCylinder(class AActor* Actor, float& CollisionRadius, float& CollisionHalfHeight)
{
	Actor->GetSimpleCollisionCylinder(CollisionRadius, CollisionHalfHeight);
}

FString UGameplayStaticsEx::GetSocketTag(USceneComponent* Target, FName InSocketName)
{
	if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Target))
	{
		if (UStaticMeshSocket const* Socket = StaticMeshComponent->GetSocketByName(InSocketName))
		{
			return Socket->Tag;
		}
	}

	return FString();
}

ETeamAttitude::Type UGameplayStaticsEx::GetTeamAttitudeTowards(AActor* ThisActor, AActor* OtherActor)
{
	if (ThisActor && OtherActor)
	{
		if (const IGenericTeamAgentInterface* TeamAgent = Cast<const IGenericTeamAgentInterface>(ThisActor))
		{
			return TeamAgent->GetTeamAttitudeTowards(*OtherActor);
		}
	}

	return ETeamAttitude::Neutral;
}

void UGameplayStaticsEx::SetSourceCubemapAngle(USkyLightComponent* SkyLightComponent, float NewSourceCubemapAngle)
{
	SkyLightComponent->SourceCubemapAngle = NewSourceCubemapAngle;
	SkyLightComponent->SetCaptureIsDirty();
}

void UGameplayStaticsEx::DrawPolyline(UCanvas* InCanvas, UMaterialInterface* RenderMaterial, const TArray<FVector2D>& Points, float Thickness, float UTiling)
{
	if (InCanvas && RenderMaterial && Points.Num() > 1)
	{
		const int32 NumPoints = Points.Num();

		TArray<FVector2D> Offsets;
		Offsets.Reserve(NumPoints);
		for (int32 PointIdx = 0; PointIdx < NumPoints; PointIdx++)
		{
			FVector2D Direction;
			float Length = Thickness;

			if (PointIdx == 0)
			{
				// Starting point uses (next - current)
				Direction = (Points[PointIdx + 1] - Points[PointIdx]).GetSafeNormal();
			}
			else if (PointIdx == NumPoints - 1)
			{
				// Ending point uses (current - previous)
				Direction = (Points[PointIdx] - Points[PointIdx - 1]).GetSafeNormal();
			}
			else
			{
				// Compute miter join normal and length
				const FVector2D DirectionA = (Points[PointIdx] - Points[PointIdx - 1]).GetSafeNormal();
				const FVector2D DirectionB = (Points[PointIdx + 1] - Points[PointIdx]).GetSafeNormal();
				const FVector2D Tangent = (DirectionA + DirectionB).GetSafeNormal();
				const FVector2D Perp = FVector2D(-DirectionA.Y, DirectionA.X);
				const FVector2D Miter = FVector2D(-Tangent.Y, Tangent.X);
				Direction = Tangent;
				Length = Thickness / FVector2D::DotProduct(Miter, Perp);
			}

			const FVector2D Normal = FVector2D(-Direction.Y, Direction.X);
			Offsets.Add(Normal * Length);
		}

		TArray<FCanvasUVTri> Triangles;
		float U = 0.f;
		for (int32 PointIdx = 1; PointIdx < NumPoints; PointIdx++)
		{
			const FVector2D Pos0 = Points[PointIdx - 1];
			const FVector2D Pos1 = Points[PointIdx];
			const FVector2D Off0 = Offsets[PointIdx - 1];
			const FVector2D Off1 = Offsets[PointIdx];
			const float Distance = (UTiling != 0.f) ? (FVector2D::Distance(Pos0, Pos1) / UTiling) : 0.f;

			FCanvasUVTri& Tri0 = Triangles.Emplace_GetRef();
			Tri0.V0_Pos = Pos0 + Off0;
			Tri0.V1_Pos = Pos0 - Off0;
			Tri0.V2_Pos = Pos1 + Off1;
			Tri0.V0_UV = FVector2D(U, 1.f);
			Tri0.V1_UV = FVector2D(U, 0.f);
			Tri0.V2_UV = FVector2D(U + Distance, 1.f);

			FCanvasUVTri& Tri1 = Triangles.Emplace_GetRef();
			Tri1.V0_Pos = Pos0 - Off0;
			Tri1.V1_Pos = Pos1 - Off1;
			Tri1.V2_Pos = Pos1 + Off1;
			Tri1.V0_UV = FVector2D(U, 0.f);
			Tri1.V1_UV = FVector2D(U + Distance, 0.f);
			Tri1.V2_UV = FVector2D(U + Distance, 1.f);

			U += Distance;
		}

		FCanvasTriangleItem TriangleItem(FVector2D::ZeroVector, FVector2D::ZeroVector, FVector2D::ZeroVector, NULL);
		TriangleItem.MaterialRenderProxy = RenderMaterial->GetRenderProxy();
		TriangleItem.TriangleList = MoveTemp(Triangles);
		TriangleItem.bFreezeTime = false;

		InCanvas->DrawItem(TriangleItem);
	}
}

void ChamferCorner(TArray<FVector2D>& Points, const FVector2D& P0, const FVector2D& P1, const FVector2D& P2, float Radius, float AngleStep)
{
	FVector2D V0 = P1 - P0;
	FVector2D V1 = P1 - P2;
	const float V0Size = V0.Size();
	const float V1Size = V1.Size();
	const float MinSize = FMath::Min(V0Size, V1Size);
	V0 /= V0Size;
	V1 /= V1Size;

	// Calculate length of segment between corner and the points of intersection with the circle of a given radius
	const float Angle = FMath::Acos(V0 | V1);
	if ((PI - Angle) < AngleStep)
	{
		// Segments are colinear, output the existing corner and exit
		Points.Emplace(P1);
		return;
	}

	const float HalfAngleTan = FMath::Tan(FMath::Abs(Angle) / 2);
	float SegmentLen = Radius / HalfAngleTan;
	if (SegmentLen > MinSize)
	{
		SegmentLen = MinSize;
		Radius = MinSize * HalfAngleTan;
	}

	// Points of intersection are calculated by the proportion between the coordinates of the vector, length of vector and the length of the segment
	const FVector2D C0 = P1 - SegmentLen * V0;
	const FVector2D C1 = P1 - SegmentLen * V1;

	// Calculate coordinates of the circle center by the addition of angular vectors
	const FVector2D C = P1 * 2 - C0 - C1;
	const float R = FMath::Sqrt(SegmentLen * SegmentLen + Radius * Radius);
	const FVector2D CircleCenter = P1 - (R / C.Size()) * C;

	// Calculate arc
	const float StartAngle = FMath::Atan2(C0.Y - CircleCenter.Y, C0.X - CircleCenter.X);
	const float EndAngle = FMath::Atan2(C1.Y - CircleCenter.Y, C1.X - CircleCenter.X);
	float SweepAngle = FMath::FindDeltaAngleRadians(StartAngle, EndAngle);

	// Output points of the resulting arc
	const int32 NumDivisions = FMath::CeilToInt(FMath::Abs(SweepAngle) / AngleStep);
	AngleStep = SweepAngle / NumDivisions;

	const int32 NumPoints = NumDivisions + 1;
	Points.Reserve(Points.Num() + NumPoints);

	for (int32 PointIdx = 0; PointIdx < NumPoints; PointIdx++)
	{
		float Sin, Cos;
		FMath::SinCos(&Sin, &Cos, StartAngle + AngleStep * PointIdx);
		Points.Emplace(CircleCenter + FVector2D(Cos * Radius, Sin * Radius));
	}
}

void UGameplayStaticsEx::DrawChamferedPolyline(UCanvas* InCanvas, UMaterialInterface* RenderMaterial, const TArray<FVector2D>& InPoints, float Thickness, float UTiling, float Radius, float Step)
{
	if (InCanvas && RenderMaterial && InPoints.Num() > 1)
	{
		if (Radius > 0.f && InPoints.Num() >= 3 && Step > 0.f)
		{
			Step = FMath::DegreesToRadians(Step);

			// Endpoints are untouched, chamfer every corner vertex
			TArray<FVector2D> Points;
			Points.Reserve(InPoints.Num() * 2);  // Wild guess

			Points.Add(InPoints[0]);
			const int32 LastCornerIdx = InPoints.Num() - 1;
			for (int32 PointIdx = 1; PointIdx < LastCornerIdx; PointIdx++)
			{
				ChamferCorner(Points, InPoints[PointIdx - 1], InPoints[PointIdx], InPoints[PointIdx + 1], Radius, Step);
			}
			Points.Add(InPoints.Last());

			DrawPolyline(InCanvas, RenderMaterial, Points, Thickness, UTiling);
		}
		else
		{
			DrawPolyline(InCanvas, RenderMaterial, InPoints, Thickness, UTiling);
		}
	}
}
