// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/AuraGameOverWidget.h"

#include "UI/AuraUIStyle.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Kismet/KismetSystemLibrary.h"

void UAuraGameOverWidget::BuildWidgetTree()
{
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = Root;

	/* ① 全屏黑幕（带一点血色），把游戏画面整个压暗 */
	{
		UBorder* Scrim = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Scrim"));
		Scrim->SetBrush(FAuraUIStyle::Rounded(FLinearColor(0.035f, 0.004f, 0.008f, 0.88f), 0.f));

		UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(Scrim);
		PanelSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		PanelSlot->SetOffsets(FMargin(0.f));
	}

	/* ② 居中面板 */
	Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("GameOverPanel"));
	Panel->SetBrush(FAuraUIStyle::RoundedOutlined(FLinearColor(0.025f, 0.016f, 0.018f, 0.94f), 16.f, FLinearColor(0.72f, 0.10f, 0.10f, 0.85f), 2.f));
	Panel->SetPadding(FMargin(64.f, 42.f));
	Panel->SetHorizontalAlignment(HAlign_Center);
	Panel->SetVerticalAlignment(VAlign_Center);
	{
		UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(Panel);
		PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		PanelSlot->SetPosition(FVector2D::ZeroVector);
		PanelSlot->SetAutoSize(true);
	}

	UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>();
	Panel->SetContent(Box);

	/* --- 标题 --- */
	{
		UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("GameOverTitle"));
		FAuraUIStyle::StyleText(Title, 46, FLinearColor(0.86f, 0.13f, 0.11f, 1.f), true);
		Title->SetText(FText::FromString(TEXT("GAME OVER")));

		UVerticalBoxSlot* PanelSlot = Box->AddChildToVerticalBox(Title);
		PanelSlot->SetHorizontalAlignment(HAlign_Center);
	}

	/* --- 副标题 --- */
	{
		UTextBlock* SubTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("GameOverSubTitle"));
		FAuraUIStyle::StyleText(SubTitle, 17, FAuraUIStyle::TextMuted(), false);
		SubTitle->SetText(FText::FromString(TEXT("你已阵亡")));

		UVerticalBoxSlot* PanelSlot = Box->AddChildToVerticalBox(SubTitle);
		PanelSlot->SetHorizontalAlignment(HAlign_Center);
		PanelSlot->SetPadding(FMargin(0.f, 6.f, 0.f, 0.f));
	}

	/* --- 分隔线 --- */
	{
		USizeBox* DividerBox = WidgetTree->ConstructWidget<USizeBox>();
		DividerBox->SetHeightOverride(1.f);
		DividerBox->SetWidthOverride(360.f);

		UBorder* Divider = WidgetTree->ConstructWidget<UBorder>();
		Divider->SetBrush(FAuraUIStyle::Rounded(FLinearColor(0.72f, 0.10f, 0.10f, 0.45f), 0.5f));
		DividerBox->AddChild(Divider);

		Box->AddChildToVerticalBox(DividerBox)->SetPadding(FMargin(0.f, 22.f, 0.f, 26.f));
	}

	/* --- 退出游戏 --- */
	{
		USizeBox* ButtonBox = WidgetTree->ConstructWidget<USizeBox>();
		ButtonBox->SetWidthOverride(320.f);

		UButton* QuitButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("QuitButton"));
		QuitButton->SetStyle(FAuraUIStyle::DangerButtonStyle());
		QuitButton->OnClicked.AddDynamic(this, &UAuraGameOverWidget::HandleQuitClicked);
		{
			UTextBlock* QuitLabel = WidgetTree->ConstructWidget<UTextBlock>();
			FAuraUIStyle::StyleText(QuitLabel, 19, FLinearColor(0.96f, 0.85f, 0.84f, 1.f), true);
			QuitLabel->SetJustification(ETextJustify::Center);
			QuitLabel->SetText(FText::FromString(TEXT("退出游戏")));
			QuitButton->SetContent(QuitLabel);
		}
		ButtonBox->AddChild(QuitButton);

		Box->AddChildToVerticalBox(ButtonBox)->SetHorizontalAlignment(HAlign_Center);
	}

	/* 出场前先隐身，避免第一帧闪一下 */
	SetRenderOpacity(0.f);
	SetRenderScale(FVector2D(0.94f));
}

void UAuraGameOverWidget::HandleQuitClicked()
{
	/* PIE 下就是结束 PIE；打包后就是退出程序 */
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, /*bIgnorePlatformRestrictions=*/false);
}

void UAuraGameOverWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	ElapsedTime += InDeltaTime;

	/* 整屏淡入 */
	SetRenderOpacity(FMath::Clamp(ElapsedTime / FadeInDuration, 0.f, 1.f));

	/* 面板轻微弹出：BackOut 曲线，先冲过头再回落 */
	const float PopRatio = FMath::Clamp(ElapsedTime / PopDuration, 0.f, 1.f);
	constexpr float C1 = 1.70158f;
	constexpr float C3 = C1 + 1.f;
	const float BackOut = 1.f + C3 * FMath::Pow(PopRatio - 1.f, 3.f) + C1 * FMath::Pow(PopRatio - 1.f, 2.f);
	SetRenderScale(FVector2D(FMath::Lerp(0.94f, 1.f, BackOut)));
}
