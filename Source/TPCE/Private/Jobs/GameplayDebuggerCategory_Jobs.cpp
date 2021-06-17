// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "GameplayDebuggerCategory_Jobs.h"

#if WITH_GAMEPLAY_DEBUGGER

#include "GameFramework/Pawn.h"
#include "GameFramework/ExtAIController.h"
#include "Jobs/AIJobsComponent.h"

FGameplayDebuggerCategory_Jobs::FGameplayDebuggerCategory_Jobs()
{
}

TSharedRef<FGameplayDebuggerCategory> FGameplayDebuggerCategory_Jobs::MakeInstance()
{
	return MakeShareable(new FGameplayDebuggerCategory_Jobs());
}

void FGameplayDebuggerCategory_Jobs::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	if (APawn* MyPawn = Cast<APawn>(DebugActor))
	{
		const UAIJobsComponent* JobsComponent = nullptr;

		if (AExtAIController* MyController = MyPawn->GetController<AExtAIController>())
		{
			JobsComponent = MyController->GetJobsComponent();
		}

		if (JobsComponent == nullptr)
		{
			if (AAIController* MyController = MyPawn->GetController<AAIController>())
			{
				JobsComponent = MyPawn->FindComponentByClass<UAIJobsComponent>();
			}
		}

		if (JobsComponent)
		{
			JobsComponent->DescribeSelfToGameplayDebugger(this);
		}
	}
}

#endif // WITH_GAMEPLAY_DEBUGGER
