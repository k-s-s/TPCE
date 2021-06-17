// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "GameFramework/ExtAIController.h"

#include "Components/ExtPathFollowingComponent.h"
#include "Jobs/AIJobsComponent.h"
#include "VisualLogger/VisualLogger.h"

AExtAIController::AExtAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UExtPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
}

void AExtAIController::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	// Cache JobsComponent if not already set
	if (JobsComponent == nullptr || JobsComponent->IsPendingKill() == true)
	{
		JobsComponent = FindComponentByClass<UAIJobsComponent>();
	}
}

void AExtAIController::SetJobsComponent(UAIJobsComponent& InJobsComponent)
{
	if (JobsComponent != nullptr)
	{
		UE_VLOG(this, LogAIJobs, Warning, TEXT("Setting jobs component while AIController already has one!"));
	}
	JobsComponent = &InJobsComponent;
}

void AExtAIController::OnUnPossess()
{
	if (JobsComponent)
	{
		JobsComponent->AbandonCurrentJob();
	}

	Super::OnUnPossess();
}
