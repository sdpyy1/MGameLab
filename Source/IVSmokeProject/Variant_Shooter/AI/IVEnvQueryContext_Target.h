// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "IVEnvQueryContext_Target.generated.h"

/**
 *  Custom EnvQuery Context that returns the actor currently targeted by an NPC
 */
UCLASS()
class IVSMOKEPROJECT_API UIVEnvQueryContext_Target : public UEnvQueryContext
{
	GENERATED_BODY()
	
public:

	/** Provides the context locations or actors for this EnvQuery */
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;

};
