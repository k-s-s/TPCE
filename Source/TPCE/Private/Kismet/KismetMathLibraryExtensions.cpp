// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "Kismet/KismetMathLibraryExtensions.h"
#include "Math/MathExtensions.h"

const FName DivideByZeroWarning = FName("DivideByZeroWarning");

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

float UKismetMathLibraryEx::SoftClipRange(float Value, float Low, float High, float Knee)
{
	if (Low > High)
	{
		Swap(Low, High);
	}

	return FMathEx::SoftClipRange(Value, Low, High, Knee);
}

float UKismetMathLibraryEx::Saturate(float Value)
{
	return FMath::Clamp(Value, 0.f, 1.f);
}

float UKismetMathLibraryEx::OneMinus(float Value)
{
	return 1.f - Value;
}

float UKismetMathLibraryEx::ConstantBiasScale(float Value, float Bias, float Scale)
{
	return (Value + Bias) * Scale;
}

float UKismetMathLibraryEx::EaseSinusoidal(float Value)
{
	return .5f * (1.f - FMath::Cos(Value * PI));
}

KISMET_MATH_FORCEINLINE
uint8 UKismetMathLibraryEx::NegativePercent_ByteByte(uint8 A, uint8 B)
{
	if (B == 0)
	{
		ReportError_NegativePercent_ByteByte();
		return 0;
	}

	return ((A % B) + B) % B;
}

KISMET_MATH_FORCEINLINE
int32 UKismetMathLibraryEx::NegativePercent_IntInt(int32 A, int32 B)
{
	if (B == 0)
	{
		ReportError_NegativePercent_IntInt();
		return 0;
	}

	return ((A % B) + B) % B;
}

/* This function is custom thunked, the real function is GenericNegativePercent_FloatFloat */
float UKismetMathLibraryEx::NegativePercent_FloatFloat(float A, float B)
{
	check(0);
	return 0;
}

