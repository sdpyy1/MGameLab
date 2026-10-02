// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Ability/AuraSummonAbility.h"

#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "AIController.h"
#include "BrainComponent.h"

/* =========================================================================================
 *	错峰生成（独立于能力生命周期）
 *	关键：把"待生成队列 / 施法者 / 类型"装进一个 TSharedRef 状态对象，
 *	挂在 Avatar 所在 World 的定时器上。能力即使提前 EndAbility 被回收，
 *	状态对象和 World 仍存活，剩下的小怪照样一只只蹦出来，不会半路断掉。
 * ========================================================================================= */

namespace
{
	struct FStaggerSpawnState
	{
		TArray<FVector> Locations;
		TSubclassOf<AActor> Class;
		TArray<TSubclassOf<AActor>> SummonClasses;	// 用于"没指定类型时随机挑"的兜底
		float LifeSpan = 0.f;
		float Interval = 0.4f;
		bool bDeferAI = true;	// 是否延迟到全部生成完才给 AI 控制器
		TArray<TWeakObjectPtr<APawn>> SpawnedPawns;	// 延迟期间收集，完成时统一接管
		TWeakObjectPtr<AActor> Avatar;
		TWeakObjectPtr<AActor> Owner;
		TWeakObjectPtr<UAuraSummonAbility> Ability;	// 仅用于广播 OnMinionSpawned，失效就跳过
		FTimerHandle TimerHandle;
	};

	/** 真正的生成逻辑：只依赖施法者/Avatar/World，不依赖能力对象本身 */
	AActor* SpawnSummonActorRuntime(
		AActor* Avatar,
		AActor* Owner,
		UWorld* World,
		TSubclassOf<AActor> InSummonClass,
		const FVector& SpawnLocation,
		float LifeSpan,
		bool bPauseAI = false)
	{
		if (!World || !InSummonClass.Get()) return nullptr;

		FRotator SpawnRotation = Avatar ? Avatar->GetActorRotation() : FRotator::ZeroRotator;
		SpawnRotation.Yaw += FMath::FRandRange(-30.f, 30.f);

		const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

		AActor* SpawnedActor = World->SpawnActorDeferred<AActor>(
			InSummonClass,
			SpawnTransform,
			Owner,
			Cast<APawn>(Avatar),
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

		if (!SpawnedActor) return nullptr;

		if (LifeSpan > 0.f)
		{
			SpawnedActor->SetLifeSpan(LifeSpan);
		}

		UGameplayStatics::FinishSpawningActor(SpawnedActor, SpawnTransform);

		/* 始终给控制器：保证 GAS / 血条在生成时就正常初始化（血条立刻出现） */
		if (APawn* SpawnedPawn = Cast<APawn>(SpawnedActor))
		{
			if (!SpawnedPawn->GetController())
			{
				SpawnedPawn->SpawnDefaultController();
			}

			/* 延迟放行：只冻结行为树（不派发移动指令）。
			   保留 Walking 移动模式，重力与贴地仍生效，角色会稳稳站在地面，只是不走动。 */
			if (bPauseAI)
			{
				if (AAIController* AIC = Cast<AAIController>(SpawnedPawn->GetController()))
				{
					if (AIC->BrainComponent)
					{
						AIC->BrainComponent->StopLogic(TEXT("DeferredSummon"));
					}
				}
			}
		}

		if (ACharacter* SpawnedCharacter = Cast<ACharacter>(SpawnedActor))
		{
			const float HalfHeight = SpawnedCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			SpawnedCharacter->SetActorLocation(SpawnedCharacter->GetActorLocation() + FVector(0.f, 0.f, HalfHeight));
		}

		return SpawnedActor;
	}

	AActor* AdvanceStaggeredSpawn(const TSharedRef<FStaggerSpawnState>& State)
	{
		AActor* Avatar = State->Avatar.Get();
		UWorld* World = Avatar ? Avatar->GetWorld() : nullptr;
		if (!World || State->Locations.Num() == 0) return nullptr;

		const FVector Location = State->Locations[0];
		State->Locations.RemoveAt(0);

		TSubclassOf<AActor> ClassToSpawn = State->Class.Get() ? State->Class : nullptr;
		if (!ClassToSpawn.Get() && State->SummonClasses.Num() > 0)
		{
			ClassToSpawn = State->SummonClasses[FMath::RandRange(0, State->SummonClasses.Num() - 1)];
		}

		/* 延迟放行时，生成即给控制器（血条立即可见），但冻结 AI 直到整批生成完 */
		const bool bPauseAI = State->bDeferAI;
		AActor* Spawned = SpawnSummonActorRuntime(Avatar, State->Owner.Get(), World, ClassToSpawn, Location, State->LifeSpan, bPauseAI);
		if (Spawned)
		{
			if (UAuraSummonAbility* Ability = State->Ability.Get())
			{
				Ability->OnMinionSpawned.Broadcast(Spawned);
			}

			/* 延迟期间把 Pawn 记录起来，等全部生成完统一接管 */
			if (State->bDeferAI)
			{
				if (APawn* SpawnedPawn = Cast<APawn>(Spawned))
				{
					State->SpawnedPawns.Add(SpawnedPawn);
				}
			}
		}

		if (State->Locations.Num() > 0)
		{
			FTimerDelegate TimerDelegate;
			TSharedRef<FStaggerSpawnState> StateCopy = State;
			TimerDelegate.BindLambda([StateCopy]()
			{
				AdvanceStaggeredSpawn(StateCopy);
			});

			World->GetTimerManager().SetTimer(State->TimerHandle, TimerDelegate, State->Interval, /*bLoop=*/false);
		}
		else if (State->bDeferAI)
		{
			/* 没有更多待生成 → 这是最后一只，统一"放行"所有已生成小怪的 AI（恢复行为树与移动） */
			for (TWeakObjectPtr<APawn>& WeakPawn : State->SpawnedPawns)
			{
				if (APawn* Pawn = WeakPawn.Get())
				{
					if (AAIController* AIC = Cast<AAIController>(Pawn->GetController()))
					{
						if (AIC->BrainComponent)
						{
							AIC->BrainComponent->RestartLogic();
						}
					}
				}
			}
		}

		return Spawned;
	}
}

/* =========================================================================================
 *	生成位置
 * ========================================================================================= */

TArray<FVector> UAuraSummonAbility::GetSpawnLocations() const
{
	const AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar) return TArray<FVector>();

