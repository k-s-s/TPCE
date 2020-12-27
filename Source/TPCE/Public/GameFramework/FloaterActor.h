// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"

#include "FloaterActor.generated.h"

UENUM()
enum class EFloaterShowState : uint8
{
	Hidden,
	Showing,
	Shown,
	Hiding,
};

/**
 * Actor that is a convenient base for floating menus and icons. Can be set to follow a player's pawn and face their view.
 * Use Show() and Hide() to have it pop out and back in.
 */
UCLASS()
class TPCE_API AFloaterActor : public AActor
{
	GENERATED_BODY()

public:
	AFloaterActor();

	// Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetActorHiddenInGame(bool bNewHidden) override;
	// End AActor Interface

	/** Begins showing this actor. */
	UFUNCTION(BlueprintCallable, Category=Floater)
	void Show();

	/** Stops showing this actor. */
	UFUNCTION(BlueprintCallable, Category=Floater)
	void Hide();

	/** Play rate for the pop up and hide animations. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Floater, meta=(ClampMin=0))
	float AnimationRate;

	/** Animation for the actor appearing. RGB is scale, A is vertical offset. */
	UPROPERTY(EditDefaultsOnly, Category=Floater, AdvancedDisplay)
	class UCurveLinearColor* PopUpCurve;

	/** Animation for the actor disappearing. RGB is scale, A is vertical offset. */
	UPROPERTY(EditDefaultsOnly, Category=Floater, AdvancedDisplay)
	class UCurveLinearColor* PopInCurve;

	/** Returns the movement component subobject. */
	class UPushToTargetComponent* GetPushToTarget() const { return PushToTarget; }

protected:
	/** Current animation phase. */
	UPROPERTY(Transient, BlueprintReadOnly, Category=Floater)
	EFloaterShowState ShowState;

	/** Movement component. */
	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	class UPushToTargetComponent* PushToTarget;

	/** Name of the movement component. */
	static FName PushToTargetName;

	void OnTimelineProgress(FLinearColor Value);
	void OnTimelineFinished();

	FTimeline PopUpTimeline;
	FTimeline PopInTimeline;
};
