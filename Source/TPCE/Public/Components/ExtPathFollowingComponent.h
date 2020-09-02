// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

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
