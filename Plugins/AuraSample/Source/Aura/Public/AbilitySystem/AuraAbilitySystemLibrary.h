// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AuraAbilitySystemLibrary.generated.h"
struct FTaggedMontage;
class UAuraOverlayWidgetController;
class UAuraAttributeMenuController;
/**
 *	一些全局可用的静态函数
 */
UCLASS()
class AURA_API UAuraAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibary|WidgetController")
	static UAuraOverlayWidgetController* GetOverlayWidgetController(const UObject* UWorldContextObject);

	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibary|WidgetController")
	static UAuraAttributeMenuController* GetAttributeMenuController(const UObject* UWorldContextObject);
	
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibary|Utils", meta = (WorldContext = "WorldContextObject"))
	static FVector GetMouseTargetLocation(const UObject* WorldContextObject);

	/* 查询一个球形范围内的活的角色 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibary|GameplayMechanics")
	static void GetLivePlayerWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, const FVector& SphereLocation, float Radius);

	/* 在一个TaggedMontageArray中选择一个Montage*/
	UFUNCTION(BlueprintCallable, Category = "Utils")
	static FTaggedMontage GetRandomTaggedMontage(TArray<FTaggedMontage> AttackMontages);

	
	/* 判断Second是不是敌人 */
	UFUNCTION(BlueprintCallable, Category = "Utils")
	static bool isEnemy(AActor * First, AActor * Second);
};
