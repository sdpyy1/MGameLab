// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/AuraUserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"

/*
	给一个Widget设置Controller时，会触发WidgetControllerSet()事件，这个事件可以在一个Widget的蓝图中来监听
*/
void UAuraUserWidget::SetWidgetController(UObject* InController)
{
	this->WidgetController = InController;

	/* 控件树可能已经搭好了（例如世界空间组件），这里立刻补一次订阅 */
	TryBindDelegates();

	WidgetControllerSet();
}

TSharedRef<SWidget> UAuraUserWidget::RebuildWidget()
{
	/*
		纯 C++ 界面：没有 Widget Blueprint，所以 WidgetTree 得自己建。
		UUserWidget::Initialize() 里也会兜底创建，但显式建出来更直观、也不依赖引擎版本细节。
	*/
	if (WidgetTree == nullptr)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	}

	/* 第一次生成 Slate 控件之前，把 C++ 控件树搭好 */
	if (!bWidgetTreeBuilt && WidgetTree->RootWidget == nullptr)
	{
		bWidgetTreeBuilt = true;
		BuildWidgetTree();

		ensureMsgf(WidgetTree->RootWidget != nullptr,
			TEXT("%s::BuildWidgetTree() 没有给 WidgetTree->RootWidget 赋值，界面会是空的"),
			*GetClass()->GetName());
	}

	return Super::RebuildWidget();
}

void UAuraUserWidget::TryBindDelegates()
{
	if (bDelegatesBound) return;

	/* 控件树还没搭好就先不订，之后 NativeConstruct / NativeTick 还会再来 */
	if (WidgetTree == nullptr || WidgetTree->RootWidget == nullptr) return;

	/* 子类可能因为控制器还没就绪而拒绝订阅，那就下次再试 */
	if (BindWidgetDelegates())
	{
		bDelegatesBound = true;
	}
}

void UAuraUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	TryBindDelegates();
}

void UAuraUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	/* 兜底：无论控件树和控制器谁先谁后，最多一帧之后一定订上 */
	if (!bDelegatesBound)
	{
		TryBindDelegates();
	}
}
