// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "Jobs/AIJobsComponent.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "AIController.h"
#include "GameFramework/ExtAIController.h"
#include "Jobs/AIJob.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebuggerTypes.h"
#include "GameplayDebuggerCategory.h"
#endif

DEFINE_LOG_CATEGORY(LogAIJobs);

UAIJobsComponent::UAIJobsComponent()
	: UpdateInterval(.2f)
{
	bWantsInitializeComponent = true;
}

void UAIJobsComponent::OnRegister()
{
	Super::OnRegister();

	AIOwner = Cast<AAIController>(GetOwner());

	if (AExtAIController* ExtAIOwner = Cast<AExtAIController>(GetOwner()))
	{
		// This should not be needed but aparently AAIController::PostRegisterAllComponents gets called component's OnRegister
		ensure(ExtAIOwner->GetJobsComponent() == nullptr
			|| ExtAIOwner->GetJobsComponent() == this
			|| (ExtAIOwner->GetWorld() && ExtAIOwner->GetWorld()->WorldType != EWorldType::Editor));
		if (ExtAIOwner->GetJobsComponent() == nullptr)
		{
			ExtAIOwner->SetJobsComponent(*this);
		}
	}

	for (UAIJob* Job : AvailableJobs)
	{
		Job->TimeBecameInactive = GetWorld()->GetTimeSeconds();
	}
}

void UAIJobsComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (UpdateInterval > 0.f)
	{
		// Stagger initial updates to avoid hitches
		const float InitialDelay = UpdateInterval * FMath::SRand() + KINDA_SMALL_NUMBER;
		SetTimer(InitialDelay);
	}
}

void UAIJobsComponent::SetTimer(const float TimeInterval)
{
	// Only necessary to update if we are the server
	const AActor* Owner = GetOwner();
	if (Owner && GEngine->GetNetMode(GetWorld()) < NM_Client)
	{
		Owner->GetWorldTimerManager().SetTimer(TimerHandle_EvaluateJobs, this, &UAIJobsComponent::EvaluateJobs, TimeInterval, false);
	}
}

void UAIJobsComponent::SetUpdateInterval(const float NewUpdateInterval)
{
	if (UpdateInterval != NewUpdateInterval)
	{
		UpdateInterval = NewUpdateInterval;

		const AActor* Owner = GetOwner();
		if (IsValid(Owner))
		{
			if (UpdateInterval <= 0.f)
			{
				SetTimer(0.f);
			}
			else
			{
				float CurrentElapsed = Owner->GetWorldTimerManager().GetTimerElapsed(TimerHandle_EvaluateJobs);
				CurrentElapsed = FMath::Max(0.f, CurrentElapsed);

				if (CurrentElapsed < UpdateInterval)
				{
					// Extend lifetime by remaining time
					SetTimer(UpdateInterval - CurrentElapsed);
				}
				else if (CurrentElapsed > UpdateInterval)
				{
					// Basically fire next update, because time has already expired
					// Don't want to fire immediately in case an update tries to change the interval, looping endlessly
					SetTimer(KINDA_SMALL_NUMBER);
				}
			}
		}
	}
}

AActor* UAIJobsComponent::GetBodyActor() const
{
	if (AController* OwnerController = Cast<AController>(GetOuter()))
	{
		return OwnerController->GetPawn();
	}
	return Cast<AActor>(GetOuter());
}

void UAIJobsComponent::AbandonCurrentJob()
{
	if (CurrentJob && CurrentJob->IsActive())
	{
		CurrentJob->FinishJob();
	}
}

void UAIJobsComponent::MoveJobs(TArray<UAIJob*>& InJobs)
{
	for (UAIJob* Job : InJobs)
	{
		Job->Rename(nullptr, this, REN_ForceNoResetLoaders | REN_DoNotDirty);
		Job->TimeBecameInactive = GetWorld()->GetTimeSeconds();
		AvailableJobs.Add(Job);
	}
	InJobs.Empty();
}

