// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FCustomRenderModule : public IModuleInterface
{
public:
	
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static inline FCustomRenderModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FCustomRenderModule>("CustomRender");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("CustomRender");
	}
};
