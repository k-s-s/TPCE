// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Object.h"
#include "Jobs/AIJobsComponent.h"

#include "AIJob.generated.h"

class UAIJobsComponent;
class FGameplayDebuggerCategory;

// UAIJob should technically be Within=UAIJobsComponent
// However this way it's possible to edit jobs in instanced actors, then pass them on to the jobs component
UCLASS(Blueprintable, Abstract, hidedropdown, EditInlineNew, ClassGroup="AI")
class TPCE_API UAIJob : public UObject
{
	GENERATED_BODY()

public:

	UAIJob();

	// Begin UObject Interface
	virtual void PostInitProperties() override;
	virtual void PostRename(UObject* OldOuter, const FName OldName) override;
	virtual void BeginDestroy();
	// End UObject Interface

	void BeginJob();
	void Tick(float DeltaSeconds);
	bool IsActive() const { return bActive; };
	bool CanCancel() const;
	float ScoreJob() const;
	void UpdateJobScore() { JobScore = ScoreJob(); }
	float GetJobScore() const { return JobScore; };
	FString GetJobName() const;

	/** If True, the job is removed from list after completion. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Jobs")
	bool bRunOnce;

	/** If True, the job won't be stopped even if there is one with a higher score. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Jobs")
	bool bRunUntilDone;

	/** Removes the job if it's inactive for too long. Time limit disabled if 0. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Jobs", meta=(ClampMin="0", UIMin="0"))
	float InactiveTimeLimit;

	/** Aborts the job if it runs for too long. Time limit disabled if 0. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Jobs", meta=(ClampMin="0", UIMin="0"))
	float RunningTimeLimit;

	/** Mark the job as finished and call OnEndJob. */
	UFUNCTION(BlueprintCallable, Category="AI|Jobs")
	void FinishJob();

	/** Get the time elapsed since the job was created or became inactive. */
	UFUNCTION(BlueprintCallable, Category="AI|Jobs")
	virtual float GetJobTimeInactive() const;

	/** Get the time elapsed since the job was started. */
	UFUNCTION(BlueprintCallable, Category="AI|Jobs")
	virtual float GetJobTimeActive() const;

#if WITH_GAMEPLAY_DEBUGGER
	virtual void DescribeSelfToGameplayDebugger(FGameplayDebuggerCategory* DebuggerCategory) const;
#endif // WITH_GAMEPLAY_DEBUGGER

protected:

	/** Overridable native function for when the job becomes active. */
	virtual void OnBeginJob() {}

	/** Overridable native function for when the job is finished or abandoned. */
	virtual void OnEndJob() {}

	/** Event called when the job becomes active. */
	UFUNCTION(BlueprintImplementableEvent, Category="AI Jobs", meta=(DisplayName="On Begin Job"))
	void ReceiveBeginJob();

	/** Event called when the job is finished or abandoned. */
	UFUNCTION(BlueprintImplementableEvent, Category="AI Jobs", meta=(DisplayName="On End Job"))
	void ReceiveEndJob();

	/** Event called every frame while the job is active. */
	UFUNCTION(BlueprintImplementableEvent, Category="AI Jobs", meta=(DisplayName="Tick"))
	void ReceiveTick(float DeltaSeconds);

	/** Scoring function for the job. Negative scoring jobs won't be considered. */
	UFUNCTION(BlueprintImplementableEvent, Category="AI Jobs", meta=(DisplayName="Score Job"))
	float ReceiveScoreJob() const;

	/** Whether the job is currently running. */
	UPROPERTY(Transient, BlueprintReadOnly, Category="AI Jobs")
	bool bActive;

	/** Last received job score. */
	UPROPERTY(Transient, BlueprintReadOnly, Category="AI Jobs")
	float JobScore;

	/** Jobs component that owns this job. */
	UPROPERTY(Transient, BlueprintReadOnly, Category="AI Jobs")
	UAIJobsComponent* JobsComponent;

	float TimeBecameActive;
	float TimeBecameInactive;
	mutable FString CachedJobName;

	friend class UAIJobsComponent;
};
