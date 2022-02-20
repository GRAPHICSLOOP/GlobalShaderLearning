// Fill out your copyright notice in the Description page of Project Settings.


#include "RenderFunctionLibrary.h"

#include "UnrealAudioDeviceModule.h"


DEFINE_LOG_CATEGORY_STATIC(CustomLog, Log, All);

TGlobalResource<FCustomVertexBuffer> G_CustomVertexBuffer;
TGlobalResource<FCustomIndexBuffer> G_CustomIndexBuffer;

void FImportImage::Init2DWithParams(int32 InSizeX, int32 InSizeY, ETextureSourceFormat InFormat, bool InSRGB)
{
	SizeX = InSizeX;
	SizeY = InSizeY;
	NumMips = 1;
	Format = InFormat;
	SRGB = InSRGB;
}

void DrawRenderTarget_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	FTextureRenderTargetResource* RenderTargetResource,
	ERHIFeatureLevel::Type FeatureLevel,
	FName TextureRenderTargetName,
	FLinearColor Color,
	FRHITexture* Texuture,
	FCustomUniformData Data)
{
	check(IsInRenderingThread());

	//#if WANTS_DRAW_MESH_EVENTS  
	//	FString EventName;
	//	TextureRenderTargetName.ToString(EventName);
	//	SCOPED_DRAW_EVENTF(RHICmdList, SceneCapture, TEXT("ShaderTest %s"), *EventName);
	//#else  
	//	SCOPED_DRAW_EVENT(RHICmdList, DrawRenderTarget_RenderThread);
	//#endif  


	// 创建passinfo
	FRHIRenderPassInfo PassInfo(RenderTargetResource->GetRenderTargetTexture(),
								ERenderTargetActions::DontLoad_DontStore); // 两个参数正常运行 官方给出的也是两个参数
	RHICmdList.BeginRenderPass(PassInfo, TEXT("CustomPass"));
	{
		// 1.获取shader
		FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);
		TShaderMapRef<FSimpleShaderVS> VertexShader(GlobalShaderMap);
		TShaderMapRef<FSimpleShaderPS> PixelShader(GlobalShaderMap);

		// 2.创建顶点描述
		FCustomVertexDeclaration CustomVertexDeclaration;
		CustomVertexDeclaration.InitRHI();

		// 3.设置PSO
		FGraphicsPipelineStateInitializer PSOIniter;
		RHICmdList.ApplyCachedRenderTargets(PSOIniter);
		PSOIniter.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
		PSOIniter.BlendState = TStaticBlendState<>::GetRHI();
		PSOIniter.RasterizerState = TStaticRasterizerState<>::GetRHI();
		PSOIniter.PrimitiveType = PT_TriangleList;
		PSOIniter.BoundShaderState.VertexDeclarationRHI = CustomVertexDeclaration.VertexDeclarationRHI;
		PSOIniter.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
		PSOIniter.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
		SetGraphicsPipelineState(RHICmdList, PSOIniter);

		// 4.设置viewport
		RHICmdList.SetViewport(0, 0, 0, RenderTargetResource->GetSizeX(), RenderTargetResource->GetSizeY(), 1.f);

		// 设置shader参数
		PixelShader->SetParameters(RHICmdList, Color, Texuture);
		PixelShader->SetUniformData(RHICmdList, Data);

		// 5.准备渲染
		RHICmdList.SetStreamSource(0, G_CustomVertexBuffer.VertexBufferRHI, 0);
		RHICmdList.DrawIndexedPrimitive(G_CustomIndexBuffer.IndexBufferRHI, 0, 0, 4, 0, 2, 1);
	}
	RHICmdList.EndRenderPass();
}

