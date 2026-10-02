// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/AuraUserWidget.h"
#include "UI/WidgetController/AuraOverlayWidgetController.h"
#include "AuraOverlayWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UBorder;
class USizeBox;
class UVerticalBox;
class UAuraMessageWidget;

/**
 *	屏幕常驻 HUD（纯 C++ 搭建）：
 *	  · 左下角：等级徽章 + 生命 / 魔力条（掉血有延迟残影，加血有平滑过渡）
 *	  · 右下角：打开属性菜单的按钮
 *	  · 收到 GameplayEffect 消息时，在角色头顶附近弹提示（见 HandleMessageRow）
 */
UCLASS()
class AURA_API UAuraOverlayWidget : public UAuraUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	virtual void BuildWidgetTree() override;
	virtual bool BindWidgetDelegates() override;

	/* ---------- WidgetController 委托 ---------- */
	UFUNCTION() void HandleHealthChanged(float NewValue);
	UFUNCTION() void HandleMaxHealthChanged(float NewValue);
	UFUNCTION() void HandleManaChanged(float NewValue);
	UFUNCTION() void HandleMaxManaChanged(float NewValue);
	UFUNCTION() void HandleMessageRow(FUIWidgetRow Row);
	UFUNCTION() void HandleMenuButtonClicked();

	/**
	 *	造一条"残影 + 主体 + 数值 + 描边"的进度条。
	 *	@param NamePrefix	控件名前缀，方便在 Slate 调试器里认人
	 *	@param FillColor	主体颜色
	 *	@param GhostColor	掉血残影颜色
	 *	@param OutGhostBar / OutMainBar / OutValueText	零件引用，用来在 Tick 里刷新
	 */
	UWidget* MakeBar(const FString& NamePrefix, const FLinearColor& FillColor, const FLinearColor& GhostColor,
		float Width, float Height, float CornerRadius,
		TObjectPtr<UProgressBar>& OutGhostBar, TObjectPtr<UProgressBar>& OutMainBar, TObjectPtr<UTextBlock>& OutValueText);

	/** 刷新一条进度条（主体快速插值、残影慢速跟随、数值文本同步） */
	static void RefreshBar(UProgressBar* MainBar, UProgressBar* GhostBar, UTextBlock* ValueText,
		float DisplayValue, float GhostValue, float MaxValue);

protected:
	/* ---------- 零件 ---------- */
	UPROPERTY() TObjectPtr<UProgressBar> HealthBar;
	UPROPERTY() TObjectPtr<UProgressBar> HealthGhostBar;
	UPROPERTY() TObjectPtr<UTextBlock> HealthValueText;

	UPROPERTY() TObjectPtr<UProgressBar> ManaBar;
	UPROPERTY() TObjectPtr<UProgressBar> ManaGhostBar;
	UPROPERTY() TObjectPtr<UTextBlock> ManaValueText;

	UPROPERTY() TObjectPtr<UTextBlock> LevelText;

	/** 屏幕右侧中间的消息通知列表（多条消息会依次往下堆叠） */
	UPROPERTY() TObjectPtr<UVerticalBox> MessageList;

	/** 正在播放的消息通知；播完之后由本控件负责移除 */
	UPROPERTY() TArray<TObjectPtr<UAuraMessageWidget>> ActiveMessages;

	/* ---------- 动画状态 ---------- */
	/* 目标值来自委托；Display 是条上真正的值；Ghost 是掉血残影 */
	float TargetHealth = 0.f;
	float MaxHealth = 1.f;
	float DisplayHealth = 0.f;
	float GhostHealth = 0.f;

	float TargetMana = 0.f;
	float MaxMana = 1.f;
	float DisplayMana = 0.f;
	float GhostMana = 0.f;
};
