// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/MovementComponent.h"

#include "PushToTargetComponent.generated.h"

UENUM(BlueprintType)
enum class EPushToTargetRotationType : uint8
{
	/** Do not rotate. */
	None,
	/** Face the target. */
	FaceTarget,
	/** Face the local player's view location. */
	FaceView,
};

/**
 * Updates the position of another component simulating an unrealistic attraction force towards a target.
 * This 'force' becomes stronger with distance and ignores both inertia and gravity. Supports sliding over
 * surfaces and can simulate friction.
 *
 * Normally the root component of the owning actor is moved. Does not affect components that are simulating physics.
 */
UCLASS(ClassGroup=Movement, meta=(BlueprintSpawnableComponent), hideCategories=(Velocity))
class TPCE_API UPushToTargetComponent: public UMovementComponent
{
	GENERATED_BODY()

public:

	UPushToTargetComponent();

private:

	/** The target component we are homing towards. Can only be set at runtime (when the component is spawned or updating). */
	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category=PushToTarget, meta=(AllowPrivateAccess="true"))
	TWeakObjectPtr<USceneComponent> TargetComponent;

	/** The target location we are homing towards, if set. Can only be set at runtime (when the component is spawned or updating). */
	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category=PushToTarget, meta=(AllowPrivateAccess="true"))
	FVector TargetLocation;

	/** The socket name of the current target we are homing towards. Can only be set at runtime (when the component is spawned or updating). */
	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category=PushToTarget, meta=(AllowPrivateAccess="true"))
	FName TargetSocketName;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category=PushToTarget, meta=(AllowPrivateAccess="true"))
	bool bIsBlocked;

	/**
	 * Draws markers at the target (in green) and the lagged position (in yellow).
	 * A line is drawn between the two locations, in green normally but in red if the distance to the lag target has been clamped (by TargetLagMaxDistance).
	 */
	UPROPERTY(EditDefaultsOnly, Category=PushToTarget, AdvancedDisplay)
	bool bDrawDebugMarkers;

	FVector PreviousDesiredLocation;
	FRotator PreviousDesiredRotation;
	bool bTargetLocationSet;

protected:

	/** Minimum delta time considered when ticking. Delta times below this are not considered. This is a very small non-zero positive value to avoid potential divide-by-zero in simulation code. */
	static const float MIN_TICK_TIME;

	/** Return the target location considering the socket name if any. */
	FVector GetTargetLocation() const;

	/** Return the target rotation considering the current rotation behavior. */
	FRotator GetTargetRotation(const FVector& InCurrentLocation) const;

	/**
	 * Return the location of the UpdatedComponent in world space to be used for this frame.
	 * Default implementation simply returns the UpdatedComponent's location unmodified, but derived classes can override
	 * this method to implement diferent rules depending on the state of the components involved, for example, producing
	 * a boost or a drag effect.
	 */
	virtual FVector AdjustCurrentLocationToTarget(const FVector& InCurrentLocation, const FVector& InTargetLocation) const;

	/**
	 * Return the rotation of the UpdatedComponent in world space to be used for this frame.
	 * Default implementation simply returns the UpdatedComponent's rotation unmodified.
	 */
	virtual FRotator AdjustCurrentRotationToTarget(const FRotator& InCurrentRotation, const FRotator& InTargetRotation) const;

	/** Interpolation method used for target location lag. */
	virtual FVector VInterpTo(const FVector& Current, const FVector& Target, float DeltaTime, float InterpSpeed);

	/** Interpolation method used for target rotation lag. */
	virtual FRotator RInterpTo(const FRotator& Current, const FRotator& Target, float DeltaTime, float InterpSpeed);

	/**
	 * Move the updated component accounting for initial penetration, blocking collisions and sliding surfaces.
	 *
	 * @return true if the movement could be performed and the involved components remain valid, otherwise false.
	 */
	virtual bool MoveUpdatedComponent(const FVector& Delta, const FQuat& NewRotation);

