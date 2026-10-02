// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"

class UNiagaraSystem;

USTRUCT(Blueprintable)
struct FTaggedMontage
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	UAnimMontage* Montage = nullptr;

	/* Montage 里的 Notify 携带的 Tag：能力用它来接收 Notify 事件（WaitGameplayEvent），默认统一为 Montage.Notify.Attack */
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FGameplayTag NotifyTag;

	/* 这条蒙太奇的攻击方式（武器 / 左手 / 右手…），决定从哪个 Socket 取命中位置 */
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	FGameplayTag AttackMethodTag;

	/* 命中音效 */
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	USoundBase * HitSound = nullptr;
};




// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};
class UAnimMontage;
/**
 *	一些战斗相关的接口定义，会被所有AuraCharacter继承
 */
class AURA_API ICombatInterface
{
	GENERATED_BODY()

public:
	virtual int32 GetPlayerLevel();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FVector GetCombatSocketLocation(const FGameplayTag& AttackMethodTag);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateFacingVector(const FVector& Target);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UAnimMontage* GetHitReactMontage();
		
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	TArray<FTaggedMontage> GetAttackMontages();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UNiagaraSystem * GetBloodEffect();


	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool isDead() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	AActor * GetAvatar();
	virtual void Die() = 0;

};