float UKismetMathLibraryEx::GenericNegativePercent_FloatFloat(float A, float B)
{
	return (B != 0.f) ? FMath::Fmod(FMath::Fmod(A, B) + B, B) : 0.f;
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

	return 0.f;
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

KISMET_MATH_FORCEINLINE
FVector2D UKismetMathLibraryEx::V2Lerp(FVector2D A, FVector2D B, float V)
{
	return A + V * (B - A);
}

KISMET_MATH_FORCEINLINE
FIntVector UKismetMathLibraryEx::IntVector_Zero()
{
	return FIntVector::ZeroValue;
}

KISMET_MATH_FORCEINLINE
FIntVector UKismetMathLibraryEx::IntVector_One()
{
	return FIntVector(1);
}

KISMET_MATH_FORCEINLINE
FIntVector UKismetMathLibraryEx::IntVector_None()
{
	return FIntVector::NoneValue;
}

KISMET_MATH_FORCEINLINE
FIntVector UKismetMathLibraryEx::IntVector_Forward()
{
	return FIntVector(1, 0, 0);
}

KISMET_MATH_FORCEINLINE
FIntVector UKismetMathLibraryEx::IntVector_Backward()
{
	return FIntVector(-1, 0, 0);
}

KISMET_MATH_FORCEINLINE
FIntVector UKismetMathLibraryEx::IntVector_Up()
{
	return FIntVector(0, 0, 1);
}

KISMET_MATH_FORCEINLINE
FIntVector UKismetMathLibraryEx::IntVector_Down()
{
	return FIntVector(0, 0, -1);
}

KISMET_MATH_FORCEINLINE
FIntVector UKismetMathLibraryEx::IntVector_Right()
{
	return FIntVector(0, 1, 0);
}

KISMET_MATH_FORCEINLINE
FIntVector UKismetMathLibraryEx::IntVector_Left()
{
	return FIntVector(0, -1, 0);
}

KISMET_MATH_FORCEINLINE
FIntVector UKismetMathLibraryEx::Add_IntVectorIntVector(FIntVector A, FIntVector B)
{
	return A + B;
}

KISMET_MATH_FORCEINLINE
FIntVector UKismetMathLibraryEx::Add_IntVectorInt(FIntVector A, int32 B)
{
	return A + FIntVector(B);
}

KISMET_MATH_FORCEINLINE
FIntVector UKismetMathLibraryEx::Subtract_IntVectorIntVector(FIntVector A, FIntVector B)
{
	return A - B;
}

KISMET_MATH_FORCEINLINE
FIntVector UKismetMathLibraryEx::Subtract_IntVectorInt(FIntVector A, int32 B)
{
	return A - FIntVector(B);
}

KISMET_MATH_FORCEINLINE
FIntVector UKismetMathLibraryEx::Multiply_IntVectorIntVector(FIntVector A, FIntVector B)
{
	return FIntVector(A.X * B.X, A.Y * B.Y, A.Z * B.Z);
}

KISMET_MATH_FORCEINLINE
FIntVector UKismetMathLibraryEx::Multiply_IntVectorInt(FIntVector A, int32 B)
{
	return FIntVector(A.X * B, A.Y * B, A.Z * B);
}

KISMET_MATH_FORCEINLINE
FIntVector UKismetMathLibraryEx::Divide_IntVectorIntVector(FIntVector A, FIntVector B)
{
	if (B.X == 0 || B.Y == 0 || B.Z == 0)
	{
		ReportError_Divide_IntVectorOnIntVector();
		return FIntVector::ZeroValue;
	}

	return FIntVector(A.X / B.X, A.Y / B.Y, A.Z / B.Z);
}

KISMET_MATH_FORCEINLINE
FIntVector UKismetMathLibraryEx::Divide_IntVectorInt(FIntVector A, int32 B)
{
	if (B == 0)
	{
		ReportError_Divide_IntVectorOnInt();
		return FIntVector::ZeroValue;
	}

	return FIntVector(A.X / B, A.Y / B, A.Z / B);
}

KISMET_MATH_FORCEINLINE
bool UKismetMathLibraryEx::Equal_IntVectorIntVector(FIntVector A, FIntVector B)
{
	return A == B;
}

KISMET_MATH_FORCEINLINE
bool UKismetMathLibraryEx::NotEqual_IntVectorIntVector(FIntVector A, FIntVector B)
{
	return A != B;
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

void UKismetMathLibraryEx::GetProjectionMatrix(FVector2D Pivot, FVector2D Translation, float RotationAngle, float Scale, float HorizontalFOVAngle, float AspectRatio, FMatrix& ProjectionMatrix)
{
	// Taken from FComposurePostMoveSettings::GetProjectionMatrix
	float HalfFOV = 0.5 * FMath::DegreesToRadians(HorizontalFOVAngle);

	FMatrix OriginalProjectionMatrix = FReversedZPerspectiveMatrix(
		HalfFOV,
		HalfFOV,
		/* XAxisMultiplier =*/ 1.0,
		/* YAxisMultiplier =*/ AspectRatio,
		GNearClippingPlane,
		GNearClippingPlane);

	FVector2D NormalizedViewRect = FMath::Tan(HalfFOV) * FVector2D(1, 1 / AspectRatio);
	FVector2D NormalizedPostMoveTranslation = 2.0 * NormalizedViewRect * Translation;
	FVector2D NormalizedPivot = NormalizedViewRect * (Pivot - 0.5) * FVector2D(2, 2);

	FMatrix ScaleMatrix(
		FPlane(Scale, 0, 0, 0),
		FPlane(0, Scale, 0, 0),
		FPlane(0, 0, 1, 0),
		FPlane(0, 0, 0, 1));

	FMatrix PreRotationMatrix(
		FPlane(1, 0, 0, 0),
		FPlane(0, 1, 0, 0),
		FPlane(-NormalizedPivot.X, -NormalizedPivot.Y, 1, 0),
		FPlane(0, 0, 0, 1));

	float Rotation = FMath::DegreesToRadians(RotationAngle);
	FMatrix RotationMatrix(
		FPlane(FMath::Cos(Rotation), FMath::Sin(Rotation), 0, 0),
		FPlane(-FMath::Sin(Rotation), FMath::Cos(Rotation), 0, 0),
		FPlane(0, 0, 1, 0),
		FPlane(0, 0, 0, 1));

	FMatrix PostRotationMatrix(
		FPlane(1, 0, 0, 0),
		FPlane(0, 1, 0, 0),
		FPlane(NormalizedPivot.X, NormalizedPivot.Y, 1, 0),
		FPlane(0, 0, 0, 1));

	FMatrix TranslateMatrix(
		FPlane(1, 0, 0, 0),
		FPlane(0, 1, 0, 0),
		FPlane(NormalizedPostMoveTranslation.X, NormalizedPostMoveTranslation.Y, 1, 0),
		FPlane(0, 0, 0, 1));

	ProjectionMatrix = PreRotationMatrix
		* ScaleMatrix
		* RotationMatrix
		* PostRotationMatrix
		* TranslateMatrix
		* OriginalProjectionMatrix;
}

void UKismetMathLibraryEx::ReportError_NegativePercent_ByteByte()
{
	FFrame::KismetExecutionMessage(TEXT("Modulo by zero"), ELogVerbosity::Warning, DivideByZeroWarning);
}

void UKismetMathLibraryEx::ReportError_NegativePercent_IntInt()
{
	FFrame::KismetExecutionMessage(TEXT("Modulo by zero"), ELogVerbosity::Warning, DivideByZeroWarning);
}

void UKismetMathLibraryEx::ReportError_Divide_IntVectorOnInt()
{
	FFrame::KismetExecutionMessage(TEXT("Divide by zero: Divide_IntVectorInt"), ELogVerbosity::Warning, DivideByZeroWarning);
}

void UKismetMathLibraryEx::ReportError_Divide_IntVectorOnIntVector()
{
	FFrame::KismetExecutionMessage(TEXT("Divide by zero: Divide_IntVectorIntVector"), ELogVerbosity::Warning, DivideByZeroWarning);
}
