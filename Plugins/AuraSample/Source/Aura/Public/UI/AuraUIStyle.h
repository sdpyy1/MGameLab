// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/TextBlock.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"

/**
 *	Aura 的 UI 样式库（纯 C++ 界面用，不依赖任何 WBP / 贴图资源）。
 *	所有配色、圆角刷、字体集中在这里，改一处整个界面都会跟着变。
 */
struct FAuraUIStyle
{
	/* ===================== 配色 ===================== */

	/* 面板 */
	static FLinearColor PanelFill()      { return FLinearColor(0.016f, 0.020f, 0.035f, 0.88f); }
	static FLinearColor PanelOutline()   { return FLinearColor(0.80f, 0.63f, 0.30f, 0.75f); }
	static FLinearColor RowFill()        { return FLinearColor(1.f, 1.f, 1.f, 0.04f); }
	static FLinearColor Divider()        { return FLinearColor(0.80f, 0.63f, 0.30f, 0.35f); }

	/* 文字 */
	static FLinearColor TextPrimary()    { return FLinearColor(0.94f, 0.93f, 0.87f, 1.f); }
	static FLinearColor TextMuted()      { return FLinearColor(0.66f, 0.65f, 0.61f, 1.f); }
	static FLinearColor Accent()         { return FLinearColor(0.98f, 0.79f, 0.32f, 1.f); }

	/* 血条 / 蓝条 */
	static FLinearColor HealthFill()     { return FLinearColor(0.62f, 0.07f, 0.09f, 1.f); }
	static FLinearColor HealthGhost()    { return FLinearColor(0.99f, 0.70f, 0.35f, 0.92f); }
	static FLinearColor ManaFill()       { return FLinearColor(0.07f, 0.32f, 0.80f, 1.f); }
	static FLinearColor ManaGhost()      { return FLinearColor(0.55f, 0.78f, 1.f, 0.92f); }
	static FLinearColor BarOutline()     { return FLinearColor(0.f, 0.f, 0.f, 0.80f); }
	static FLinearColor EnemyHealthFill(){ return FLinearColor(0.72f, 0.10f, 0.10f, 1.f); }

	/* 遮罩 */
	static FLinearColor Scrim()          { return FLinearColor(0.f, 0.f, 0.f, 0.55f); }

	/* ===================== 刷子 ===================== */

	/** 圆角纯色刷（Radius 传半径，想做成胶囊就传高度的一半） */
	static FSlateBrush Rounded(const FLinearColor& Fill, float Radius = 8.f)
	{
		FSlateRoundedBoxBrush Brush(Fill, Radius);
		return Brush;
	}

	/** 带描边的圆角刷 */
	static FSlateBrush RoundedOutlined(const FLinearColor& Fill, float Radius, const FLinearColor& OutlineColor, float OutlineWidth)
	{
		FSlateRoundedBoxBrush Brush(Fill, Radius, OutlineColor, OutlineWidth);
		return Brush;
	}

	/** 什么都不画（用于进度条背景，让下层的"残影条"透出来） */
	static FSlateBrush NoBrush()
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::NoDrawType;
		return Brush;
	}

	/** 进度条样式：透明背景 + 圆角纯色填充 */
	static FProgressBarStyle BarStyle(const FLinearColor& Fill, float Radius)
	{
		FProgressBarStyle Style;
		Style.SetBackgroundImage(NoBrush());
		Style.SetFillImage(Rounded(Fill, Radius));
		Style.SetMarqueeImage(NoBrush());
		return Style;
	}

	/** 通用按钮样式（深色底 + 金色描边，悬停变亮） */
	static FButtonStyle ButtonStyle()
	{
		FButtonStyle Style;
		Style.SetNormal(RoundedOutlined(FLinearColor(0.09f, 0.08f, 0.06f, 0.92f), 8.f, Accent(), 1.5f));
		Style.SetHovered(RoundedOutlined(FLinearColor(0.24f, 0.19f, 0.10f, 0.96f), 8.f, Accent(), 2.f));
		Style.SetPressed(RoundedOutlined(FLinearColor(0.34f, 0.26f, 0.13f, 0.98f), 8.f, Accent(), 2.f));
		Style.SetDisabled(RoundedOutlined(FLinearColor(0.07f, 0.07f, 0.07f, 0.70f), 8.f, FLinearColor(0.35f, 0.35f, 0.35f, 0.6f), 1.f));
		Style.SetNormalPadding(FMargin(18.f, 9.f));
		Style.SetPressedPadding(FMargin(18.f, 10.f));
		return Style;
	}

	/** 危险操作按钮（红描边，用于退出游戏之类的按钮） */
	static FButtonStyle DangerButtonStyle()
	{
		const FLinearColor DangerOutline(0.80f, 0.16f, 0.14f, 0.9f);

		FButtonStyle Style;
		Style.SetNormal(RoundedOutlined(FLinearColor(0.12f, 0.03f, 0.03f, 0.92f), 8.f, DangerOutline, 1.5f));
		Style.SetHovered(RoundedOutlined(FLinearColor(0.28f, 0.05f, 0.05f, 0.96f), 8.f, DangerOutline, 2.f));
		Style.SetPressed(RoundedOutlined(FLinearColor(0.38f, 0.07f, 0.06f, 0.98f), 8.f, DangerOutline, 2.f));
		Style.SetDisabled(RoundedOutlined(FLinearColor(0.07f, 0.07f, 0.07f, 0.70f), 8.f, FLinearColor(0.35f, 0.35f, 0.35f, 0.6f), 1.f));
		Style.SetNormalPadding(FMargin(18.f, 9.f));
		Style.SetPressedPadding(FMargin(18.f, 10.f));
		return Style;
	}

	/* ===================== 字体 ===================== */

	/** 引擎自带字体（Roboto）；换字体只需要改这里 */
	static FSlateFontInfo Font(int32 Size, bool bBold = false)
	{
		return FCoreStyle::GetDefaultFontStyle(bBold ? "Bold" : "Regular", Size);
	}

	/** 一步配置文字：字号 + 颜色 + 阴影（阴影保证在任何背景上都看得清） */
	static void StyleText(UTextBlock* Text, int32 Size, const FLinearColor& Color, bool bBold = false, bool bShadow = true)
	{
		if (!Text) return;

		Text->SetFont(Font(Size, bBold));
		Text->SetColorAndOpacity(FSlateColor(Color));

		if (bShadow)
		{
			Text->SetShadowOffset(FVector2D(1.f, 1.f));
			Text->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.85f));
		}
	}
};
