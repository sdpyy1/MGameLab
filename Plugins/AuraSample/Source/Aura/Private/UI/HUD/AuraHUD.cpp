// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/AuraHUD.h"

#include "UI/Widget/AuraOverlayWidget.h"
#include "UI/Widget/AuraAttributeMenuWidget.h"
#include "UI/Widget/AuraGameOverWidget.h"
#include "UI/WidgetController/AuraOverlayWidgetController.h"
#include "UI/WidgetController/AuraAttributeMenuController.h"
#include "Player/AuraPlayerController.h"

UAuraOverlayWidgetController* AAuraHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	if (OverlayWidgetController == nullptr)
	{
		/* BP 里没配就退回 C++ 类，保证纯 C++ 也能跑起来 */
		if (OverlayWidgetControllerClass)
		{
			OverlayWidgetController = NewObject<UAuraOverlayWidgetController>(this, OverlayWidgetControllerClass.Get());
		}
		else
		{
			OverlayWidgetController = NewObject<UAuraOverlayWidgetController>(this);
		}

		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		OverlayWidgetController->BindCallbacksToDependencies();
	}
	return OverlayWidgetController;
}

UAuraAttributeMenuController* AAuraHUD::GetAttributeMenuController(const FWidgetControllerParams& WCParams)
{
	if (AttributeMenuController == nullptr)
	{
		if (AttributeMenuControllerClass)
		{
			AttributeMenuController = NewObject<UAuraAttributeMenuController>(this, AttributeMenuControllerClass.Get());
		}
		else
		{
			AttributeMenuController = NewObject<UAuraAttributeMenuController>(this);
		}

		AttributeMenuController->SetWidgetControllerParams(WCParams);
		AttributeMenuController->BindCallbacksToDependencies();
	}
	return AttributeMenuController;
}

void AAuraHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAuraAbilitySystemComponent* ASC, UAuraAttributeSet* AS)
{
	checkf(PC, TEXT("InitOverlay 需要一个有效的 PlayerController"));

	/* 属性菜单之后要用同一套参数，先缓存 */
	CachedWidgetControllerParams = FWidgetControllerParams(PC, PS, ASC, AS);
	bHasCachedParams = true;

	/* ① 常驻 HUD：纯 C++ 控件，不看 BP 里配的 OverlayWidgetClass */
	OverlayWidget = CreateWidget<UAuraOverlayWidget>(PC, UAuraOverlayWidget::StaticClass());
	checkf(OverlayWidget, TEXT("创建 UAuraOverlayWidget 失败"));

	OverlayWidget->SetWidgetController(GetOverlayWidgetController(CachedWidgetControllerParams));

	/*
		顺序很重要：
		先 AddToViewport（此时 NativeConstruct 跑完、委托订阅完成），
		再广播初始值，控件才收得到。
	*/
	OverlayWidget->AddToViewport();

	OverlayWidgetController->BroadcastInitialValues();
}

void AAuraHUD::ToggleAttributeMenu()
{
	if (bAttributeMenuOpen)
	{
		CloseAttributeMenu();
	}
	else
	{
		OpenAttributeMenu();
	}
}

void AAuraHUD::OpenAttributeMenu()
{
	if (bAttributeMenuOpen || !bHasCachedParams) return;

	APlayerController* PC = CachedWidgetControllerParams.PlayerController;
	if (!PC) return;

	/* 懒创建：第一次打开时才建控件和控制器 */
	if (AttributeMenuWidget == nullptr)
	{
		AttributeMenuWidget = CreateWidget<UAuraAttributeMenuWidget>(PC, UAuraAttributeMenuWidget::StaticClass());
		if (!AttributeMenuWidget) return;

		AttributeMenuWidget->SetWidgetController(GetAttributeMenuController(CachedWidgetControllerParams));

		/* ZOrder 给高一点，压在常驻 HUD 上面 */
		AttributeMenuWidget->AddToViewport(10);
		AttributeMenuController->BroadcastInitialValues();
	}

	AttributeMenuWidget->SetVisibility(ESlateVisibility::Visible);
	bAttributeMenuOpen = true;

	/* 菜单打开期间把输入交给 UI，避免点菜单顺手把攻击技能放出去 */
	if (AAuraPlayerController* AuraPC = Cast<AAuraPlayerController>(PC))
	{
		AuraPC->SetAttributeMenuInputMode(true);
	}
}

void AAuraHUD::CloseAttributeMenu()
{
	if (!bAttributeMenuOpen) return;
	bAttributeMenuOpen = false;

	if (AttributeMenuWidget)
	{
		AttributeMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (AAuraPlayerController* AuraPC = Cast<AAuraPlayerController>(CachedWidgetControllerParams.PlayerController))
	{
		AuraPC->SetAttributeMenuInputMode(false);
	}
}

void AAuraHUD::ShowGameOver()
{
	if (bGameOverShown) return;
	bGameOverShown = true;

	/* 属性菜单先关掉，免得两个界面叠在一起 */
	CloseAttributeMenu();

	/* 常驻 HUD 收起来：人都倒了，血条蓝条没意义了 */
	if (OverlayWidget)
	{
		OverlayWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	APlayerController* PC = CachedWidgetControllerParams.PlayerController;
	if (!PC)
	{
		PC = GetOwningPlayerController();
	}
	if (!PC) return;

	GameOverWidget = CreateWidget<UAuraGameOverWidget>(PC, UAuraGameOverWidget::StaticClass());
	if (!GameOverWidget) return;

	/* ZOrder 给最高，压在所有界面上面 */
	GameOverWidget->AddToViewport(100);

	/* 输入全部交给 UI：移动与技能都不再响应 */
	if (AAuraPlayerController* AuraPC = Cast<AAuraPlayerController>(PC))
	{
		AuraPC->EnterGameOverState();
	}
}
