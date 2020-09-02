// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "Components/ScalableWidgetComponent.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Slate/WidgetRenderer.h"
#include "Widgets/SWindow.h"

UScalableWidgetComponent::UScalableWidgetComponent()
	: RenderScale(1.0f)
{
}

void UScalableWidgetComponent::SetRenderScale(const float NewRenderScale)
{
	if (RenderScale != NewRenderScale)
	{
		RenderScale = NewRenderScale;
		if (IsRegistered())
		{
			MarkRenderStateDirty();
		}
	}
}

void UScalableWidgetComponent::UpdateRenderTarget(FIntPoint DesiredRenderTargetSize)
{
	// This depends on an engine modification (fix) to work correctly
	// See https://github.com/greisane/UnrealEngine/commit/dd0cee91f4c54225f054283c36127c1c213c8f07
	Super::UpdateRenderTarget(FIntPoint(DesiredRenderTargetSize.X * RenderScale, DesiredRenderTargetSize.Y * RenderScale));
}

bool UScalableWidgetComponent::UpdateDrawSize()
{
	if (GUsingNullRHI)
	{
		return false;
	}

	if (!SlateWindow.IsValid())
	{
		return false;
	}

	const int32 MaxAllowedDrawSize = GetMax2DTextureDimension();
	if (DrawSize.X <= 0 || DrawSize.Y <= 0 || DrawSize.X > MaxAllowedDrawSize || DrawSize.Y > MaxAllowedDrawSize)
	{
		return false;
	}

	if (bDrawAtDesiredSize)
	{
		SlateWindow->SlatePrepass(RenderScale);

		FVector2D DesiredSize = SlateWindow->GetDesiredSize();
		DesiredSize.X = FMath::RoundToInt(DesiredSize.X);
		DesiredSize.Y = FMath::RoundToInt(DesiredSize.Y);
		CurrentDrawSize = DesiredSize.IntPoint();

		WidgetRenderer->SetIsPrepassNeeded(false);
	}
	else
	{
		CurrentDrawSize = DrawSize;

		WidgetRenderer->SetIsPrepassNeeded(true);
	}

	return true;
}

void UScalableWidgetComponent::DrawWidgetToRenderTarget(float DeltaTime)
{
	const FIntPoint PreviousDrawSize = CurrentDrawSize;

	if (!UpdateDrawSize())
	{
		return;
	}

	if (CurrentDrawSize != PreviousDrawSize)
	{
		UpdateBodySetup(true);
		RecreatePhysicsState();
	}

	UpdateRenderTarget(CurrentDrawSize);

	// The render target could be null if the current draw size is zero
	if (RenderTarget)
	{
		bRedrawRequested = false;

		// greisane: Render at render target size instead of CurrentDrawSize
		WidgetRenderer->DrawWindow(
			RenderTarget,
			SlateWindow->GetHittestGrid(),
			SlateWindow.ToSharedRef(),
			RenderScale,
			FVector2D(RenderTarget->SizeX, RenderTarget->SizeY),
			DeltaTime);

		LastWidgetRenderTime = GetCurrentTime();
	}

	if (CurrentDrawSize != PreviousDrawSize)
	{
		OnDrawSizeChanged.Broadcast(CurrentDrawSize);
	}
}
