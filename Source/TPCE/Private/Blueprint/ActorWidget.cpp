// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "Blueprint/ActorWidget.h"

UActorWidget::UActorWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UActorWidget::OnWidgetComponentChanged(UActorWidgetComponent* OldWidgetComponent)
{

}

void UActorWidget::SetWidgetComponent(UActorWidgetComponent* NewWidgetComponent)
{
	if (ActorWidgetComponent != NewWidgetComponent)
	{
		auto OldWidgetComponent = ActorWidgetComponent;
		ActorWidgetComponent = NewWidgetComponent;
		OnWidgetComponentChanged(OldWidgetComponent);
		K2_OnWidgetComponentChanged(OldWidgetComponent);
	}
}

