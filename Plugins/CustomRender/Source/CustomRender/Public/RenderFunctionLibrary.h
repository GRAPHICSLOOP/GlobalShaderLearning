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
	static void UseGlobalShader(class UTextureRenderTarget2D* TextureRenderTarget, AActor* Actor,
											FLinearColor Color, UTexture2D* Texture, FCustomUniformData Data);

	
	/**
	 * @brief 自定义ComputeShader
	 * @param TextureRenderTarget 
	 * @param AC 
	 * @param GTime 全局时间
	 */
	UFUNCTION(BlueprintCallable, Category = "RenderShaderLibaryPlugin", meta = (WorldContext = "WorldContextObject"))
	static void UseComputeShader(class UTextureRenderTarget2D* TextureRenderTarget, AActor* Actor, float GTime);

	
	/**
	 * @brief 贴图写入
	 * @param TextureToBeWrite 要被写入的贴图
	 */
	UFUNCTION(BlueprintCallable, Category = "RenderShaderLibaryPlugin", meta = (WorldContext = "WorldContextObject"))
	static void TextureWriting(UTexture2D* TextureToBeWrite);

	
	/**
	 * @brief 从路径中加载贴图并创建资源到指定目录
	 * @param Filename 本地路径
	 * @param PackageName UE4路径
	 */
	UFUNCTION(BlueprintCallable, Category = "RenderShaderLibaryPlugin", meta = (WorldContext = "WorldContextObject"))
	static void LoadTexture2DFormFile(const FString& Filename,const FString& PackageName);
};

UObject* CustomCreateBinary(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
							UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd,
							FFeedbackContext* Warn ,const FString& Filename);

bool ImportImage(const uint8* Buffer, uint32 Length,FFeedbackContext* Warn, FImportImage& OutImage);


#undef LOCTEXT_NAMESPACE
