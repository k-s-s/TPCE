// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "AIController.h"

#include "ExtAIController.generated.h"

class UAIJobsComponent;

/**
 * Base class for AI controllers with extended functionality.
 */
UCLASS()
class TPCE_API AExtAIController : public AAIController
{
	GENERATED_BODY()

public:

	AExtAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Begin AActor Interface
	virtual void PostRegisterAllComponents() override;
	// End AActor Interface

	// Begin AController Interface
	virtual void OnUnPossess() override;
	// End AController Interface

	void SetJobsComponent(UAIJobsComponent& InPerceptionComponent);

	/** Returns the jobs subobject. */
	UAIJobsComponent* GetJobsComponent() { return JobsComponent; }

protected:

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="AI", meta=(AllowPrivateAccess="true"))
	UAIJobsComponent* JobsComponent;
};
