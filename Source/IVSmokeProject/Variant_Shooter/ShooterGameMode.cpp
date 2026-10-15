// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/IVShooterGameMode.h"
#include "IVShooterUI.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void AIVShooterGameMode::BeginPlay()
{
	Super::BeginPlay();

	// create the UI
	IVShooterUI = CreateWidget<UIVShooterUI>(UGameplayStatics::GetPlayerController(GetWorld(), 0), ShooterUIClass);
	IVShooterUI->AddToViewport(0);
}

void AIVShooterGameMode::IncrementTeamScore(uint8 TeamByte)
{
	// retrieve the team score if any
	int32 Score = 0;
	if (int32* FoundScore = TeamScores.Find(TeamByte))
	{
		Score = *FoundScore;
	}

	// increment the score for the given team
	++Score;
	TeamScores.Add(TeamByte, Score);

	// update the UI
	IVShooterUI->BP_UpdateScore(TeamByte, Score);
}
