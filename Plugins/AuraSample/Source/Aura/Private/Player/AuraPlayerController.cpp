// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include <Interaction/EnemyInterface.h>
#include <Interaction/CombatInterface.h>
#include <Input/AuraEnhancedInputComponent.h>
#include <AbilitySystem/AuraAbilitySystemComponent.h>
#include <AbilitySystemBlueprintLibrary.h>
#include "Components/SplineComponent.h"
#include "AuraGameplayTags.h"
#include "NAvigationSystem.h"
#include "NavigationPath.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Character.h"
#include "kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "UI/Widget/DamageTextWidgetComponent.h"

AAuraPlayerController::AAuraPlayerController()
{

	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	check(AuraContext);

	/* ① 注册 InputMappingContext：物理按键（鼠标左键…）→ InputAction（IA_LMB…） */
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem) {
		Subsystem->AddMappingContext(AuraContext, 0);
	}
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	FInputModeGameAndUI inputModelData;
	inputModelData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	inputModelData.SetHideCursorDuringCapture(false);
	SetInputMode(inputModelData);
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	/* 死亡后不再做指针高亮与自动寻路 */
	if (IsAvatarDead()) return;

	CursorTrace();
	AutoRun();
}

bool AAuraPlayerController::IsAvatarDead() const
{
	APawn* ControlledPawn = GetPawn();
	return ControlledPawn && ICombatInterface::Execute_isDead(ControlledPawn);
}

void AAuraPlayerController::EnterGameOverState()
{
	/* 清掉自动寻路与瞄准状态 */
	bAutoRunning = false;
	bTargeting = false;
	FollowTime = 0.f;

	/* 屏蔽移动与视角输入 */
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);

	/* 输入全部交给 UI，保证"退出游戏"按钮点得到 */
	FInputModeUIOnly InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputModeData);
}

void AAuraPlayerController::AutoRun()
{
	if (!bAutoRunning) return;
	if (APawn* ControlPawn = GetPawn()) {
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
		ControlPawn->AddMovementInput(Direction);

		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if (DistanceToDestination <= AutoRunAcceptanceRadius) {
			bAutoRunning = false;
		}
	}
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UAuraEnhancedInputComponent* EnhancedInputComponent = CastChecked<UAuraEnhancedInputComponent>(InputComponent);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);

	/*
		②③ InputAction → GameplayTag → 绑定到回调
		按 DA_InputConfig 里配的对应关系逐行绑定：
		Started→Press、Completed→Release、Triggered→Held，
		并把每行的 InputTag 作为 payload 一起绑进去（流程总览见 AuraInputConfig.h 顶部）
	*/
	EnhancedInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPress, &ThisClass::AbilityInputTagRelease, &ThisClass::AbilityInputTagHeld);
}

void AAuraPlayerController::ShowDamageNumber(float Damage, ACharacter* Target)
{
	if (!IsValid(Target)) return;

	/* 纯 C++ 组件：样式与动画都在 UAuraDamageTextWidget 里 */
	UDamageTextWidgetComponent* DamageTextComponent = NewObject<UDamageTextWidgetComponent>(Target, UDamageTextWidgetComponent::StaticClass());
	if (!DamageTextComponent) return;

	DamageTextComponent->RegisterComponent();
	DamageTextComponent->AttachToComponent(Target->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	DamageTextComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	/* 在受击点上方随机散开，连续命中时数字不会完全重叠 */
	const FVector SpawnOffset(FMath::FRandRange(-30.f, 30.f), FMath::FRandRange(-30.f, 30.f), FMath::FRandRange(95.f, 125.f));
	DamageTextComponent->SetWorldLocation(Target->GetActorLocation() + SpawnOffset);

	DamageTextComponent->SetDamageText(Damage);
}

void AAuraPlayerController::SetAttributeMenuInputMode(bool bMenuOpen)
{
	if (bMenuOpen)
	{
		/* 菜单打开：输入全给 UI，点菜单不会顺手把攻击技能放出去 */
		FInputModeUIOnly InputModeData;
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputModeData);
	}
	else
	{
		/* 恢复游戏输入（和 BeginPlay 里的设置保持一致） */
		FInputModeGameAndUI InputModeData;
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputModeData.SetHideCursorDuringCapture(false);
		SetInputMode(InputModeData);
	}
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	/* 死亡后不接受移动输入 */
	if (IsAvatarDead()) return;

	const FVector2D InputAixVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>()) {
		ControlledPawn->AddMovementInput(ForwardDirection, InputAixVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAixVector.X);
	}
}

