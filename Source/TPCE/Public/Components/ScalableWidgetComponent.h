// Copyright (c) 2020 greisane <ggreisane@gmail.com>

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"

#include "ScalableWidgetComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDrawSizeChanged, FIntPoint, NewDrawSize);

/** Widget component with scalable internal resolution. */
UCLASS(Blueprintable, ClassGroup="UserInterface", hidecategories=(Object,Activation,"Components|Activation",Sockets,Base,Lighting,LOD,Mesh), editinlinenew, meta=(BlueprintSpawnableComponent) )
class TPCE_API UScalableWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UScalableWidgetComponent();

	// Begin UWidgetComponent Interface
	virtual void UpdateRenderTarget(FIntPoint DesiredRenderTargetSize) override;
	// End UWidgetComponent Interface

	UPROPERTY(BlueprintAssignable, Category=UserInterface)
	FDrawSizeChanged OnDrawSizeChanged;

	/** Gets the render scale for the widget. */
	UFUNCTION(BlueprintCallable, Category=UserInterface)
	float GetRenderScale() const { return RenderScale; }

	/** Sets the render scale to use for this widget. */
	UFUNCTION(BlueprintCallable, Category=UserInterface)
	void SetRenderScale(const float NewRenderScale);

protected:
	/** Scale of internal render target. */
	UPROPERTY(EditAnywhere, Category=UserInterface, meta=(ClampMin=0))
	float RenderScale;

	virtual void DrawWidgetToRenderTarget(float DeltaTime) override;
	bool UpdateDrawSize();
};