public:

	/** Offset in local space of the targeted socket or component. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=PushToTarget)
	FVector SocketRelativeOffset;

	/** Offset in local space of the targeted actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=PushToTarget)
	FVector ActorRelativeOffset;

	/** Offset in local space of the controller possessing the targeted actor, if available. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=PushToTarget)
	FVector ControllerRelativeOffset;

	/** Offset in world space. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=PushToTarget)
	FVector WorldOffset;

	/**
	 * If true, the updated component lags behind the target to smooth its movement.
	 * @see Speed
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=PushToTarget)
	bool bEnableLag;

	/**
	 * Controls how strong the attraction is, thus how quickly the updated component reaches the target. Low values are slower (more drag), high values are faster (less drag), while zero is instant (no drag).
	 * If the updated componet is simulating physics
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=PushToTarget, meta=(EditCondition="bEnableLag", ClampMin="0.0", ClampMax="10000.0", UIMin="0.0", UIMax="10000.0"))
	float Speed;

	/**
	 * Controls rotation interpolation speed. Low values are slower (more lag), high values are faster (less lag), while zero is instant (no lag).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=PushToTarget, meta=(EditCondition="bEnableLag", ClampMin="0.0", ClampMax="10000.0", UIMin="0.0", UIMax="10000.0"))
	float RotationSpeed;

	/** Rotation behaviour of the component. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=PushToTarget)
	EPushToTargetRotationType RotationType;

	/** If true, the updated component will slide over surfaces when blocked. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=PushToTarget)
	bool bSlide;

	/** A normalized coefficient of friction representing the ammount of movement lost due to friction when sliding. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=PushToTarget, meta=(EditCondition="bSlide", ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float Friction;

	/** Makes the component stay upright in world space, regardless of the rotation mode. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=PushToTarget)
	bool bStayUpright;

	/** If true, the updated component is teleported immediately to the new homing target when assigned. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=PushToTarget)
	bool bTeleportToTargetToStart;

	/**
	 * If true, forces sub-stepping to break up movement into discrete smaller steps to improve accuracy of the trajectory.
	 * Objects that move in a straight line typically do *not* need to set this, as movement always uses continuous collision detection (sweeps).
	 * @see MaxSimulationTimeStep, MaxSimulationIterations
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=PushToTargetSimulation, AdvancedDisplay)
	bool bForceSubStepping;

	/**
	 * Max time delta for each discrete simulation step.
	 * Lowering this value can address issues with fast-moving objects or complex collision scenarios, at the cost of performance.
	 *
	 * WARNING: if (MaxSimulationTimeStep * MaxSimulationIterations) is too low for the min framerate, the last simulation step may exceed MaxSimulationTimeStep to complete the simulation.
	 * @see MaxSimulationIterations, bForceSubStepping
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0166", ClampMax="0.50", UIMin="0.0166", UIMax="0.50"), Category=PushToTargetSimulation, AdvancedDisplay)
	float MaxSimulationTimeStep;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void InitializeComponent() override;
	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;

	/** Return true if still in the world.  It will check things like the KillZ, outside world bounds, etc. and handle the situation. */
	virtual bool IsStillInWorld();

	/** Sets the target component or socket and clears the target world location. */
	UFUNCTION(BlueprintCallable, Category="Game|Components|PushToTarget")
	virtual void SetTargetComponent(USceneComponent* NewTargetComponent, const FName SocketName = NAME_None);

	/** Sets the target world location and clears the target component. */
	UFUNCTION(BlueprintCallable, Category="Game|Components|PushToTarget")
	virtual void SetTargetLocation(FVector NewTargetLocation);

	/** Clears both the target component and target world location. */
	UFUNCTION(BlueprintCallable, Category="Game|Components|PushToTarget")
	void ClearTarget();

	/** Immediately moves to the target location and rotation, according to the current settings. */
	UFUNCTION(BlueprintCallable, Category="Game|Components|PushToTarget")
	void SnapToTarget();

	UFUNCTION(BlueprintCallable, Category="Game|Components|PushToTarget")
	void GetDesiredLocationAndRotation(FVector& OutLocation, FRotator& OutRotation) const;

	FORCEINLINE USceneComponent* GetTargetComponent() { return TargetComponent.Get(); }

	FORCEINLINE FName GetTargetSocketName() { return TargetSocketName; }

	FORCEINLINE USceneComponent* GetUpdatedComponent() { return UpdatedComponent; }

	FORCEINLINE UPrimitiveComponent* GetUpdatedPrimitive() { return UpdatedPrimitive; }

	FORCEINLINE bool IsBlocked() const { return bIsBlocked; }
};
