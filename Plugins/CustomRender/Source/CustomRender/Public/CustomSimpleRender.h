// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "ShaderParameterUtils.h"
#include "CustomSimpleRender.generated.h"

BEGIN_UNIFORM_BUFFER_STRUCT(FCustomUniformDataParameters,)
	SHADER_PARAMETER(FVector4, ColorOne)
	SHADER_PARAMETER(FVector4, ColorTwo)
	SHADER_PARAMETER(uint32, ColorIndex)
END_UNIFORM_BUFFER_STRUCT()

USTRUCT(BlueprintType)
struct FCustomUniformData
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = ShaderData)
	FLinearColor ColorOne;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = ShaderData)
	FLinearColor ColorTwo;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = ShaderData)
	int32 ColorIndex;
};

class CUSTOMRENDER_API FSimpleShaderVS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSimpleShaderVS, Global)
public:
	FSimpleShaderVS()
	{
	}

	FSimpleShaderVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer);

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters,
											FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
};

class FSimpleShaderPS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSimpleShaderPS, Global)
public:
	FSimpleShaderPS()
	{
	}

	FSimpleShaderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer);


	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters,
											FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}

	// ??????shader??????
	void SetParameters(FRHICommandListImmediate& RHICmdList, FLinearColor Color, FRHITexture* Texture);
	void SetUniformData(FRHICommandListImmediate& RHICmdList, FCustomUniformData& Data);
private:
	LAYOUT_FIELD(FShaderParameter, SimpleColorPara);
	LAYOUT_FIELD(FShaderResourceParameter, TexturePara);
	LAYOUT_FIELD(FShaderResourceParameter, SampleStatePara);
};

class FSimpleShaderCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSimpleShaderCS, Global)
public:
	FSimpleShaderCS()
	{
	}

	FSimpleShaderCS(const ShaderMetaType::CompiledShaderInitializerType& Initializer);

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters,
											FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}

	void SetParameters(FRHICommandListImmediate& RHICmdList, FUnorderedAccessViewRHIRef& UAV, float Time);
	void UnsetParameters(FRHICommandListImmediate& RHICmdList, FUnorderedAccessViewRHIRef& UAV);

private:
	// raw shader parameter
	LAYOUT_FIELD(FRWShaderParameter, OutputSurfacePara);
	LAYOUT_FIELD(FShaderParameter, TimePara);
};

struct FCustomVertex
{
	FVector4 Pos; // ??????
	FVector2D UV;
};

class FCustomVertexBuffer : public FVertexBuffer
{
public:
	virtual void InitRHI() override
	{
		// ????????????????????????????????????????????????

		// ??????????????????
		TResourceArray<FCustomVertex, VERTEXBUFFER_ALIGNMENT> VertexBuffer;
		VertexBuffer.SetNumUninitialized(4);
		VertexBuffer[0].Pos = FVector4(-1, 1, 0, 1);
		VertexBuffer[1].Pos = FVector4(1, 1, 0, 1);
		VertexBuffer[2].Pos = FVector4(-1, -1, 0, 1);
		VertexBuffer[3].Pos = FVector4(1, -1, 0, 1);
		VertexBuffer[0].UV = FVector2D(0.0f, 1.0f);
		VertexBuffer[1].UV = FVector2D(1.0f, 1.0f);
		VertexBuffer[2].UV = FVector2D(0.0f, 0.0f);
		VertexBuffer[3].UV = FVector2D(1.0f, 0.0f);

		// ??????????????????
		FRHIResourceCreateInfo CreateInfo(&VertexBuffer);
		VertexBufferRHI = RHICreateVertexBuffer(VertexBuffer.GetResourceDataSize(), BUF_Static, CreateInfo);
	}
};

class FCustomIndexBuffer : public FIndexBuffer
{
public:
	virtual void InitRHI() override
	{
		// ?????????????????????????????????????????????

		const uint16 Indices[] = {0, 1, 2, 2, 1, 3};
		const uint16 NumbIndices = UE_ARRAY_COUNT(Indices); // ????????????

		// ????????????buffer
		TResourceArray<uint16, INDEXBUFFER_ALIGNMENT> IndexBuffer;
		IndexBuffer.AddUninitialized(NumbIndices);
		FMemory::Memcpy(IndexBuffer.GetData(), Indices, NumbIndices * sizeof(uint16)); // ????????????????????????buffer???

		// ??????????????????
		FRHIResourceCreateInfo CreateInfo(&IndexBuffer);
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16),
											IndexBuffer.GetResourceDataSize(),
											BUF_Static, CreateInfo);
	}
};

// ??????????????????
class FCustomVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;
public:
	virtual void InitRHI() override
	{
		// ?????????
		FVertexDeclarationElementList ElementList;
		const uint16 Stride = sizeof(FCustomVertex);
		ElementList.Add(FVertexElement(0, STRUCT_OFFSET(FCustomVertex, Pos), VET_Float4, 0, Stride)); // ????????????????????????
		ElementList.Add(FVertexElement(0, STRUCT_OFFSET(FCustomVertex, UV), VET_Float2, 1, Stride)); // ????????????????????????
		VertexDeclarationRHI = RHICreateVertexDeclaration(ElementList);
	}
};
