// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AuraGameModeBase.generated.h"
class UCharacterClassInfo;
/**
 *
 */
UCLASS()
class AURA_API AAuraGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:

	/* 不同的角色需要的不同参数，挂载在这里，方便获取*/
	UPROPERTY(EditDefaultsOnly, Category = "Character|DataDriven")
	TObjectPtr<UCharacterClassInfo> CharacterClassInfo;
};
