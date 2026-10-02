// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/AuraUserWidget.h"
#include "AuraMessageWidget.generated.h"

class UTextBlock;
class UImage;
class USizeBox;
class UBorder;

/**
 *	一条消息通知（拾取药水之类的提示）。
 *
 *	由 UAuraOverlayWidget 加到"屏幕右侧中间"的通知列表里，多条会堆叠显示。
 *	动画：从右边蹦进来（带过冲）→ 停留 → 一边变全透明一边往右飘走。
 *	播完后由 UAuraOverlayWidget 把它从列表里移除（自己不做销毁，避免在 Tick 里改控件树）。
 */
UCLASS()
class AURA_API UAuraMessageWidget : public UAuraUserWidget
{
	GENERATED_BODY()

public:
	/** 播放一条消息（控件树还没搭好时会先缓存，等构造完再写进去） */
	void SetMessage(const FText& InMessage, UTexture2D* InIcon);

	/** 动画是否已经播完 */
	bool IsFinished() const { return ElapsedTime >= LifeTime; }

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	virtual void BuildWidgetTree() override;
	virtual void NativeConstruct() override;

	/** 把缓存的消息写进控件 */
	void ApplyMessage();

	UPROPERTY() TObjectPtr<UBorder> MessagePanel;
	UPROPERTY() TObjectPtr<UTextBlock> MessageText;
	UPROPERTY() TObjectPtr<USizeBox> IconBox;
	UPROPERTY() TObjectPtr<UImage> Icon;

	FText CachedMessage;
	TObjectPtr<UTexture2D> CachedIcon = nullptr;
	bool bHasMessage = false;

	/** 已播放时间 */
	float ElapsedTime = 0.f;

	/* ---------- 动画参数 ---------- */
	/** 单条消息存活时间（到这里 IsFinished 为真，随后被移除） */
	static constexpr float LifeTime = 2.7f;

	/** "蹦进来"阶段时长 */
	static constexpr float PopDuration = 0.28f;

	/** 从第几秒开始淡出 */
	static constexpr float FadeStart = 1.9f;

	/** 出场时从右边多远滑进来 */
	static constexpr float SlideInDistance = 160.f;

	/** 淡出时往右飘走的距离 */
	static constexpr float SlideOutDistance = 70.f;

	/** 出现时的初始缩放（略小，配合 BackOut 形成"弹"的感觉） */
	static constexpr float PopStartScale = 0.85f;
};