void DrawComputerShader_RenderThread(FRHICommandListImmediate& RHICmdList,
									FTextureRenderTargetResource* RenderTargetResource,
									ERHIFeatureLevel::Type FeatureLevel,
									EPixelFormat PixelFormat,
									float GTime)
{
	
	check(IsInRenderingThread());

	FTexture2DRHIRef Texture2DRHIRef = RenderTargetResource->GetRenderTargetTexture();
	uint32 GroupNumb = 32;
	uint32 GroupSizeX = FMath::DivideAndRoundUp(RenderTargetResource->GetSizeX(), GroupNumb);
	uint32 GroupSizeY = FMath::DivideAndRoundUp(RenderTargetResource->GetSizeY(), GroupNumb);

	// 设置computershader
	FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);
	TShaderMapRef<FSimpleShaderCS> ComputerShader(GlobalShaderMap);
	RHICmdList.SetComputeShader(ComputerShader.GetComputeShader());

	// 创建UAV
	FRHIResourceCreateInfo CreateInfo;
	FTexture2DRHIRef UAVRHIRef = RHICreateTexture2D(RenderTargetResource->GetSizeX(), RenderTargetResource->GetSizeY(),
													PixelFormat, 1, 1, TexCreate_ShaderResource | TexCreate_UAV,
													CreateInfo);
	FUnorderedAccessViewRHIRef UAV = RHICreateUnorderedAccessView(UAVRHIRef);

	// 开始渲染
	ComputerShader->SetParameters(RHICmdList,UAV,GTime);
	DispatchComputeShader(RHICmdList,ComputerShader,GroupSizeX,GroupSizeY,1);
	ComputerShader->UnsetParameters(RHICmdList,UAV);

	// 复制结果
	FRHICopyTextureInfo CopyInfo;
	RHICmdList.CopyTexture(UAVRHIRef,Texture2DRHIRef,CopyInfo);

	// 调用GlobalShader
	// FLinearColor Color = FLinearColor(FVector4(1.f,1.f,1.f,1.f));
	// FCustomUniformData Data;
	// Data.ColorIndex = 0;
	// Data.ColorOne = FLinearColor(FVector4(1.f,1.f,1.f,1.f));
	// Data.ColorTwo = FLinearColor(FVector4(1.f,1.f,1.f,1.f));
	// DrawRenderTarget_RenderThread(RHICmdList,RenderTargetResource,FeatureLevel,FName("NULL"),Color,tempRHIRef.GetReference(),Data);
}


URenderFunctionLibrary::URenderFunctionLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void URenderFunctionLibrary::UseGlobalShader(UTextureRenderTarget2D* TextureRenderTarget, AActor* Actor,
														FLinearColor Color, UTexture2D* Texture,
														FCustomUniformData Data)
{
	check(IsInGameThread());

	if (!TextureRenderTarget)
	{
		return;
	}
	
	FRHITexture* RHITexture = Texture->TextureReference.TextureReferenceRHI.GetReference();
	FTextureRenderTargetResource* TextureRenderTargetResource = TextureRenderTarget->GameThread_GetRenderTargetResource();

	// 主要就是获取 FeatureLevel
	const UWorld* World = Actor->GetWorld();
	ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();
	
	FName TextureRenderTargetName = TextureRenderTarget->GetFName();
	
	ENQUEUE_RENDER_COMMAND(CaptureCommand)(
		[TextureRenderTargetResource, FeatureLevel, Color, TextureRenderTargetName,RHITexture,Data](
		FRHICommandListImmediate& RHICmdList)
		{
			DrawRenderTarget_RenderThread(RHICmdList, TextureRenderTargetResource, FeatureLevel,
										TextureRenderTargetName, Color, RHITexture, Data);
		}
	);
}

