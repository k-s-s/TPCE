// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "Math/UnrealMathUtility.h"
#include "Math/Bounds.h"
#include "ExtraTypes.h"

#define COS_0		(1.0f)
#define COS_15		(0.96592582629f)
#define COS_30		(0.86602540378f)
#define COS_45		(0.70710678118f)
#define COS_60		(0.5f)
#define COS_90		(0.0f)

struct FMathEx : public FMath
{
	using FMath::RInterpTo;



	/** Maps negative values to positive values while going back and forth. */
	static FORCEINLINE uint32 ZigZag(int32 x)
	{
		return (uint32(x) << 1) ^ (x >> 31);
	}

	/** Maps negative values to positive values while going back and forth. */
	static FORCEINLINE uint64 ZigZag_64(int64 x)
	{
		return (uint64(x) << 1) ^ (x >> 63);
	}

	/** Reverses ZigZag. Maps alternating values to positive and negative values. */
	static FORCEINLINE int32 ReverseZigZag(uint32 y)
	{
		return static_cast<int32>(((y >> 1) ^ -(static_cast<int64>(y) & 1)));
	}

	/** Reverses ZigZag_64. Maps alternating values to positive and negative values. */
	static FORCEINLINE int64 ReverseZigZag_64(uint64 y)
	{
		return static_cast<int64>(((y >> 1) ^ -(static_cast<int64>(y) & 1)));
	}



	/** Interpolate rotator from Current to Target. Scaled by distance to Target, so it has a strong start speed and ease out. */
	static TPCE_API FQuat QInterpTo(const FQuat& Current, const FQuat& Target, float DeltaTime, float InterpSpeed);

	/** Interpolate from Current to Target as rotation axes. Scaled by distance to Target, so it has a strong start speed and ease out. */
	static TPCE_API float FInterpAngleTo(const float Current, const float Target, float DeltaTime, float InterpSpeed);

	/** Interpolate from Current to Target as rotation axes. Scaled by distance to Target, so it has a strong start speed and ease out. */
	static TPCE_API float FInterpConstantAngleTo(const float Current, const float Target, float DeltaTime, float InterpSpeed);



	/**
	 * Interpolate vector from Current to Target with InterpSpeed as the minimum interpolation speed.
	 * Scaled by distance to Target until it becomes lower than 1 when it starts to use a constant interpolation.
	 * Target is guaranteed to be reached in a more reasonable time than with the standard function.
	 */
	static TPCE_API FVector VSafeInterpTo(const FVector& Current, const FVector& Target, float DeltaTime, float InterpSpeed);

	/**
	 * Interpolate vector2D from Current to Target with InterpSpeed as the minimum interpolation speed.
	 * Scaled by distance to Target until it becomes lower than 1 when it starts to use a constant interpolation.
	 * Target is guaranteed to be reached in a more reasonable time than with the standard function.
	 */
	static TPCE_API FVector2D Vector2DSafeInterpTo(const FVector2D& Current, const FVector2D& Target, float DeltaTime, float InterpSpeed);

	/**
	 * Interpolate rotator from Current to Target with InterpSpeed as the minimum interpolation speed.
	 * Scaled by distance to Target until it becomes lower than 1 when it starts to use a constant interpolation.
	 * Target is guaranteed to be reached in a more reasonable time than with the standard function.
	 */
	static TPCE_API FRotator RSafeInterpTo(const FRotator& Current, const FRotator& Target, float DeltaTime, float InterpSpeed);

	/**
	 * Interpolate from Current to Target with InterpSpeed as the minimum interpolation speed.
	 * Scaled by distance to Target until it becomes lower than 1 when it starts to use a constant interpolation.
	 * Target is guaranteed to be reached in a more reasonable time than with the standard function.
	 */
	static TPCE_API float FSafeInterpTo(const float Current, const float Target, float DeltaTime, float InterpSpeed);

	/**
	 * Interpolate angle from Current to Target along the smallest arc with InterpSpeed as the minimum interpolation speed.
	 * Scaled by distance to Target until it becomes lower than 1 when it starts to use a constant interpolation.
	 * Target is guaranteed to be reached in a more reasonable time than with the standard function.
	 */
	static TPCE_API float FSafeInterpAngleTo(const float Current, const float Target, float DeltaTime, float InterpSpeed);

	/**
	 * Interpolate Linear Color from Current to Target with InterpSpeed as the minimum interpolation speed.
	 * Scaled by distance to Target until it becomes lower than 1 when it starts to use a constant interpolation.
	 * Target is guaranteed to be reached in a more reasonable time than with the standard function.
	 */
	static TPCE_API FLinearColor CSafeInterpTo(const FLinearColor& Current, const FLinearColor& Target, float DeltaTime, float InterpSpeed);



	/** Interpolate from Current to Target using a spring-damper like function that does not overshoot. */
	static TPCE_API float FSmoothInterpTo(float Current, float Target, float& CurrentVelocity, float SmoothTime, float MaxSpeed, float DeltaTime);

	/** Interpolate from Current to Target using a spring-damper like function that does not overshoot. */
	static TPCE_API float FSmoothInterpAngleTo(float Current, float Target, float& CurrentVelocity, float SmoothTime, float MaxSpeed, float DeltaTime);

	/** Interpolate from Current to Target using a spring-damper like function that does not overshoot. */
	static TPCE_API FVector VSmoothInterpTo(const FVector& Current, const FVector& Target, FVector& CurrentVelocity, float SmoothTime, float MaxSpeed, float DeltaTime);

	/** Interpolate from Current to Target using a spring-damper like function that does not overshoot. */
	static TPCE_API FRotator RSmoothInterpTo(const FRotator& Current, const FRotator& Target, FRotator& CurrentVelocity, float SmoothTime, float MaxSpeed, float DeltaTime);

	/** Interpolate from Current to Target using a spring-damper like function that does not overshoot. */
	static TPCE_API FRotator RSmoothInterpTo(const FRotator& Current, const FRotator& Target, FRotator& CurrentVelocity, float SmoothTime, const FRotator& MaxSpeed, float DeltaTime);

	/** Find the cardinal direction for an angle given the current cardinal direction, the half angle width of the north segment and a buffer for tolerance. */
	static TPCE_API ECardinalDirection FindCardinalDirection(float Angle, const ECardinalDirection CurrentCardinalDirection, const float NorthSegmentHalfWidth = 60.f, const float Buffer = 5.0f);

	/** Find the cardinal direction to a target point given the start point and rotation. */
	static TPCE_API ECardinalDirection FindCardinalDirection(const FVector& Point, const FVector& StartPoint, const FQuat& Rotation);



	/**
	 * Applies a soft clip function. Output same as input until Start, then it smoothly tapers off.
	 * There should be at least a small difference between Start and End to determine the direction of the clipping.
	 * [0..Start..End..Inf] -> [0..Start..(Start+End)/2..(Start+End)/2]
	 */
	static TPCE_API float SoftClip(float Value, float Start, float End);

	/**
	 * Applies a soft clip function so that the value tapers off then stops as it reaches either Low or High.
	 * Input domain is [Low-Knee..High+Knee]
	 */
	static TPCE_API float SoftClipRange(float Value, float Low, float High, float Knee);
};