	const FVector Center = Avatar->GetActorLocation();

	const int32 Count = FMath::Max(NumSpawn, 1);
	const float DeltaAngle = 360.f / static_cast<float>(Count);

	/* 半径上下限反过来也能正常工作，先夹一下更省心 */
	const float MinRadius = FMath::Min(MinSpawnDistance, MaxSpawnDistance);
	const float MaxRadius = FMath::Max(MinSpawnDistance, MaxSpawnDistance);

	TArray<FVector> SpawnLocations;
	SpawnLocations.Reserve(Count);

	for (int32 Index = 0; Index < Count; ++Index)
	{
		/* 这一份的起始角度 + 份内随机抖动；半径也随机 */
		const float Angle = Index * DeltaAngle + FMath::FRandRange(0.f, DeltaAngle);
		const float Radius = FMath::FRandRange(MinRadius, MaxRadius);

		const FVector Direction = FRotator(0.f, Angle, 0.f).Vector();
		FVector SpawnLocation = AdjustSpawnLocationToGround(Center + Direction * Radius);

		if (bDrawDebugSpawnLocations)
		{
			DrawDebugSphere(Avatar->GetWorld(), SpawnLocation, 18.f, 12, FColor::Orange, false, 5.f);
		}

		SpawnLocations.Add(SpawnLocation);
	}

	return SpawnLocations;
}

FVector UAuraSummonAbility::AdjustSpawnLocationToGround(const FVector& InLocation) const
{
	const AActor* Avatar = GetAvatarActorFromActorInfo();
	UWorld* World = Avatar ? Avatar->GetWorld() : nullptr;

	if (!bSnapToGround || !World) return InLocation;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AuraSummonGroundTrace), /*bTraceComplex=*/false);
	Params.AddIgnoredActor(Avatar);

	const FVector TraceStart = InLocation + FVector(0.f, 0.f, GroundTraceHalfHeight);
	const FVector TraceEnd = InLocation - FVector(0.f, 0.f, GroundTraceHalfHeight);

	FHitResult Hit;
	if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params) && Hit.bBlockingHit)
	{
		return Hit.ImpactPoint + FVector(0.f, 0.f, SpawnHeightOffset);
	}

	/* 没打到地面就用原来的高度，别把召唤物生成到虚空里 */
	return InLocation;
}

