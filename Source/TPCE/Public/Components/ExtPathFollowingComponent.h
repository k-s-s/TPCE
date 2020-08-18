// Copyright (c) 2020 greisane <ggreisane@gmail.com>

#pragma once

#include "CoreMinimal.h"
#include "Navigation/PathFollowingComponent.h"

#include "ExtPathFollowingComponent.generated.h"

/**
 * Path following component with extended functionality.
 * Improved focal point of movement.
 */
UCLASS()
class TPCE_API UExtPathFollowingComponent : public UPathFollowingComponent
{
	GENERATED_BODY()

public:
	UExtPathFollowingComponent();

	// Begin UPathFollowingComponent Interface
	virtual FVector GetMoveFocus(bool bAllowStrafe) const override;
	// End UPathFollowingComponent Interface
};
