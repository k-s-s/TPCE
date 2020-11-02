// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "Kismet/KismetMathLibraryExtensions.h"
#include "Math/MathExtensions.h"

UKismetMathLibraryEx::UKismetMathLibraryEx(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UKismetMathLibraryEx::RayPlaneIntersection(const FVector& RayStart, const FVector& RayDir, const FPlane& APlane, float& T, FVector& Intersection)
{
	// Check ray is not parallel to plane
	if (FMath::IsNearlyZero((RayDir | APlane), SMALL_NUMBER))
	{
		T = 0.0f;
		Intersection = FVector::ZeroVector;
		return false;
	}

	T = ((APlane.W - (RayStart | APlane)) / (RayDir | APlane));

	// Calculate intersection point
	Intersection = RayStart + RayDir * T;

	return true;
}


FBounds UKismetMathLibraryEx::Multiply_BoundsFloat(FBounds A, float B)
{
	return A * B;
}

FBounds UKismetMathLibraryEx::Multiply_BoundsInt(FBounds A, int32 B)
{
	return A * B;
}

FBounds UKismetMathLibraryEx::Multiply_BoundsBounds(FBounds A, const FBounds&  B)
{
	return A * B;
}

FBounds UKismetMathLibraryEx::Divide_BoundsFloat(FBounds A, float B)
{
	return A / B;
}

FBounds UKismetMathLibraryEx::Divide_BoundsInt(FBounds A, int32 B)
{
	return A / B;
}

FBounds UKismetMathLibraryEx::Divide_BoundsBounds(FBounds A, const FBounds&  B)
{
	return A / B;
}

FBounds UKismetMathLibraryEx::Add_BoundsBounds(FBounds A, const FBounds&  B)
{
	return A + B;
}

FBounds UKismetMathLibraryEx::Add_BoundsFloat(FBounds A, float B)
{
	return A + B;
}

FBounds UKismetMathLibraryEx::Add_BoundsInt(FBounds A, int32 B)
{
	return A + B;
}

FBounds UKismetMathLibraryEx::Subtract_BoundsBounds(FBounds A, const FBounds&  B)
{
	return A - B;
}

FBounds UKismetMathLibraryEx::Subtract_BoundsFloat(FBounds A, float B)
{
	return A - B;
}

FBounds UKismetMathLibraryEx::Subtract_BoundsInt(FBounds A, int32 B)
{
	return A - B;
}

bool UKismetMathLibraryEx::EqualEqual_BoundsBounds(const FBounds&  A, const FBounds&  B, float ErrorTolerance)
{
	return A.Equals(B, ErrorTolerance);
}

bool UKismetMathLibraryEx::NotEqual_BoundsBounds(const FBounds&  A, const FBounds&  B, float ErrorTolerance)
{
	return !A.Equals(B, ErrorTolerance);
}

bool UKismetMathLibraryEx::BoundsIsReversed(const FBounds& A)
{
	return A.IsReversed();
}

float UKismetMathLibraryEx::BoundsLength(const FBounds& A)
{
	return A.Length();
}

bool UKismetMathLibraryEx::BoundsContains(const FBounds& A, const float Value)
{
	return A.Contains(Value);
}

void UKismetMathLibraryEx::BoundsExpand(FBounds& A, float Amount)
{
	A.Expand(Amount);
}

void UKismetMathLibraryEx::BoundsInclude(FBounds& A, float Value)
{
	A.Include(Value);
}

float UKismetMathLibraryEx::BoundsInterpolate(const FBounds& A, float Alpha)
{
	return A.Interpolate(Alpha);
}

ECardinalDirection UKismetMathLibraryEx::CalculateCardinalDirection(float Angle, ECardinalDirection CurrentCardinalDirection, const float NorthSegmentHalfWidth, float Buffer)
{
	return FMathEx::FindCardinalDirection(Angle, CurrentCardinalDirection, NorthSegmentHalfWidth, Buffer);
}

float UKismetMathLibraryEx::SoftCap(float Value, float Start, float End)
{
	const float t = FMath::GetRangePct(Start, End, Value);

	if (t < 0.0f)
	{
		return Value;
	}
	else if (t <= 1.0f)
	{
		return Start + (t - t*t * 0.5) * (End - Start);
	}
	else
	{
		return (Start + End) * 0.5;
	}
}

float UKismetMathLibraryEx::Saturate(float Value)
{
	return FMath::Clamp(Value, 0.0f, 1.0f);
}

float UKismetMathLibraryEx::OneMinus(float Value)
{
	return 1.0f - Value;
}

float UKismetMathLibraryEx::GetVectorComponent(const FVector& A, const EVectorComponent Select)
{
	if (Select == EVectorComponent::X)
	{
		return A.X;
	}
	else if (Select == EVectorComponent::Y)
	{
		return A.Y;
	}
	else if (Select == EVectorComponent::Z)
	{
		return A.Z;
	}

	return 0.0f;
}

FVector UKismetMathLibraryEx::SetVectorComponent(const FVector& A, const EVectorComponent Select, const float& Value)
{
	if (Select == EVectorComponent::X)
	{
		return FVector(Value, A.Y, A.Z);
	}
	else if (Select == EVectorComponent::Y)
	{
		return FVector(A.X, Value, A.Z);
	}
	else if (Select == EVectorComponent::Z)
	{
		return FVector(A.X, A.Y, Value);
	}

	return A;
}

FTransform UKismetMathLibraryEx::Transform_Identity()
{
	return FTransform();
}

float UKismetMathLibraryEx::ExponentialMovingAverage_Float(float CurrentSample, float PreviousSample, float PreviousAverage, float DeltaTime, float Alpha)
{
	// From Exponential Moving Averages for Irregular Time Series at https://oroboro.com/irregular-ema/
	if (DeltaTime <= 0.f || Alpha <= 0.f)
	{
		return CurrentSample;
	}
	float a = DeltaTime / Alpha;
	float u = FMath::Exp(-a);
	float v = (1.f - u) / a;
	float WAvg = (u * PreviousAverage) + ((v - u) * PreviousSample) + ((1.f - v) * CurrentSample);
	return WAvg;
}

KISMET_MATH_FORCEINLINE
float UKismetMathLibraryEx::RandomFloatVariance(float Base, float Variance)
{
	return FMath::FRandRange(Base - Variance * .5f, Base + Variance * .5f);
}
