// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"

#include "AuraPlayerState.generated.h"

/**
 * 玩家状态：目前没有额外数据。
 * GAS（ASC/AttributeSet）与等级 Level 都已统一收敛到 AAuraCharacterBase，
 * 这里保留一个空的 PlayerState 子类，供 GameMode 的 PlayerStateClass 使用；
 * 若确认不再需要，可连同 BP_AuraGameMode 里的 PlayerStateClass 一起删除。
 */
UCLASS()
class AURA_API AAuraPlayerState : public APlayerState
{
	GENERATED_BODY()
};
