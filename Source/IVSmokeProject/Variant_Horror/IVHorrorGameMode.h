// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "IVHorrorGameMode.generated.h"

/**
 *  Simple GameMode for a first person horror game
 */
UCLASS(abstract)
class IVSMOKEPROJECT_API AIVHorrorGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	/** Constructor */
	AIVHorrorGameMode();
};
