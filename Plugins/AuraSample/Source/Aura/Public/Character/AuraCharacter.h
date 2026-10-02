// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "AuraCharacter.generated.h"

/**
 *
 */
UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase
{
	GENERATED_BODY()
public:
	AAuraCharacter();

	/*
		Spawn Character
		↓
		Spawn PlayerState
		↓
		Controller Possess Character
		↓
		基类 PossessedBy() 统一初始化GAS，主角额外初始化HUD
	*/
	virtual void PossessedBy(AController* NewController) override;

	/*
		Combat Interface
		主角特有：死亡时弹出 GameOver 界面（基类负责掉武器、布娃娃、溶解）
	*/
	virtual void Die() override;
};
