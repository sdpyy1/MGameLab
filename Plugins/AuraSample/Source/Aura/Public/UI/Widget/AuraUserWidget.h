// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AuraUserWidget.generated.h"

/**
 *	所有 Aura 界面的基类。
 *
 *	本项目不使用 Widget Blueprint：界面全部在 C++ 里用 WidgetTree->ConstructWidget<>() 搭出来。
 *	子类只需要重写 BuildWidgetTree()，不需要任何 .uasset。
 */
UCLASS()
class AURA_API UAuraUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UObject* InController);

	/*
		Widget -> Controller
	*/
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UObject> WidgetController;

protected:
	/**
	 *	在这里用 WidgetTree->ConstructWidget<T>(T::StaticClass(), TEXT("名字")) 创建控件，
	 *	最后别忘了给 WidgetTree->RootWidget 配上根控件。
	 */
	virtual void BuildWidgetTree() {}

	/**
	 *	订阅委托。
	 *	@return true 表示这次真的订上了；如果控制器还没准备好可以返回 false，基类会在之后重试。
	 */
	virtual bool BindWidgetDelegates() { return true; }

	/** UUserWidget 生成 Slate 控件前会调用这里，保证控件树一定已经搭好 */
	virtual TSharedRef<SWidget> RebuildWidget() override;

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/*
		给一个Widget设置Controller时，会触发WidgetControllerSet()事件，这个事件可以在一个Widget的蓝图中来监听
	*/
	UFUNCTION(BlueprintImplementableEvent)
	void WidgetControllerSet();

private:
	/**
	 *	尝试订阅委托：控件树就绪 + 子类订阅成功 才算完成。
	 *	SetWidgetController / NativeConstruct / NativeTick 都会调它，
	 *	所以不管"控件树"和"控制器"哪个先来，最多一帧之后一定能订上。
	 */
	void TryBindDelegates();

	/** 防止控件树被重复搭建 */
	bool bWidgetTreeBuilt = false;

	/** 防止委托被重复绑定 */
	bool bDelegatesBound = false;
};
