// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/AuraEnemy.h"
#include <AbilitySystem/AuraAttributeSet.h>
#include <AbilitySystem/AuraAbilitySystemComponent.h>
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/Widget/AuraUserWidget.h"
#include "UI/Widget/AuraEnemyHealthBarWidget.h"
#include "AuraGameplayTags.h"
#include "AI/AuraAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"

AAuraEnemy::AAuraEnemy()
{
	//Cursor Trace In Visibility Channel For Enemy HighLight In PlayerController
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	/* ASC/AttributeSet 由基类 AAuraCharacterBase 统一创建，这里不再重复创建 */

	// 用movementComponent来更好的处理转向
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;

	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent());
}

void AAuraEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AuraAIController = Cast<AAuraAIController>(NewController);
	if (!AuraAIController)
	{
		/* 召唤出来的小怪如果没配 AI Controller Class 就会走到这里，只警告不要崩 */
		UE_LOG(LogTemp, Warning, TEXT("[%s] 接管者不是 AAuraAIController（%s），AI 不会启动：请检查敌人 BP 的 AI Controller Class"),
			*GetName(), *GetNameSafe(NewController));
		return;
	}

	if (!BehaviorTree)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] BehaviorTree 未设置，AI 不会启动：请检查敌人 BP 上的 BehaviorTree"), *GetName());
		return;
	}

	AuraAIController->GetBlackboardComponent()->InitializeBlackboard(*(BehaviorTree->BlackboardAsset));
	AuraAIController->RunBehaviorTree(BehaviorTree);
	AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("bHitReacting"), false);
	AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("bRangeAttacker"), CharacterClass == ECharacterClass::Ranger);
	AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("bElementalist"), CharacterClass == ECharacterClass::Elementalist);
}

void AAuraEnemy::Die()
{
	SetLifeSpan(LifeSpan);
	if (AuraAIController) AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("bDead"), true);

	/* 死亡后就别再挂着血条了 */
	if (HealthBar) HealthBar->SetVisibility(false);

	Super::Die();
}

void AAuraEnemy::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;
}

AActor* AAuraEnemy::GetCombatTarget_Implementation()
{
	return CombatTarget;
}

void AAuraEnemy::HighlightActor()
{
	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(250);
	Weapon->SetRenderCustomDepth(true);
	Weapon->SetCustomDepthStencilValue(250);
}

void AAuraEnemy::UnHighlightActor()
{
	GetMesh()->SetRenderCustomDepth(false);
	Weapon->SetRenderCustomDepth(false);
}

void AAuraEnemy::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

	/*
		血条统一用 C++ 控件。
		注意：SetWidgetClass() 只在"组件已经 BeginPlay"时才会重建内部控件实例，
		时机不对就会留下 BP 里那个旧 WBP，所以这里直接 CreateWidget + SetWidget，不依赖调用时机。
	*/
	HealthBar->SetWidgetSpace(EWidgetSpace::Screen);        // 屏幕空间：始终正对镜头、大小恒定
	HealthBar->SetDrawSize(FVector2D(180.f, 34.f));         // 比血条本身略大，给受击顿挫留出余量
	HealthBar->SetPivot(FVector2D(0.5f, 1.f));              // 以底边中点为锚点，血条正好悬在头顶
	HealthBar->SetTwoSided(true);

	UAuraEnemyHealthBarWidget* HealthBarWidget = CreateWidget<UAuraEnemyHealthBarWidget>(GetWorld(), UAuraEnemyHealthBarWidget::StaticClass());
	if (HealthBarWidget)
	{
		HealthBar->SetWidget(HealthBarWidget);
		HealthBarWidget->SetWidgetController(this);
	}

	// Bind Callback To ACS
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data) {
		OnHealthChangedDelegate.Broadcast(Data.NewValue);
		});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data) {
		OnMaxHealthChangedDelegate.Broadcast(Data.NewValue);
		});

	// Bind HitReactTag New or Remove to HitReactTagChanged()
	AbilitySystemComponent->RegisterGameplayTagEvent(FAuraGameplayTags::Get().Effects_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AAuraEnemy::HitReactTagChanged);

	OnHealthChangedDelegate.Broadcast(AttributeSet->GetHealth());
	OnMaxHealthChangedDelegate.Broadcast(AttributeSet->GetMaxHealth());
}
void AAuraEnemy::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? WalkSpeedAfterHitReact: BaseWalkSpeed;
	if (AuraAIController && AuraAIController->GetBlackboardComponent())  // Client is null
	{
		AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("bHitReacting"), bHitReacting);
	}
}


