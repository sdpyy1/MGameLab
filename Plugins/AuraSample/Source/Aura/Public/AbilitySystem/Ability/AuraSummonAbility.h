// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AuraAttackGameplayAbility.h"
#include "AuraSummonAbility.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

/** 单只召唤物生成时广播的委托签名 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSummonMinionSpawnedSignature, AActor*, SpawnedActor);

UCLASS()
class AURA_API UAuraSummonAbility : public UAuraAttackGameplayAbility
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Summoning")
	TArray<FVector> GetSpawnLocations() const;

	UFUNCTION(BlueprintCallable, Category = "Summoning")
	TSubclassOf<AActor> GetRandomSummonClass() const;
	
	UFUNCTION(BlueprintCallable, Category = "Summoning")
	TArray<AActor*> SummonActors(TSubclassOf<AActor> InSummonClass, float LifeSpan = 0.f);

	/**
	 * 错峰（一只接一只）生成：第一只立即生成，其余按 SpawnInterval 间隔逐个蹦出。
	 * 返回值是第一只（若有），后续每只通过 OnMinionSpawned 逐只广播。
	 * 想让所有小怪一次性全出来就继续用 SummonActors。
	 */
	UFUNCTION(BlueprintCallable, Category = "Summoning")
	AActor* SummonActorsStaggered(TSubclassOf<AActor> InSummonClass, float LifeSpan = 0.f, float IntervalOverride = -1.f);

	UFUNCTION(BlueprintCallable, Category = "Summoning")
	AActor* SummonSingleActor(TSubclassOf<AActor> InSummonClass, float LifeSpan = 0.f);

	/** 每只召唤物真正生成时广播（错峰生成会逐只触发，可用来播"蹦出"特效/音效） */
	UPROPERTY(BlueprintAssignable, Category = "Summoning")
	FSummonMinionSpawnedSignature OnMinionSpawned;

	/* ============================ 召唤法阵 ============================ */

	/**
	 * 展示召唤法阵：
	 *  1) 先按当前配置抽出这一轮的生成位置并锁定（这就是"最终生成 Actor 的位置"）；
	 *  2) 再在每个生成点上播放 Niagara 法阵（SummonCircleEffect）；
	 *  3) 返回锁定的位置。
	 * 之后调用 SummonActors / SummonSingleActor，召唤物一定落在法阵所在的位置上。
	 */
	UFUNCTION(BlueprintCallable, Category = "Summoning|Circle")
	TArray<FVector> ShowSummonCircle();

	/** 提前收起法阵（不调用也可以，特效播完会自动销毁） */
	UFUNCTION(BlueprintCallable, Category = "Summoning|Circle")
	void StopSummonCircle();

protected:
	FVector AdjustSpawnLocationToGround(const FVector& InLocation) const;
	
	AActor* SpawnSummonActor(TSubclassOf<AActor> InSummonClass, const FVector& SpawnLocation, float LifeSpan, bool bPauseAI = false);

	/** 取这一次召唤要用的位置：法阵锁定过就用锁定的（取走即复位），否则现抽一批 */
	TArray<FVector> TakeSpawnLocations();

	/** 在指定位置播一个法阵特效并记录句柄 */
	void SpawnSummonCircleEffect(const FVector& InLocation, const FRotator& InRotation);

public:
	/* ============================ 配置（在蓝图子类里配） ============================ */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Summoning")
	int32 NumSpawn = 5;

	/** 错峰生成时每只之间的间隔（秒）；配合 SummonActorsStaggered 使用 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Summoning")
	float SpawnInterval = 0.4f;

	/** true：所有小怪全部生成完成后才统一给 AI 控制器（生成期间它们不会行动）；
	 *  false：生成一只立刻接管一只（原行为） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Summoning")
	bool bDeferAIUntilAllSpawned = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Summoning")
	TArray<TSubclassOf<AActor>> SummonClasses;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Summoning")
	float MinSpawnDistance = 180.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Summoning")
	float MaxSpawnDistance = 420.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Summoning")
	bool bSnapToGround = true;

	/** 贴地射线的上下探测高度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Summoning")
	float GroundTraceHalfHeight = 400.f;

	/** 贴地之后再抬多高（角色会自动再抬半个胶囊高，这里只留一点点余量即可） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Summoning")
	float SpawnHeightOffset = 10.f;

	/** 调试：在生成点画球，方便调半径和高度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Summoning|Debug")
	bool bDrawDebugSpawnLocations = false;

	/* ============================ 法阵配置 ============================ */

	/** 召唤法阵 Niagara：会在"最终生成 Actor 的位置"上播放 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Summoning|Circle")
	TObjectPtr<UNiagaraSystem> SummonCircleEffect;

	/** 法阵缩放（法阵大小和召唤半径对不上时用它对齐） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Summoning|Circle")
	FVector SummonCircleScale = FVector(1.f);

	/** 法阵在生成点基础上再抬高的高度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Summoning|Circle")
	float SummonCircleZOffset = 0.f;

	/** true：每个生成点各放一个法阵；false：只在阵型中心放一个 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Summoning|Circle")
	bool bCircleAtEachSpawnLocation = true;

protected:
	/** 本轮锁定的生成位置：ShowSummonCircle 写入，召唤时消费 */
	UPROPERTY(Transient)
	TArray<FVector> CachedSpawnLocations;

	/** ShowSummonCircle 生成的法阵组件，StopSummonCircle 统一收起 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UNiagaraComponent>> SummonCircleComponents;
};
