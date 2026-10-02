// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/AuraUserWidget.h"
#include "AuraDamageTextWidget.generated.h"

class UTextBlock;

/**
 *	飘字（伤害数字）：纯 C++ 动画。
 *
 *	播放过程：先"弹一下"放大回落 → 向上飘 → 后半段淡出。
 *	组件（UDamageTextWidgetComponent）负责到点销毁。
 */
UCLASS()
class AURA_API UAuraDamageTextWidget : public UAuraUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** 设置要显示的伤害值并重新播放动画 */
	void SetDamageText(float Damage);

protected:
	virtual void BuildWidgetTree() override;
	virtual void NativeConstruct() override;

	/** 把 DamageValue 写进文本（控件树不一定已经就绪，所以单独抽出来） */
	void ApplyDamageText();

	UPROPERTY() TObjectPtr<UTextBlock> DamageText;

	/** 要显示的伤害值 */
	float DamageValue = 0.f;

	/** 已播放时间 */
	float ElapsedTime = 0.f;

	/** 每次生成时给一个随机的横向偏移 / 角度，连着打多个数字才不会叠在一起 */
	FVector2D RandomOffset = FVector2D::ZeroVector;
	float RandomAngle = 0.f;

	/** 动画总时长 */
	static constexpr float LifeTime = 1.15f;

	/** 向上飘的距离（像素） */
	static constexpr float RiseDistance = 78.f;
};
