// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreTypes.h"
#include "Misc/AssertionMacros.h"
#include "Math/UnrealMathUtility.h"
#include "Containers/UnrealString.h"
#include "Math/Vector.h"

/**
 * Implements an axis-aligned box.
 *
 * Boxes describe an axis-aligned extent in three dimensions. They are used for many different things in the
 * Engine and in games, such as bounding volumes, collision detection and visibility calculation.
 */
struct FIntBox
{
public:

	/** Holds the box's minimum point. */
	FIntVector Min;

	/** Holds the box's maximum point. */
	FIntVector Max;

	/** Holds a flag indicating whether this box is valid. */
	uint8 IsValid;

public:

	/** Default constructor (no initialization). */
	FIntBox() { }

	/**
	 * Creates and initializes a new box with zero extent and marks it as invalid.
	 *
	 * @param EForceInit Force Init Enum.
	 */
	explicit FIntBox( EForceInit )
	{
		Init();
	}

	/**
	 * Creates and initializes a new box from the specified extents.
	 *
	 * @param InMin The box's minimum point.
	 * @param InMax The box's maximum point.
	 */
	TPCE_API FIntBox( const FIntVector& InMin, const FIntVector& InMax )
		: Min(InMin)
		, Max(InMax)
		, IsValid(1)
	{ }

	/**
	 * Creates and initializes a new box from the given set of points.
	 *
	 * @param Points Array of Points to create for the bounding volume.
	 * @param Count The number of points.
	 */
	TPCE_API FIntBox( const FIntVector* Points, int32 Count );

	/**
	 * Creates and initializes a new box from an array of points.
	 *
	 * @param Points Array of Points to create for the bounding volume.
	 */
	FIntBox( const TArray<FIntVector>& Points );

public:

	/**
	 * Compares two boxes for equality.
	 *
	 * @return true if the boxes are equal, false otherwise.
	 */
	FORCEINLINE bool operator==( const FIntBox& Other ) const
	{
		return (Min == Other.Min) && (Max == Other.Max);
	}

	/**
	 * Compares two boxes for equality.
	 *
	 * @return true if the boxes are equal, false otherwise.
	 */
	FORCEINLINE bool operator!=(const FIntBox& Other) const
	{
		return (Min != Other.Min) || (Max != Other.Max);
	}

	/**
	 * Adds to this bounding box to include a given point.
	 *
	 * @param Other the point to increase the bounding volume to.
	 * @return Reference to this bounding box after resizing to include the other point.
	 */
	FORCEINLINE FIntBox& operator+=( const FIntVector &Other );

	/**
	 * Gets the result of addition to this bounding volume.
	 *
	 * @param Other The other point to add to this.
	 * @return A new bounding volume.
	 */
	FORCEINLINE FIntBox operator+( const FIntVector& Other ) const
	{
		return FIntBox(*this) += Other;
	}

	/**
	 * Adds to this bounding box to include a new bounding volume.
	 *
	 * @param Other the bounding volume to increase the bounding volume to.
	 * @return Reference to this bounding volume after resizing to include the other bounding volume.
	 */
	FORCEINLINE FIntBox& operator+=( const FIntBox& Other );

	/**
	 * Gets the result of addition to this bounding volume.
	 *
	 * @param Other The other volume to add to this.
	 * @return A new bounding volume.
	 */
	FORCEINLINE FIntBox operator+( const FIntBox& Other ) const
	{
		return FIntBox(*this) += Other;
	}

	/**
	 * Gets reference to the min or max of this bounding volume.
	 *
	 * @param Index the index into points of the bounding volume.
	 * @return a reference to a point of the bounding volume.
	 */
    FORCEINLINE FIntVector& operator[]( int32 Index )
	{
		check((Index >= 0) && (Index < 2));

		if (Index == 0)
		{
			return Min;
		}

		return Max;
	}

public:

	/**
	 * Calculates the distance of a point to this box.
	 *
	 * @param Point The point.
	 * @return The distance.
	 */
	FORCEINLINE float ComputeSquaredDistanceToPoint( const FVector& Point ) const
	{
		return ComputeSquaredDistanceFromBoxToPoint((FVector)Min, (FVector)Max, Point);
	}

	/**
	 * Increases the box size.
	 *
	 * @param W The size to increase the volume by.
	 * @return A new bounding box.
	 */
	FORCEINLINE FIntBox ExpandBy(int32 W) const
	{
		return FIntBox(Min - FIntVector(W, W, W), Max + FIntVector(W, W, W));
	}

