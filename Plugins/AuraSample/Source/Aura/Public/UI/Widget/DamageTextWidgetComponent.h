// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "Engine/EngineTypes.h"
#include "DamageTextWidgetComponent.generated.h"

/**
 *	伤害飘字的宿主组件。
 *
 *	纯 C++：组件自己指定 UAuraDamageTextWidget 作为控件类，
 *	样式 / 动画全在那个控件里，不需要任何 WBP。
 */
UCLASS()
class AURA_API UDamageTextWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UDamageTextWidgetComponent();

	virtual void BeginPlay() override;

	/** 显示一次伤害数字；组件会在 LifeTime 之后自动销毁 */
	void SetDamageText(float Damage);

protected:
	/** 时间到，销毁自己 */
	void HandleLifeTimeElapsed();

	/** 自动销毁时间，略长于飘字动画（1.15s），保证动画播得完 */
	UPROPERTY(EditDefaultsOnly, Category = "DamageText")
	float LifeTime = 1.6f;

	FTimerHandle DestroyTimerHandle;
};
