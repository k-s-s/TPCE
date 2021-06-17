// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Templates/SubclassOf.h"

#include "AIJobsComponent.generated.h"

TPCE_API DECLARE_LOG_CATEGORY_EXTERN(LogAIJobs, Display, All);

class AAIController;
class UAIJob;
class FGameplayDebuggerCategory;

UCLASS(ClassGroup=AI, HideCategories=(Activation, Collision), meta=(BlueprintSpawnableComponent), config=Game)
class TPCE_API UAIJobsComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	typedef TArray<UAIJob*> TJobArray;

protected:

	/** Available jobs to poll from. */
	UPROPERTY(EditDefaultsOnly, Instanced, Category="AI Jobs")
	TArray<UAIJob*> AvailableJobs;

	/** Amount of time between job evaluations. Use SetUpdateInterval() to adjust this at runtime. A value <= 0 prevents any updates. */
	UPROPERTY(EditDefaultsOnly, Category="AI Jobs", meta=(ClampMin="0", UIMin="0"))
	float UpdateInterval;

	/** Current ongoing job. */
	UPROPERTY(Transient)
	UAIJob* CurrentJob;

	UPROPERTY(Transient)
	AAIController* AIOwner;

public:

	UAIJobsComponent();

	// Begin UActorComponent Interface
	virtual void OnRegister() override;
	virtual void InitializeComponent() override;
	// End UActorComponent Interface

	void MoveJobs(TArray<UAIJob*>& InJobs);
	FORCEINLINE TJobArray::TIterator GetAvailableJobsIterator() { return TJobArray::TIterator(AvailableJobs); }
	FORCEINLINE TJobArray::TConstIterator GetAvailableJobsConstIterator() const { return TJobArray::TConstIterator(AvailableJobs); }

	/** Create and add a new job of the given class. */
	UFUNCTION(BlueprintCallable, Category="AI|Jobs", meta=(DeterminesOutputType="JobClass"))
	UAIJob* AddJob(TSubclassOf<UAIJob> JobClass);

	/** Remove the given job and return True if successful. */
	UFUNCTION(BlueprintCallable, Category="AI|Jobs")
	bool RemoveJob(UAIJob* Job);

	/** Get the outer AI controller if available. */
	UFUNCTION(BlueprintCallable, Category="AI|Jobs")
	AAIController* GetAIOwner() const { return AIOwner; }

	/** Get the outer controller's pawn if available, otherwise the outer as an actor. */
	UFUNCTION(BlueprintCallable, Category="AI|Jobs")
	AActor* GetBodyActor() const;

	/**
	 * Changes the UpdateInterval.
	 * If we are currently waiting for an interval, this can either extend or shorten that interval.
	 * A value <= 0 prevents any updates.
	 */
	UFUNCTION(BlueprintCallable, Category="AI|Jobs")
	virtual void SetUpdateInterval(const float NewUpdateInterval);

	UFUNCTION(BlueprintCallable, Category="AI|Jobs")
	virtual void AbandonCurrentJob();

#if WITH_GAMEPLAY_DEBUGGER
	virtual void DescribeSelfToGameplayDebugger(FGameplayDebuggerCategory* DebuggerCategory) const;
#endif // WITH_GAMEPLAY_DEBUGGER

protected:

	/** Select and begin the most suitable job for the moment. */
	virtual void EvaluateJobs();

	/** Modify the timer to fire in TimeDelay seconds. A value <= 0 disables the timer. */
	virtual void SetTimer(const float TimeDelay);

	/** Timer handle for EvaluateJobs function. */
	FTimerHandle TimerHandle_EvaluateJobs;
};