void URenderFunctionLibrary::UseComputeShader(UTextureRenderTarget2D* TextureRenderTarget, AActor* Actor,float GTime)
{
	check(IsInGameThread());
	
	if (TextureRenderTarget == nullptr || Actor == nullptr)
		return;

	// 主要就是获取 FeatureLevel
	const UWorld* World = Actor->GetWorld();
	ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();

	// 获取对应的格式
	EPixelFormat PixelFormat = GetPixelFormatFromRenderTargetFormat(TextureRenderTarget->RenderTargetFormat);
	FTextureRenderTargetResource* TextureRenderTargetResource = TextureRenderTarget->GameThread_GetRenderTargetResource();

	ENQUEUE_RENDER_COMMAND(CaptureCommand)(
		[TextureRenderTargetResource, FeatureLevel,PixelFormat,GTime](
		FRHICommandListImmediate& RHICmdList)
		{
			DrawComputerShader_RenderThread(RHICmdList, TextureRenderTargetResource, FeatureLevel,PixelFormat,GTime);
		}
	);
}

void URenderFunctionLibrary::TextureWriting(UTexture2D* TextureToBeWrite)
{
	check(IsInGameThread());

	if (TextureToBeWrite == nullptr)
		return;

	TextureToBeWrite->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	TextureToBeWrite->SRGB = 0;
#if WITH_EDITORONLY_DATA
	TextureToBeWrite->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	// 需要注意的是，MipGenSetting是编辑器数据，该变量是在WITH_EDITORONLY_DATA里被定义的，因此是无法被打包出去的
#endif
	TextureToBeWrite->UpdateResource();
	// 锁定-开始准备写入数据
	void* Data = TextureToBeWrite->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	if (Data == nullptr)
	{
		TextureToBeWrite->PlatformData->Mips[0].BulkData.Unlock();
		return;
	}
	const int32 TextureSize = TextureToBeWrite->GetSizeX() * TextureToBeWrite->GetSizeY();
	TArray<FColor> Colors;
	for (int32 i = 0; i < TextureSize; i++)
	{
		Colors.Add(FColor::Blue);
	}


	int32 Stride = sizeof(FColor);
	FMemory::Memcpy(Data, Colors.GetData(), TextureSize * Stride);
	TextureToBeWrite->PlatformData->Mips[0].BulkData.Unlock();
	TextureToBeWrite->UpdateResource();
}

void URenderFunctionLibrary::LoadTexture2DFormFile(const FString& Filename, const FString& PackageName)
{
	// 文件相关
	const FString Name = FPaths::GetBaseFilename(Filename);
	const FString FileExtension = FPaths::GetExtension(Filename);

	// PackageName是游戏content的文件路径，注意它是没有后缀名的
	UPackage* Pkg = CreatePackage(*PackageName);
	// 确保pkg已经加载
	Pkg->FullyLoad();

	// load as binary
	TArray<uint8> Data;
	if (!FFileHelper::LoadFileToArray(Data, *Filename))
	{
		UE_LOG(CustomLog, Error, TEXT("Failed to load file '%s' to array"), *Filename);
		return;
	}

	Data.Add(0); // 
	const uint8* Ptr = &Data[0];
	UObject* result = CustomCreateBinary(UTexture2D::StaticClass(), Pkg, FName(*Name),
										RF_Public | RF_Standalone | RF_Transactional, nullptr, *FileExtension, Ptr,
										Ptr + Data.Num() - 1, nullptr, Filename);

	if (result != nullptr)
	{
		result->MarkPackageDirty();
		result->PostEditChange();

		FAssetRegistryModule::AssetCreated(result);
	}
	else
	{
		UE_LOG(CustomLog, Error, TEXT("Failed to load file"));
	}
}

UObject* CustomCreateBinary(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
							UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd,
							FFeedbackContext* Warn, const FString& Filename)
{
	UTexture* Texture = nullptr;
	bool bAllowNonPowerOfTwo = false;

	// 是否有效
	const int32 Length = BufferEnd - Buffer;
	if (Length < 0)
		return nullptr;

	//
	// 生成2D Image
	//
	FImportImage Image;
	if (ImportImage(Buffer, Length, Warn, Image))
	{
		Texture = NewObject<UTexture2D>(InParent, Name, Flags);
		if (Texture)
			Texture->Source.Init(Image.SizeX, Image.SizeY, 1, Image.NumMips, Image.Format, Image.RawData.GetData());
	}

#if WITH_EDITORONLY_DATA
	if (Texture)
	{
		// 更新文件导入信息
		Texture->AssetImportData->Update(Filename, nullptr);
	}
#endif

	return Texture;
}

