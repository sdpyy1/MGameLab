// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "AuraHUD.generated.h"

class UAuraOverlayWidget;
class UAuraOverlayWidgetController;
class UAuraAttributeMenuWidget;
class UAuraAttributeMenuController;
class UAuraGameOverWidget;
class UAuraAbilitySystemComponent;
class UAuraAttributeSet;

/**
 *	Aura 的 HUD：负责创建并持有 HUD 控件与对应的 WidgetController。
 *
 *	界面全部是 C++ 类（UAuraOverlayWidget / UAuraAttributeMenuWidget），
 *	不再需要 WBP_Overlay / WBP_AttributeMenu 之类的资产。
 */
UCLASS()
class AURA_API AAuraHUD : public AHUD
{
	GENERATED_BODY()

public:
	void InitOverlay(APlayerController* PC, APlayerState* PS, UAuraAbilitySystemComponent* ASC, UAuraAttributeSet* AS);

	UAuraOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& WCParams);
	UAuraAttributeMenuController* GetAttributeMenuController(const FWidgetControllerParams& WCParams);

	/** 开关属性菜单（右下角按钮和菜单里的关闭按钮都走这里） */
	void ToggleAttributeMenu();
	void OpenAttributeMenu();
	void CloseAttributeMenu();

	/** 角色死亡：收起常驻 HUD 与属性菜单，弹出 GameOver 界面并把输入交给 UI */
	void ShowGameOver();

	/** GameOver 界面是否已经弹过 */
	bool IsGameOverShown() const { return bGameOverShown; }

private:
	UPROPERTY()
	TObjectPtr<UAuraOverlayWidget> OverlayWidget;

	UPROPERTY()
	TObjectPtr<UAuraOverlayWidgetController> OverlayWidgetController;

	/* 属性菜单是懒创建的：第一次打开时才建控件 */
	UPROPERTY()
	TObjectPtr<UAuraAttributeMenuWidget> AttributeMenuWidget;

	UPROPERTY()
	TObjectPtr<UAuraAttributeMenuController> AttributeMenuController;

	UPROPERTY()
	TObjectPtr<UAuraGameOverWidget> GameOverWidget;

	/* 允许在 BP 里换成子类；留空就用 C++ 类 */
	UPROPERTY(EditAnywhere, Category = "WidgetController")
	TSubclassOf<UAuraOverlayWidgetController> OverlayWidgetControllerClass;

	UPROPERTY(EditAnywhere, Category = "WidgetController")
	TSubclassOf<UAuraAttributeMenuController> AttributeMenuControllerClass;

	/* 属性菜单要用和 HUD 同一套参数，先缓存下来 */
	UPROPERTY()
	FWidgetControllerParams CachedWidgetControllerParams;

	bool bHasCachedParams = false;
	bool bAttributeMenuOpen = false;
	bool bGameOverShown = false;
};
