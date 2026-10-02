// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/AuraMessageWidget.h"

#include "UI/AuraUIStyle.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"

void UAuraMessageWidget::BuildWidgetTree()
{
	/* 根节点自适应内容大小，动画靠 RenderScale / RenderTranslation */
	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("MessageRoot"));
	WidgetTree->RootWidget = Root;

	/*
		透明底：不填充颜色，只留一圈淡淡的金色描边。
		文字本身带阴影，所以背景再亮也读得清。
	*/
	MessagePanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MessagePanel"));
	MessagePanel->SetBrush(FAuraUIStyle::RoundedOutlined(FLinearColor(0.f, 0.f, 0.f, 0.f), 9.f, FLinearColor(0.98f, 0.79f, 0.32f, 0.5f), 1.5f));
	MessagePanel->SetPadding(FMargin(14.f, 9.f));
	MessagePanel->SetVerticalAlignment(VAlign_Center);
	{
		UOverlaySlot* PanelSlot = Root->AddChildToOverlay(MessagePanel);
		PanelSlot->SetHorizontalAlignment(HAlign_Fill);
		PanelSlot->SetVerticalAlignment(VAlign_Fill);
	}

	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
	MessagePanel->SetContent(Row);

	/* 图标：没有图标就整块折叠掉 */
	{
		IconBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("IconBox"));
		IconBox->SetWidthOverride(30.f);
		IconBox->SetHeightOverride(30.f);
		IconBox->SetVisibility(ESlateVisibility::Collapsed);

		Icon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("MessageIcon"));
		IconBox->AddChild(Icon);

		UHorizontalBoxSlot* PanelSlot = Row->AddChildToHorizontalBox(IconBox);
		PanelSlot->SetVerticalAlignment(VAlign_Center);
		PanelSlot->SetPadding(FMargin(0.f, 0.f, 9.f, 0.f));
	}

	MessageText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MessageText"));
	FAuraUIStyle::StyleText(MessageText, 16, FAuraUIStyle::Accent(), true);
	Row->AddChildToHorizontalBox(MessageText)->SetVerticalAlignment(VAlign_Center);

	/* 出场前先"缩小 + 缩到右侧 + 隐身"，避免第一帧闪一下 */
	SetRenderScale(FVector2D(PopStartScale));
	SetRenderTranslation(FVector2D(SlideInDistance, 0.f));
	SetRenderOpacity(0.f);
}

void UAuraMessageWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ApplyMessage();
}

void UAuraMessageWidget::ApplyMessage()
{
	if (!bHasMessage) return;

	if (MessageText) MessageText->SetText(CachedMessage);

	if (IconBox && Icon)
	{
		if (CachedIcon)
		{
			Icon->SetBrushFromTexture(CachedIcon, true);
			IconBox->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			IconBox->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UAuraMessageWidget::SetMessage(const FText& InMessage, UTexture2D* InIcon)
{
	CachedMessage = InMessage;
	CachedIcon = InIcon;
	bHasMessage = true;

	/* 重开动画 */
	ElapsedTime = 0.f;

	ApplyMessage();
}

void UAuraMessageWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	ElapsedTime += InDeltaTime;

	float Alpha = 1.f;
	float OffsetX = 0.f;
	float Scale = 1.f;

	if (ElapsedTime < PopDuration)
	{
		/* ① 从右边蹦进来：BackOut 会先冲过头一点再回落，比线性滑入"弹"得多 */
		const float PopRatio = ElapsedTime / PopDuration;
		constexpr float C1 = 1.70158f;
		constexpr float C3 = C1 + 1.f;
		const float BackOut = 1.f + C3 * FMath::Pow(PopRatio - 1.f, 3.f) + C1 * FMath::Pow(PopRatio - 1.f, 2.f);

		OffsetX = FMath::Lerp(SlideInDistance, 0.f, BackOut);
		Scale = FMath::Lerp(PopStartScale, 1.f, BackOut);
		Alpha = FMath::Clamp(PopRatio / 0.5f, 0.f, 1.f);
	}
	else if (ElapsedTime >= FadeStart)
	{
		/* ② 淡出：一边变成全透明，一边往右飘走 */
		const float FadeRatio = FMath::Clamp((ElapsedTime - FadeStart) / FMath::Max(LifeTime - FadeStart, KINDA_SMALL_NUMBER), 0.f, 1.f);
		const float Eased = FMath::InterpEaseIn(0.f, 1.f, FadeRatio, 2.f);

		Alpha = 1.f - Eased;
		OffsetX = Eased * SlideOutDistance;
		Scale = 1.f - 0.05f * Eased;
	}

	SetRenderTranslation(FVector2D(OffsetX, 0.f));
	SetRenderScale(FVector2D(Scale));
	SetRenderOpacity(FMath::Clamp(Alpha, 0.f, 1.f));
}
