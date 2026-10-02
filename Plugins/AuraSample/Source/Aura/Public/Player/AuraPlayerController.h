// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "AuraPlayerController.generated.h"

class UNiagaraSystem;
class UInputMappingContext;
class UInputAction;
class IEnemyInterface;
class UAuraInputConfig;
class UAuraAbilitySystemComponent;
class USplineComponent;
class ACharacter;
/**
 *
 */
UCLASS()
class AURA_API AAuraPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AAuraPlayerController();
	void ShowDamageNumber(float Damage, ACharacter* Target);

	/** 属性菜单开关时切换输入模式：菜单打开时输入只给 UI，避免点菜单顺手放出技能 */
	void SetAttributeMenuInputMode(bool bMenuOpen);

	/** 角色死亡后进入结束状态：停掉移动/技能输入，把输入交给 GameOver 界面 */
	void EnterGameOverState();

	/** 角色是否已经死亡（死亡后不再响应移动与技能输入） */
	bool IsAvatarDead() const;
protected:
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;
	void AutoRun();
	virtual void SetupInputComponent() override;

private:
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> AuraContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	void Move(const struct FInputActionValue& InputActionValue);
	void CursorTrace();

	IEnemyInterface* LastActor;
	IEnemyInterface* ThisActor;

	/*
		Connect InputAction To GameplayAbility
	*/
	void AbilityInputTagPress(FGameplayTag GameplayTag);
	void AbilityInputTagRelease(FGameplayTag GameplayTag);
	void AbilityInputTagHeld(FGameplayTag GameplayTag);

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UAuraInputConfig> InputConfig;
	
	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;

	UAuraAbilitySystemComponent* GetAuraASC();
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> ClickMoveNiagara;

	FVector CachedDestination = FVector::ZeroVector;
	float FollowTime = 0.f;
	float ShortPressThreshold = 0.5f;
	bool bAutoRunning = false;
	bool bTargeting = false;
	UPROPERTY(EditDefaultsOnly)
	float AutoRunAcceptanceRadius = 50.f;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> Spline;
};
