// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/AuraUserWidget.h"
#include "AuraGameOverWidget.generated.h"

class UBorder;
class UButton;

/**
 *	角色死亡后的结束界面（纯 C++ 搭建）。
 *
 *	半透明血色黑幕 + 居中面板："GAME OVER" 标题淡入 + 面板轻微弹出，只提供"退出游戏"。
 *	输入拦截在 AAuraPlayerController 里做（死亡后不再响应移动与技能输入），
 *	这里只负责"点按钮退出"。
 */
UCLASS()
class AURA_API UAuraGameOverWidget : public UAuraUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	virtual void BuildWidgetTree() override;

	UFUNCTION() void HandleQuitClicked();

	UPROPERTY() TObjectPtr<UBorder> Panel;

	/** 已播放时间 */
	float ElapsedTime = 0.f;

	/** 整屏淡入时长 */
	static constexpr float FadeInDuration = 0.55f;

	/** 面板"弹出来"的时长 */
	static constexpr float PopDuration = 0.4f;
};