/* =========================================================================================
 *	召唤法阵
 *	核心：ShowSummonCircle 里先把位置定下来再放特效，召唤时沿用同一批坐标。
 * ========================================================================================= */

TArray<FVector> UAuraSummonAbility::ShowSummonCircle()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();

	/* 第一步：先把这一轮的生成位置抽出来并锁定 —— 这就是"最终生成 Actor 的位置" */
	CachedSpawnLocations = GetSpawnLocations();

	if (!SummonCircleEffect || CachedSpawnLocations.Num() == 0)
	{
		return CachedSpawnLocations;
	}

	/* 法阵朝向跟随施法者（多数法阵不看朝向，但给个合理值总没错） */
	const FRotator CircleRotation(0.f, Avatar ? Avatar->GetActorRotation().Yaw : 0.f, 0.f);

	if (bCircleAtEachSpawnLocation)
	{
		/* 一个生成点一个法阵：特效位置 = 召唤物位置 */
		for (const FVector& SpawnLocation : CachedSpawnLocations)
		{
			SpawnSummonCircleEffect(SpawnLocation, CircleRotation);
		}
	}
	else
	{
		/* 只放一个大法阵：摆在所有生成点的中心 */
		FVector Center = FVector::ZeroVector;
		for (const FVector& SpawnLocation : CachedSpawnLocations)
		{
			Center += SpawnLocation;
		}
		Center /= static_cast<float>(CachedSpawnLocations.Num());

		SpawnSummonCircleEffect(Center, CircleRotation);
	}

	return CachedSpawnLocations;
}

void UAuraSummonAbility::StopSummonCircle()
{
	for (const TObjectPtr<UNiagaraComponent>& CircleComponent : SummonCircleComponents)
	{
		if (IsValid(CircleComponent))
		{
			/* 组件是 bAutoDestroy 的，Deactivate 之后会自己销毁 */
			CircleComponent->Deactivate();
		}
	}

	SummonCircleComponents.Reset();
}

TArray<FVector> UAuraSummonAbility::TakeSpawnLocations()
{
	/* 法阵定过位置 → 用锁定的那一批，召唤物才会落在法阵上；
	   没定过（直接调 SummonActors）→ 现抽一批，行为和以前一致。
	   取走即复位：生成流程结束后缓存不会残留，不需要额外的重置接口。 */
	TArray<FVector> SpawnLocations = CachedSpawnLocations.Num() > 0 ? CachedSpawnLocations : GetSpawnLocations();
	CachedSpawnLocations.Reset();

	return SpawnLocations;
}

void UAuraSummonAbility::SpawnSummonCircleEffect(const FVector& InLocation, const FRotator& InRotation)
{
	UNiagaraComponent* CircleComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		this,
		SummonCircleEffect,
		InLocation + FVector(0.f, 0.f, SummonCircleZOffset),
		InRotation,
		SummonCircleScale,
		/*bAutoDestroy=*/true,
		/*bAutoActivate=*/true);

	if (CircleComponent)
	{
		SummonCircleComponents.Add(CircleComponent);
	}
}

/* =========================================================================================
 *	召唤
 * ========================================================================================= */

TSubclassOf<AActor> UAuraSummonAbility::GetRandomSummonClass() const
{
	if (SummonClasses.Num() == 0) return nullptr;

	const int32 Index = FMath::RandRange(0, SummonClasses.Num() - 1);
	return SummonClasses[Index];
}