UAIJob* UAIJobsComponent::AddJob(TSubclassOf<UAIJob> JobClass)
{
	UAIJob* NewJob = NewObject<UAIJob>(this, JobClass);
	NewJob->TimeBecameInactive = GetWorld()->GetTimeSeconds();
	AvailableJobs.Add(NewJob);
	return NewJob;
}

bool UAIJobsComponent::RemoveJob(UAIJob* Job)
{
	return (bool)AvailableJobs.RemoveSingleSwap(Job);
}

void UAIJobsComponent::EvaluateJobs()
{
	if (CurrentJob && !CurrentJob->IsActive() && CurrentJob->bRunOnce)
	{
		RemoveJob(CurrentJob);
		CurrentJob = nullptr;
	}

	UAIJob* BestNewJob = nullptr;
	float BestJobScore = -1.f;
	for (int32 JobIdx = 0; JobIdx < AvailableJobs.Num(); JobIdx++)
	{
		UAIJob* Job = AvailableJobs[JobIdx];

		if (Job->InactiveTimeLimit > 0.f && Job->GetJobTimeInactive() > Job->InactiveTimeLimit)
		{
			AvailableJobs.RemoveAtSwap(JobIdx--);
			continue;
		}

		Job->UpdateJobScore();
		const float JobScore = Job->GetJobScore();

		if (JobScore >= 0.f && JobScore > BestJobScore)
		{
			BestNewJob = Job;
			BestJobScore = JobScore;
		}
	}

	if (BestNewJob && BestNewJob != CurrentJob && (!CurrentJob || !CurrentJob->IsActive() || CurrentJob->CanCancel()))
	{
		AbandonCurrentJob();

		CurrentJob = BestNewJob;
		CurrentJob->BeginJob();
	}

	SetTimer(UpdateInterval);
}

#if WITH_GAMEPLAY_DEBUGGER

void UAIJobsComponent::DescribeSelfToGameplayDebugger(FGameplayDebuggerCategory* DebuggerCategory) const
{
	if (DebuggerCategory == nullptr)
	{
		return;
	}

	TArray<UAIJob*> SortedJobs = AvailableJobs;
	SortedJobs.Sort([](const auto& A, const auto& B)
		{
			return A.GetJobScore() > B.GetJobScore();
		});

	for (const UAIJob* Job : SortedJobs)
	{
		const float JobScore = Job->GetJobScore();
		FString Description = FString::Printf(TEXT("%s: {%s}%.2f{white}"), *Job->GetJobName(), (JobScore >= 0.f) ? TEXT("yellow") : TEXT("grey"), JobScore);
		if (!Job->IsActive() && Job->InactiveTimeLimit > 0.f)
		{
			Description += FString::Printf(TEXT(" ({yellow}%.2fs{white} left)"), FMath::Max(0.f, Job->InactiveTimeLimit - Job->GetJobTimeInactive()));
		}
		DebuggerCategory->AddTextLine(Description);
	}

	DebuggerCategory->AddTextLine(TEXT("--"));

	if (CurrentJob && CurrentJob->IsActive())
	{
		const float JobTime = CurrentJob->GetJobTimeActive();
		DebuggerCategory->AddTextLine(FString::Printf(TEXT("Active job: {green}%s{white}, Time: {%s}%.2f{white}, Cancellable: {yellow}%s"),
			*CurrentJob->GetJobName(),
			(CurrentJob->RunningTimeLimit > 0.f && JobTime > CurrentJob->RunningTimeLimit) ? TEXT("orange") : TEXT("yellow"),
			JobTime,
			CurrentJob->CanCancel() ? TEXT("Yes") : TEXT("No")));
		CurrentJob->DescribeSelfToGameplayDebugger(DebuggerCategory);
	}
	else
	{
		DebuggerCategory->AddTextLine(TEXT("No active job."));
	}
}

#endif // WITH_GAMEPLAY_DEBUGGER