	/**
	* Increases the box size.
	*
	* @param V The size to increase the volume by.
	* @return A new bounding box.
	*/
	FORCEINLINE FIntBox ExpandBy(const FIntVector& V) const
	{
		return FIntBox(Min - V, Max + V);
	}

	/**
	* Increases the box size.
	*
	* @param Neg The size to increase the volume by in the negative direction (positive values move the bounds outwards)
	* @param Pos The size to increase the volume by in the positive direction (positive values move the bounds outwards)
	* @return A new bounding box.
	*/
	FIntBox ExpandBy(const FIntVector& Neg, const FIntVector& Pos) const
	{
		return FIntBox(Min - Neg, Max + Pos);
	}

	/**
	 * Shifts the bounding box position.
	 *
	 * @param Offset The vector to shift the box by.
	 * @return A new bounding box.
	 */
	FORCEINLINE FIntBox ShiftBy( const FIntVector& Offset ) const
	{
		return FIntBox(Min + Offset, Max + Offset);
	}

	/**
	 * Gets the center point of this box.
	 *
	 * @return The center point.
	 * @see GetCenterAndExtents, GetExtent, GetSize, GetVolume
	 */
	FORCEINLINE FVector GetCenter() const
	{
		return FVector(Min + Max) * 0.5f;
	}

	/**
	 * Gets the center and extents of this box.
	 *
	 * @param center[out] Will contain the box center point.
	 * @param Extents[out] Will contain the extent around the center.
	 * @see GetCenter, GetExtent, GetSize, GetVolume
	 */
	FORCEINLINE void GetCenterAndExtents( FVector& center, FVector& Extents ) const
	{
		Extents = GetExtent();
		center = (FVector)Min + Extents;
	}

	/**
	 * Calculates the closest point on or inside the box to a given point in space.
	 *
	 * @param Point The point in space.
	 * @return The closest point on or inside the box.
	 */
	FORCEINLINE FIntVector GetClosestPointTo( const FIntVector& Point ) const;

	/**
	 * Gets the extents of this box.
	 *
	 * @return The box extents.
	 * @see GetCenter, GetCenterAndExtents, GetSize, GetVolume
	 */
	FORCEINLINE FVector GetExtent() const
	{
		return 0.5f * FVector(Max - Min);
	}

	UE_DEPRECATED(4.24, "This method performed unsafe operations and should be replaced with using .Min and .Max directly or using the [] operator on this class instead.")
	FORCEINLINE FIntVector& GetExtrema( int PointIndex )
	{
		return (&Min)[PointIndex];
	}

	UE_DEPRECATED(4.24, "This method performed unsafe operations and should be replaced with using .Min and .Max directly or using the [] operator on this class instead.")
	FORCEINLINE const FIntVector& GetExtrema( int PointIndex ) const
	{
		return (&Min)[PointIndex];
	}

	/**
	 * Gets the size of this box.
	 *
	 * @return The box size.
	 * @see GetCenter, GetCenterAndExtents, GetExtent, GetVolume
	 */
	FORCEINLINE FIntVector GetSize() const
	{
		return (Max - Min);
	}

	/**
	 * Gets the volume of this box.
	 *
	 * @return The box volume.
	 * @see GetCenter, GetCenterAndExtents, GetExtent, GetSize
	 */
	FORCEINLINE int32 GetVolume() const
	{
		return ((Max.X - Min.X) * (Max.Y - Min.Y) * (Max.Z - Min.Z));
	}

	/**
	 * Set the initial values of the bounding box to Zero.
	 */
	FORCEINLINE void Init()
	{
		Min = Max = FIntVector::ZeroValue;
		IsValid = 0;
	}

	/**
	 * Checks whether the given bounding box intersects this bounding box.
	 *
	 * @param Other The bounding box to intersect with.
	 * @return true if the boxes intersect, false otherwise.
	 */
	FORCEINLINE bool Intersect( const FIntBox& other ) const;

	/**
	 * Checks whether the given bounding box intersects this bounding box in the XY plane.
	 *
	 * @param Other The bounding box to test intersection.
	 * @return true if the boxes intersect in the XY Plane, false otherwise.
	 */
	FORCEINLINE bool IntersectXY( const FIntBox& Other ) const;

