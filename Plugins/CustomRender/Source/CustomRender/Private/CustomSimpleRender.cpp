// Fill out your copyright notice in the Description page of Project Settings.
#include "CustomSimpleRender.h"

// 定义结构体
IMPLEMENT_UNIFORM_BUFFER_STRUCT(FCustomUniformDataParameters, "CustomUniformDataParameters");
// 声明shader实现的地方,ShaderPathMap是我们设定的映射，在插件启动函数中设置的
IMPLEMENT_SHADER_TYPE(, FSimpleShaderVS, TEXT("/ShaderPathMap/Private/MyShader.usf"), TEXT("MainVS"), SF_Vertex);
IMPLEMENT_SHADER_TYPE(, FSimpleShaderPS, TEXT("/ShaderPathMap/Private/MyShader.usf"), TEXT("MainPS"), SF_Pixel);
IMPLEMENT_SHADER_TYPE(, FSimpleShaderCS, TEXT("/ShaderPathMap/Private/MyShader.usf"), TEXT("MainCS"), SF_Compute);

FSimpleShaderVS::FSimpleShaderVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
	: FGlobalShader(Initializer)
{
}

FSimpleShaderPS::FSimpleShaderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
	: FGlobalShader(Initializer)
{
	this->SimpleColorPara.Bind(Initializer.ParameterMap,TEXT("SimpleColor"));
	this->TexturePara.Bind(Initializer.ParameterMap,TEXT("MyTexture"));
	this->SampleStatePara.Bind(Initializer.ParameterMap,TEXT("MySamplerState"));
}

void FSimpleShaderPS::SetParameters(FRHICommandListImmediate& RHICmdList, FLinearColor Color,
									FTextureReferenceRHIRef Texture)
{
	SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), SimpleColorPara, Color);
	SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(),
						TexturePara,
						SampleStatePara,
						TStaticSamplerState<SF_Trilinear>::GetRHI(),
						Texture);

}

void FSimpleShaderPS::SetUniformData(FRHICommandListImmediate& RHICmdList, FCustomUniformData& Data)
{
	FCustomUniformDataParameters UniformData;
	UniformData.ColorIndex = Data.ColorIndex;
	UniformData.ColorOne = Data.ColorOne;
	UniformData.ColorTwo = Data.ColorTwo;
	SetUniformBufferParameterImmediate(RHICmdList, RHICmdList.GetBoundPixelShader(),
										GetUniformBufferParameter<FCustomUniformDataParameters>(), UniformData);
}

FSimpleShaderCS::FSimpleShaderCS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
	: FGlobalShader(Initializer)
{
	this->OutputSurfacePara.Bind(Initializer.ParameterMap,TEXT("OutputSurface"));
}

void FSimpleShaderCS::SetParameters(FRHICommandListImmediate& RHICmdList, FUnorderedAccessViewRHIRef& UAV)
{
	RHICmdList.SetUAVParameter(RHICmdList.GetBoundComputeShader(),OutputSurfacePara.GetUAVIndex(),UAV);
	//RHICmdList.TransitionResource(EResourceTransitionAccess::ERWBarrier,EResourceTransitionPipeline::EComputeToCompute,UAV);
	// 内部调用的是SetUAVParameterIfCS
	//OutputSurfacePara.SetTexture(RHICmdList,RHICmdList.GetBoundComputeShader(),nullptr,UAV);
}

void FSimpleShaderCS::UnsetParameters(FRHICommandListImmediate& RHICmdList,FUnorderedAccessViewRHIRef& UAV)
{
	RHICmdList.SetUAVParameter(RHICmdList.GetBoundComputeShader(),OutputSurfacePara.GetUAVIndex(),FUnorderedAccessViewRHIRef());
}
