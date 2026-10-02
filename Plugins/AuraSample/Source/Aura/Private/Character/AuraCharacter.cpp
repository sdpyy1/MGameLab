// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/AuraCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <AbilitySystem/AuraAbilitySystemComponent.h>
#include <AbilitySystem/AuraAttributeSet.h>

#include "GameFramework/PlayerState.h"
#include <Player/AuraPlayerController.h>
#include <UI/HUD/AuraHUD.h>
AAuraCharacter::AAuraCharacter()
{
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	/////////////////////////////  GAS /////////////////////////////
	// ASC/AttributeSet 已统一由基类 AAuraCharacterBase 创建在自己身上
}

void AAuraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);   // 基类已统一完成 GAS 初始化（属性 + 能力）

	// 主角特有：初始化 HUD 的 Overlay（血条 / 蓝条），ASC 与 AttributeSet 来自角色自己
	if (AAuraPlayerController* AuraPC = Cast<AAuraPlayerController>(GetController()))
	{
		if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(AuraPC->GetHUD()))
		{
			AuraHUD->InitOverlay(AuraPC, GetPlayerState(), AbilitySystemComponent, AttributeSet);
		}
	}
}

void AAuraCharacter::Die()
{
	Super::Die();   // 基类：掉武器、布娃娃、溶解、bDead = true

	/* 主角特有：弹出 GameOver 界面；输入拦截在 PlayerController 里（见 IsAvatarDead） */
	if (AAuraPlayerController* AuraPC = Cast<AAuraPlayerController>(GetController()))
	{
		if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(AuraPC->GetHUD()))
		{
			AuraHUD->ShowGameOver();
		}
	}
}