bool ImportImage(const uint8* Buffer, uint32 Length, FFeedbackContext* Warn, FImportImage& OutImage)
{
	// 加载imageWrap模块
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(
		FName("ImageWrapper"));

	//
	// PNG
	//
	TSharedPtr<IImageWrapper> PngImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
	if (PngImageWrapper.IsValid() && PngImageWrapper->SetCompressed(Buffer, Length))
	{
		// 默认先设置为无效
		ETextureSourceFormat TextureFormat = TSF_Invalid;
		int32 BitDepth = PngImageWrapper->GetBitDepth(); // 获取像素以多少位存储的
		ERGBFormat Format = PngImageWrapper->GetFormat();

		if (Format == ERGBFormat::Gray) // 只有R通道
		{
			if (BitDepth <= 8)
			{
				TextureFormat = TSF_G8;
				BitDepth = 8;
				Format = ERGBFormat::Gray;
			}
			else if (BitDepth == 16)
			{
				// 暂不处理，因为大部分都是8bit的
				return false; // UTextureFactory::ImportImage 有实现
			}
		}
		else if (Format == ERGBFormat::RGBA || Format == ERGBFormat::BGRA)
		{
			if (BitDepth <= 8)
			{
				TextureFormat = TSF_BGRA8;
				Format = ERGBFormat::BGRA;
				BitDepth = 8;
			}
			else
			{
				// 16bit时才是rgb
				TextureFormat = TSF_RGBA16;
				Format = ERGBFormat::RGBA;
				BitDepth = 16;
			}
		}

		if (TextureFormat == TSF_Invalid)
		{
			// 查找不到任何格式
			if (Warn != nullptr)
				Warn->Logf(ELogVerbosity::Error, TEXT("PNG file contains data in an unsupported format."));
			return false;
		}

		OutImage.Init2DWithParams(PngImageWrapper->GetWidth(), PngImageWrapper->GetHeight(), TextureFormat,
								BitDepth > 8);

		return PngImageWrapper->GetRaw(Format, BitDepth, OutImage.RawData);
	}

	//
	// JPEG
	//
	TSharedPtr<IImageWrapper> JpegImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
	if (JpegImageWrapper.IsValid() && JpegImageWrapper->SetCompressed(Buffer, Length))
	{
		// 默认先设置为无效
		ETextureSourceFormat TextureFormat = TSF_Invalid;
		int32 BitDepth = JpegImageWrapper->GetBitDepth();
		ERGBFormat Format = JpegImageWrapper->GetFormat();

		if (Format == ERGBFormat::Gray)
		{
			if (BitDepth <= 8)
			{
				TextureFormat = TSF_G8;
				Format = ERGBFormat::Gray;
				BitDepth = 8;
			}
		}
		else if (Format == ERGBFormat::RGBA)
		{
			if (BitDepth <= 8)
			{
				TextureFormat = TSF_BGRA8;
				Format = ERGBFormat::BGRA;
				BitDepth = 8;
			}
		}

		if (TextureFormat == TSF_Invalid)
		{
			Warn->Logf(ELogVerbosity::Error, TEXT("JPEG file contains data in an unsupported format."));
			return false;
		}

		OutImage.Init2DWithParams(
			JpegImageWrapper->GetWidth(),
			JpegImageWrapper->GetHeight(),
			TextureFormat,
			BitDepth < 16
		);

		if (!JpegImageWrapper->GetRaw(Format, BitDepth, OutImage.RawData))
		{
			Warn->Logf(ELogVerbosity::Error, TEXT("Failed to decode JPEG."));
			return false;
		}

		return true;
	}

	return false;
}
