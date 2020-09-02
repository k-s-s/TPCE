// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "ActorWidget.generated.h"

class UActorWidgetComponent;

/**
 *
 */
UCLASS(abstract)
class TPCE_API UActorWidget: public UUserWidget
{
	GENERATED_BODY()

public:

	UActorWidget(const FObjectInitializer& ObjectInitializer);

private:

	UPROPERTY(BlueprintReadOnly, Category = "ActorWidget", meta = (AllowPrivateAccess="true"))
	UActorWidgetComponent* ActorWidgetComponent;

protected:

	virtual void OnWidgetComponentChanged(UActorWidgetComponent* OldWidgetComponent);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName="OnWidgetComponentChanged"))
	void K2_OnWidgetComponentChanged(UActorWidgetComponent* OldWidgetComponent);

public:

	UFUNCTION(BlueprintCallable, Category = UserInterface)
	void SetWidgetComponent(UActorWidgetComponent* NewWidgetComponentOwner);

	FORCEINLINE UActorWidgetComponent* GetActorWidgetComponent() const { return ActorWidgetComponent; }

	FORCEINLINE UActorWidgetComponent* GetOwningComponent() const { return GetActorWidgetComponent(); }

};
