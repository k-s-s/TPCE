// Fill out your copyright notice in the Description page of Project Settings.

#include "Blueprint/WidgetBlueprintLibraryExtensions.h"

#include "Brushes/SlateNoResource.h"
#include "Engine/TextureRenderTarget2D.h"

UWidgetBlueprintLibraryEx::UWidgetBlueprintLibraryEx(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FSlateBrush UWidgetBlueprintLibraryEx::MakeBrushFromTextureRenderTarget(UTextureRenderTarget2D* Texture, int32 Width, int32 Height)
{
	if (Texture)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(Texture);
		Width = (Width > 0) ? Width : Texture->SizeX;
		Height = (Height > 0) ? Height : Texture->SizeY;
		Brush.ImageSize = FVector2D(Width, Height);
		return Brush;
	}

	return FSlateNoResource();
}