TArray<AActor*> UAuraSummonAbility::SummonActors(TSubclassOf<AActor> InSummonClass, float LifeSpan)
{
	const AActor* Avatar = GetAvatarActorFromActorInfo();
	UWorld* World = Avatar ? Avatar->GetWorld() : nullptr;
	if (!World) return TArray<AActor*>();

	/* 法阵展示时锁定过的位置优先，保证召唤物落在法阵里 */
	const TArray<FVector> SpawnLocations = TakeSpawnLocations();

	TArray<AActor*> SpawnedActors;
	SpawnedActors.Reserve(SpawnLocations.Num());

	for (const FVector& SpawnLocation : SpawnLocations)
	{
		/* 没指定类型就每只都随机挑一个（课程里 Shaman 就是随机小怪） */
		const TSubclassOf<AActor> ClassToSpawn = InSummonClass.Get() ? InSummonClass : GetRandomSummonClass();
		if (!ClassToSpawn.Get()) continue;

		/* 同步生成：一次性全出来，生成即接管（错峰延迟只针对 SummonActorsStaggered） */
		if (AActor* Spawned = SpawnSummonActor(ClassToSpawn, SpawnLocation, LifeSpan))
		{
			SpawnedActors.Add(Spawned);
			OnMinionSpawned.Broadcast(Spawned);
		}
	}
	return SpawnedActors;
}

AActor* UAuraSummonAbility::SummonSingleActor(TSubclassOf<AActor> InSummonClass, float LifeSpan)
{
	const AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar || !Avatar->GetWorld()) return nullptr;

	const TArray<FVector> SpawnLocations = TakeSpawnLocations();
	if (SpawnLocations.Num() == 0) return nullptr;

	const TSubclassOf<AActor> ClassToSpawn = InSummonClass.Get() ? InSummonClass : GetRandomSummonClass();
	if (!ClassToSpawn.Get()) return nullptr;

	return SpawnSummonActor(ClassToSpawn, SpawnLocations[0], LifeSpan);
}

AActor* UAuraSummonAbility::SummonActorsStaggered(TSubclassOf<AActor> InSummonClass, float LifeSpan, float IntervalOverride)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	UWorld* World = Avatar ? Avatar->GetWorld() : nullptr;
	if (!World) return nullptr;

	/* 锁定的位置优先，保证落点跟法阵一致；取走即复位 */
	TArray<FVector> Locations = TakeSpawnLocations();
	if (Locations.Num() == 0) return nullptr;

	/* 间隔：显式传了就用传的，否则用蓝图配的 SpawnInterval */
	const float Interval = IntervalOverride >= 0.f ? IntervalOverride : SpawnInterval;

	/* 把这一轮召唤所需的一切装进一个 TSharedRef 状态对象：
	   它不挂在能力身上，所以能力提前 EndAbility 被回收也不会影响后续生成。 */
	TSharedRef<FStaggerSpawnState> State = MakeShared<FStaggerSpawnState>();
	State->Locations = MoveTemp(Locations);
	State->Class = InSummonClass;
	State->SummonClasses = SummonClasses;
	State->LifeSpan = LifeSpan;
	State->Interval = Interval;
	State->bDeferAI = bDeferAIUntilAllSpawned;
	State->Avatar = Avatar;
	State->Owner = GetOwningActorFromActorInfo();
	State->Ability = this;

	/* 第一只立即蹦出来（保证有返回值、也有第一下"蹦出"表现），其余交给 Advance 错峰；
	   是否延迟接管 AI 由 State->bDeferAI 统一控制，第一只也遵守同样的规则。 */
	return AdvanceStaggeredSpawn(State);
}

AActor* UAuraSummonAbility::SpawnSummonActor(TSubclassOf<AActor> InSummonClass, const FVector& SpawnLocation, float LifeSpan, bool bPauseAI)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	UWorld* World = Avatar ? Avatar->GetWorld() : nullptr;
	if (!World || !InSummonClass.Get()) return nullptr;

	/*
		延迟生成：SpawnActorDeferred 之后、FinishSpawningActor 之前，
		可以往召唤物身上塞参数（等级、所属 AI、外观…）。
		子类重写这个函数，在两步之间加自己的逻辑就行。
		实际生成逻辑放到 SpawnSummonActorRuntime，错峰路径也能复用、且不依赖本能力对象。
	*/
	return SpawnSummonActorRuntime(
		Avatar,
		GetOwningActorFromActorInfo(),
		World,
		InSummonClass,
		SpawnLocation,
		LifeSpan,
		bPauseAI);
}