void AAuraPlayerController::CursorTrace()
{
	// Trace Cursor and HighLight Enemy!
	FHitResult cursorHit;
	GetHitResultUnderCursor(ECC_Visibility, false, cursorHit); // 需要把Enemy的碰撞预设的碰撞响应的Visibiliy通道设置为Block
	if (!cursorHit.bBlockingHit) return;
	LastActor = ThisActor;
	ThisActor = Cast<IEnemyInterface>(cursorHit.GetActor());
	if (LastActor == nullptr) {
		if (ThisActor != nullptr) {
			// HighLight
			ThisActor->HighlightActor();
		}
	}
	else {
		if (ThisActor == nullptr) {
			LastActor->UnHighlightActor();
		}
		else {
			if (ThisActor != LastActor) {
				LastActor->UnHighlightActor();
				ThisActor->HighlightActor();
			}
		}
	}
}

void AAuraPlayerController::AbilityInputTagPress(FGameplayTag GameplayTag)
{
	/* 死亡后不再响应任何技能输入 */
	if (IsAvatarDead()) return;

	/*
		按下时：用 if/else 按 Tag 分派"这个 Tag 对应的操作"
		- RMB：右键在这里是"角色控制键"，按下时记录光标是否锁定到敌人（进入瞄准），并打断自动寻路
		- 其它 Tag（LMB / 1~4…）：按下这一帧无需额外处理，真正的效果在 Held 里交给 ASC 去激活能力
	*/
	if (GameplayTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_RMB)) {
		bTargeting = ThisActor ? true : false;
		bAutoRunning = false;
	}
}

void AAuraPlayerController::AbilityInputTagRelease(FGameplayTag GameplayTag)
{
	/* 死亡后不再响应任何技能输入 */
	if (IsAvatarDead()) return;

	/*
		松开时：用 if/else 按 Tag 分派"这个 Tag 对应的操作"
		- 其它 Tag：交给 ASC 走"松开"逻辑（有些能力是松手才生效的）
		- RMB：
			· 之前已锁定到敌人(bTargeting) → 当作"按下并松开右键"，直接触发一次 Held 去放技能
			· 否则 → 当作"点地移动"：短按(≤ShortPressThreshold)时用导航系统求出路径，
			  塞进 Spline 并开启自动寻路
	*/
	if (!GameplayTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_RMB)) {
		if (GetAuraASC()) {
			GetAuraASC()->AbilityInputTagRelease(GameplayTag);
		}
		return;
	}
	if (bTargeting) {
		if (GetAuraASC()) {
			GetAuraASC()->AbilityInputTagHeld(GameplayTag);
		}
	}
	else {
		if (APawn* ControlPawn = GetPawn()) {
			if (FollowTime <= ShortPressThreshold) {
				
				// 通过导航系统获得起点终点之间一系列点，然后把他们加入到SplineComponent
				if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlPawn->GetActorLocation(), CachedDestination)) {
					Spline->ClearSplinePoints();
					for (const FVector& point : NavPath->PathPoints) {
						Spline->AddSplinePoint(point, ESplineCoordinateSpace::World);
						// DrawDebugSphere(GetWorld(), point, 8.f, 8, FColor::Red, false, 5.f);
					}
					if (NavPath->PathPoints.Num() > 0) {
						CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
						bAutoRunning = true;
						// 生成点击的例子效果
						UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ClickMoveNiagara, CachedDestination);
					}
				}
			}
		}
		FollowTime = 0.f;
		bTargeting = false;
	}
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag GameplayTag)
{
	/* 死亡后不再响应任何技能输入 */
	if (IsAvatarDead()) return;

	/*
		按住时：用 if/else 按 Tag 分派"这个 Tag 对应的操作"
		- 其它 Tag：直接交给 ASC，由它在已授予的能力里找 InputMappingTag 等于该 Tag 的能力并激活
		- RMB：
			· 已锁定到敌人 → 同样交给 ASC（对目标施法）
			· 否则 → 按住不放时朝光标方向持续移动（FollowTime 记录按住时长，供 Release 判断长/短按）
	*/
	if (!GameplayTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_RMB)) {
		if (GetAuraASC()) {
			GetAuraASC()->AbilityInputTagHeld(GameplayTag);
		}
		return;
	}

	if (bTargeting) {
		if (GetAuraASC()) {
			GetAuraASC()->AbilityInputTagHeld(GameplayTag);
		}
	}
	else {
		FollowTime += GetWorld()->GetDeltaSeconds();
		FHitResult Hit;
		if (GetHitResultUnderCursor(ECC_Visibility, false, Hit)) {
			CachedDestination = Hit.ImpactPoint;
		}

		if (APawn* ControlPawn = GetPawn()) {
			const FVector WorldDirection = (CachedDestination - ControlPawn->GetActorLocation()).GetSafeNormal();
			ControlPawn->AddMovementInput(WorldDirection);
		}
	}
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetAuraASC()
{
	if (!AuraAbilitySystemComponent) {
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return AuraAbilitySystemComponent;
}