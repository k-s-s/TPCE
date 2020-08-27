// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/WidgetBlueprintLibrary.h"

#include "WidgetBlueprintLibraryExtensions.generated.h"

class UTextureRenderTarget2D;

UCLASS()
class TPCE_API UWidgetBlueprintLibraryEx : public UWidgetBlueprintLibrary
{
	GENERATED_UCLASS_BODY()

public:

	/** 
	 * Creates a Slate Brush from a TextureRenderTarget2D
	 *
	 * @param Width  When less than or equal to zero, the Width of the brush will default to the Width of the Texture
	 * @param Height  When less than or equal to zero, the Height of the brush will default to the Height of the Texture
	 *
	 * @return A new slate brush using the texture render target.
	 */
	UFUNCTION(BlueprintPure, Category="Widget|Brush")
	static FSlateBrush MakeBrushFromTextureRenderTarget(UTextureRenderTarget2D* Texture, int32 Width = 0, int32 Height = 0);
};
