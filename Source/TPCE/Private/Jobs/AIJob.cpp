// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "Jobs/AIJob.h"

#include "Engine/World.h"
#include "Jobs/AIJobsComponent.h"
#include "VisualLogger/VisualLogger.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebuggerTypes.h"
#include "GameplayDebuggerCategory.h"
#endif

UAIJob::UAIJob()
	: bRunOnce(false)
	, bRunUntilDone(false)
	, InactiveTimeLimit(0.f)
	, RunningTimeLimit(0.f)
{
}

void UAIJob::PostInitProperties()
{
	Super::PostInitProperties();

	JobsComponent = Cast<UAIJobsComponent>(GetOuter());
}

void UAIJob::PostRename(UObject* OldOuter, const FName OldName)
{
	Super::PostRename(OldOuter, OldName);

	JobsComponent = Cast<UAIJobsComponent>(GetOuter());
}

void UAIJob::BeginDestroy()
{
	Super::BeginDestroy();

	if (bActive)
	{
		FinishJob();
	}
}

void UAIJob::BeginJob()
{
	check(JobsComponent);

	if (bActive)
	{
		UE_VLOG(this, LogAIJobs, Warning, TEXT("Tried to begin job that is already active"));
		return;
	}

	bActive = true;
	TimeBecameActive = GetWorld()->GetTimeSeconds();

	OnBeginJob();
	ReceiveBeginJob();
}

void UAIJob::FinishJob()
{
	if (!GetWorld())
	{
		// World is shutting down
		return;
	}

	if (!bActive)
	{
		UE_VLOG(this, LogAIJobs, Warning, TEXT("Tried to finish job that is not active"));
		return;
	}

	OnEndJob();
	ReceiveEndJob();

	bActive = false;
	TimeBecameInactive = GetWorld()->GetTimeSeconds();
}

void UAIJob::Tick(float DeltaSeconds)
{
	ReceiveTick(DeltaSeconds);
}

bool UAIJob::CanCancel() const
{
	if (!bActive)
	{
		return false;
	}

	if (RunningTimeLimit > 0.f && GetJobTimeActive() > RunningTimeLimit)
	{
		return true;
	}

	return !bRunUntilDone;
}

FString UAIJob::GetJobName() const
{
	if (CachedJobName.Len() == 0)
	{
		CachedJobName = GetClass()->GetName();
		CachedJobName.RemoveFromEnd(TEXT("_C"));

		int32 SeparatorIdx = INDEX_NONE;
		const bool bHasSeparator = CachedJobName.FindLastChar(TEXT('_'), SeparatorIdx);
		if (bHasSeparator)
		{
			CachedJobName = CachedJobName.Mid(SeparatorIdx + 1);
		}
	}

	return CachedJobName;
}

float UAIJob::ScoreJob() const
{
	check(JobsComponent);

	if (bActive && RunningTimeLimit > 0.f && GetJobTimeActive() > RunningTimeLimit)
	{
		// Over the time limit. Minimum score so another job can start
		return 0.f;
	}

	return ReceiveScoreJob();
}

float UAIJob::GetJobTimeActive() const
{
	return bActive ? (GetWorld()->GetTimeSeconds() - TimeBecameActive) : 0.f;
}

float UAIJob::GetJobTimeInactive() const
{
	return !bActive ? (GetWorld()->GetTimeSeconds() - TimeBecameInactive) : 0.f;
}

#if WITH_GAMEPLAY_DEBUGGER

void UAIJob::DescribeSelfToGameplayDebugger(FGameplayDebuggerCategory* DebuggerCategory) const
{
}

#endif // WITH_GAMEPLAY_DEBUGGER
