// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "IVShooterGameMode.generated.h"

class UIVShooterUI;

/**
 *  Simple GameMode for a first person shooter game
 *  Manages game UI
 *  Keeps track of team scores
 */
UCLASS(abstract)
class IVSMOKEPROJECT_API AIVShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:

	/** Type of UI widget to spawn */
	UPROPERTY(EditAnywhere, Category="Shooter")
	TSubclassOf<UIVShooterUI> ShooterUIClass;

	/** Pointer to the UI widget */
	TObjectPtr<UIVShooterUI> IVShooterUI;

	/** Map of scores by team ID */
	TMap<uint8, int32> TeamScores;

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

public:

	/** Increases the score for the given team */
	void IncrementTeamScore(uint8 TeamByte);
};
