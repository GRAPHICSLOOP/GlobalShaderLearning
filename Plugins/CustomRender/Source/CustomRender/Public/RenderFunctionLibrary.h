// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "CustomSimpleRender.h"
#include "Engine/Texture2D.h"
#include "Factories/TextureFactory.h"
#include "Misc/FeedbackContext.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorFramework/AssetImportData.h"
#include "Misc/FileHelper.h"
#include "RenderFunctionLibrary.generated.h"

#define LOCTEXT_NAMESPACE "RenderFunctionLibrary"

/**
 * 
 */
UCLASS(MinimalAPI, meta = (ScriptName = "RenderShaderLibary"))
class URenderFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()
	UFUNCTION(BlueprintCallable, Category = "RenderShaderLibaryPlugin", meta = (WorldContext = "WorldContextObject"))
	static void DrawTestShaderRenderTarget(class UTextureRenderTarget2D* OutputRenderTarget, AActor* AC,
											FLinearColor Color, UTexture2D* Texture, FCustomUniformData Data);

	UFUNCTION(BlueprintCallable, Category = "RenderShaderLibaryPlugin", meta = (WorldContext = "WorldContextObject"))
	static void TextureWriting(UTexture2D* TextureToBeWrite, AActor* AC);

	UFUNCTION(BlueprintCallable, Category = "RenderShaderLibaryPlugin", meta = (WorldContext = "WorldContextObject"))
	static void LoadTexture2DFormFile(const FString& Filename,const FString& PackageName);
};

UObject* CustomCreateBinary(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
							UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd,
							FFeedbackContext* Warn ,const FString& Filename);

bool ImportImage(const uint8* Buffer, uint32 Length,FFeedbackContext* Warn, FImportImage& OutImage);


#undef LOCTEXT_NAMESPACE