	/**
	 * Returns the overlap FIntBox of two box
	 *
	 * @param Other The bounding box to test overlap
	 * @return the overlap box. It can be 0 if they don't overlap
	 */
	TPCE_API FIntBox Overlap( const FIntBox& Other ) const;

	/**
	  * Gets a bounding volume transformed by an inverted FTransform object.
	  *
	  * @param M The transformation object to perform the inversely transform this box with.
	  * @return	The transformed box.
	  */
	TPCE_API FIntBox InverseTransformBy( const FTransform& M ) const;

	/**
	 * Checks whether the given location is inside this box.
	 *
	 * @param In The location to test for inside the bounding volume.
	 * @return true if location is inside this volume.
	 * @see IsInsideXY
	 */
	FORCEINLINE bool IsInside( const FIntVector& In ) const
	{
		return ((In.X > Min.X) && (In.X < Max.X) && (In.Y > Min.Y) && (In.Y < Max.Y) && (In.Z > Min.Z) && (In.Z < Max.Z));
	}

	/**
	 * Checks whether the given location is inside or on this box.
	 *
	 * @param In The location to test for inside the bounding volume.
	 * @return true if location is inside this volume.
	 * @see IsInsideXY
	 */
	FORCEINLINE bool IsInsideOrOn( const FIntVector& In ) const
	{
		return ((In.X >= Min.X) && (In.X <= Max.X) && (In.Y >= Min.Y) && (In.Y <= Max.Y) && (In.Z >= Min.Z) && (In.Z <= Max.Z));
	}

	/**
	 * Checks whether a given box is fully encapsulated by this box.
	 *
	 * @param Other The box to test for encapsulation within the bounding volume.
	 * @return true if box is inside this volume.
	 */
	FORCEINLINE bool IsInside( const FIntBox& Other ) const
	{
		return (IsInside(Other.Min) && IsInside(Other.Max));
	}

	/**
	 * Checks whether the given location is inside this box in the XY plane.
	 *
	 * @param In The location to test for inside the bounding box.
	 * @return true if location is inside this box in the XY plane.
	 * @see IsInside
	 */
	FORCEINLINE bool IsInsideXY( const FIntVector& In ) const
	{
		return ((In.X > Min.X) && (In.X < Max.X) && (In.Y > Min.Y) && (In.Y < Max.Y));
	}

	/**
	 * Checks whether the given box is fully encapsulated by this box in the XY plane.
	 *
	 * @param Other The box to test for encapsulation within the bounding box.
	 * @return true if box is inside this box in the XY plane.
	 */
	FORCEINLINE bool IsInsideXY( const FIntBox& Other ) const
	{
		return (IsInsideXY(Other.Min) && IsInsideXY(Other.Max));
	}

	/**
	 * Gets a bounding volume transformed by a matrix.
	 *
	 * @param M The matrix to transform by.
	 * @return The transformed box.
	 * @see TransformProjectBy
	 */
	TPCE_API FIntBox TransformBy( const FMatrix& M ) const;

	/**
	 * Gets a bounding volume transformed by a FTransform object.
	 *
	 * @param M The transformation object.
	 * @return The transformed box.
	 * @see TransformProjectBy
	 */
	TPCE_API FIntBox TransformBy( const FTransform& M ) const;

	/**
	 * Transforms and projects a world bounding box to screen space
	 *
	 * @param ProjM The projection matrix.
	 * @return The transformed box.
	 * @see TransformBy
	 */
	TPCE_API FIntBox TransformProjectBy( const FMatrix& ProjM ) const;

	/**
	 * Get a textual representation of this box.
	 *
	 * @return A string describing the box.
	 */
	FString ToString() const;

public:

	/**
	 * Utility function to build an AABB from Origin and Extent
	 *
	 * @param Origin The location of the bounding box.
	 * @param Extent Half size of the bounding box.
	 * @return A new axis-aligned bounding box.
	 */
	static FIntBox BuildAABB( const FIntVector& Origin, const FIntVector& Extent )
	{
		FIntBox NewBox(Origin - Extent, Origin + Extent);

		return NewBox;
	}

public:

	/**
	 * Serializes the bounding box.
	 *
	 * @param Ar The archive to serialize into.
	 * @param Box The box to serialize.
	 * @return Reference to the Archive after serialization.
	 */
	friend FArchive& operator<<( FArchive& Ar, FIntBox& Box )
	{
		return Ar << Box.Min << Box.Max << Box.IsValid;
	}

