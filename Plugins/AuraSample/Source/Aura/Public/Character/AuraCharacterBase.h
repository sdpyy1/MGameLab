// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Character/MGCharacterBase.h"
#include "Interaction/CombatInterface.h"
/* 协变返回 UAuraAbilitySystemComponent* 需要其完整定义 */
#include "AbilitySystem/AuraAbilitySystemComponent.h"
/* 角色档案枚举 ECharacterClass */
#include "AbilitySystem/Data/CharacterClassInfo.h"

#include "AuraCharacterBase.generated.h"

class AController;
class UNiagaraSystem;
class UAuraAbilitySystemComponent;
class UAuraAttributeSet;
class UAnimMontage;
class UMaterialInstance;
class UMaterialInstanceDynamic;
/*
	所有角色的基类（主角和敌人）
*/
UCLASS(Abstract)  // Abstract来控制这个类不能被实例化
class AURA_API AAuraCharacterBase : public AMGCharacterBase, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	AAuraCharacterBase();

	/* 角色被 Controller 占有：初始化 GAS 并赋予默认能力（主角和敌人共用同一套逻辑） */
	virtual void PossessedBy(AController* NewController) override;

/*ICombatInterface*/
public:
	virtual int32 GetPlayerLevel() override;
	virtual UAnimMontage* GetHitReactMontage_Implementation() override;
	virtual FVector GetCombatSocketLocation_Implementation(const FGameplayTag& AttackMethodTag) override;

	virtual void Die() override;
	virtual bool isDead_Implementation() const override;
	virtual AActor * GetAvatar_Implementation() override;
	virtual TArray<FTaggedMontage> GetAttackMontages_Implementation() override;
	virtual UNiagaraSystem * GetBloodEffect_Implementation() override;
/*ICombatInterface End*/

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 Level = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	ECharacterClass CharacterClass = ECharacterClass::Warrior;
	
	
/* GAS */
public:
	virtual UAuraAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAuraAttributeSet* GetAttributeSet() const;
protected:
	UPROPERTY(BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAuraAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY(BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAuraAttributeSet> AttributeSet;
	virtual void InitGas();
/* GAS End*/


protected:
	bool bDead = false;
	bool bGasInitialized = false;
	
	/*武器相关*/
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName WeaponTipSocketName;
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName LeftHandSocketName = "LeftHandSocket";
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName RightHandSocketName = "RightHandSocket";
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName TailSocketName = "TailSocket";
	
	
	/* 角色死亡时替换为消融材质 */
	void Dissolve();
	UFUNCTION(BlueprintImplementableEvent)
	void StartDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);
	UFUNCTION(BlueprintImplementableEvent)
	void StartWeaponDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Material")
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Material")
	TObjectPtr<UMaterialInstance> WeaponDissolveMaterialInstance;

	/* 受击时播放的蒙太奇 */
	UPROPERTY(EditAnywhere, Category = "Combat|Montage")
	TObjectPtr<UAnimMontage> HitReactMontage;
	/* 攻击时播放的蒙太奇(随机挑选一个) */
	UPROPERTY(EditAnywhere,Category = "Combat")
	TArray<FTaggedMontage> AttackMontages;

	/* 被近战攻击命中后的特效 */
	UPROPERTY(EditAnywhere,blueprintReadOnly, Category = "Combat")
	UNiagaraSystem * BloodEffect;

private:
	/* 角色死亡处理（单机直接调用，不再走RPC） */
	virtual void HandleDeath();
};