	/**
	 * Serializes the bounding box.
	 *
	 * @param Slot The structured archive slot to serialize into.
	 * @param Box The box to serialize.
	 */
	friend void operator<<(FStructuredArchive::FSlot Slot, FIntBox& Box)
	{
		FStructuredArchive::FRecord Record = Slot.EnterRecord();
		Record << SA_VALUE(TEXT("Min"), Box.Min) << SA_VALUE(TEXT("Max"), Box.Max) << SA_VALUE(TEXT("IsValid"), Box.IsValid);
	}

	bool Serialize( FArchive& Ar )
	{
		Ar << *this;
		return true;
	}

	bool Serialize( FStructuredArchive::FSlot Slot )
	{
		Slot << *this;
		return true;
	}
};


/**
 * FIntBox specialization for TIsPODType trait.
 */
template<> struct TIsPODType<FIntBox> { enum { Value = true }; };

/* FIntBox inline functions
 *****************************************************************************/

FORCEINLINE FIntBox& FIntBox::operator+=( const FIntVector &Other )
{
	if (IsValid)
	{
		Min.X = FMath::Min(Min.X, Other.X);
		Min.Y = FMath::Min(Min.Y, Other.Y);
		Min.Z = FMath::Min(Min.Z, Other.Z);

		Max.X = FMath::Max(Max.X, Other.X);
		Max.Y = FMath::Max(Max.Y, Other.Y);
		Max.Z = FMath::Max(Max.Z, Other.Z);
	}
	else
	{
		Min = Max = Other;
		IsValid = 1;
	}

	return *this;
}


FORCEINLINE FIntBox& FIntBox::operator+=( const FIntBox& Other )
{
	if (IsValid && Other.IsValid)
	{
		Min.X = FMath::Min(Min.X, Other.Min.X);
		Min.Y = FMath::Min(Min.Y, Other.Min.Y);
		Min.Z = FMath::Min(Min.Z, Other.Min.Z);

		Max.X = FMath::Max(Max.X, Other.Max.X);
		Max.Y = FMath::Max(Max.Y, Other.Max.Y);
		Max.Z = FMath::Max(Max.Z, Other.Max.Z);
	}
	else if (Other.IsValid)
	{
		*this = Other;
	}

	return *this;
}


FORCEINLINE FIntVector FIntBox::GetClosestPointTo( const FIntVector& Point ) const
{
	// start by considering the point inside the box
	FIntVector ClosestPoint = Point;

	// now clamp to inside box if it's outside
	if (Point.X < Min.X)
	{
		ClosestPoint.X = Min.X;
	}
	else if (Point.X > Max.X)
	{
		ClosestPoint.X = Max.X;
	}

	// now clamp to inside box if it's outside
	if (Point.Y < Min.Y)
	{
		ClosestPoint.Y = Min.Y;
	}
	else if (Point.Y > Max.Y)
	{
		ClosestPoint.Y = Max.Y;
	}

	// Now clamp to inside box if it's outside.
	if (Point.Z < Min.Z)
	{
		ClosestPoint.Z = Min.Z;
	}
	else if (Point.Z > Max.Z)
	{
		ClosestPoint.Z = Max.Z;
	}

	return ClosestPoint;
}


FORCEINLINE bool FIntBox::Intersect( const FIntBox& Other ) const
{
	if ((Min.X > Other.Max.X) || (Other.Min.X > Max.X))
	{
		return false;
	}

	if ((Min.Y > Other.Max.Y) || (Other.Min.Y > Max.Y))
	{
		return false;
	}

	if ((Min.Z > Other.Max.Z) || (Other.Min.Z > Max.Z))
	{
		return false;
	}

	return true;
}


FORCEINLINE bool FIntBox::IntersectXY( const FIntBox& Other ) const
{
	if ((Min.X > Other.Max.X) || (Other.Min.X > Max.X))
	{
		return false;
	}

	if ((Min.Y > Other.Max.Y) || (Other.Min.Y > Max.Y))
	{
		return false;
	}

	return true;
}


FORCEINLINE FString FIntBox::ToString() const
{
	return FString::Printf(TEXT("IsValid=%s, Min=(%s), Max=(%s)"), IsValid ? TEXT("true") : TEXT("false"), *Min.ToString(), *Max.ToString());
}
