// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include <Kismet/GameplayStatics.h>
#include <UI/HUD/AuraHUD.h>
#include "Character/AuraCharacterBase.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include <AbilitySystem/AuraAbilitySystemComponent.h>
#include <AbilitySystem/AuraAttributeSet.h>

#include "Interaction/CombatInterface.h"
#include "Engine/OverlapResult.h"

UAuraOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* UWorldContextObject)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(UWorldContextObject, 0);
	check(PC);
	if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(PC->GetHUD())) {
		/* ASC/AttributeSet 现在挂在角色自身 */
		AAuraCharacterBase* AuraCharacter = Cast<AAuraCharacterBase>(PC->GetPawn());
		check(AuraCharacter);
		const FWidgetControllerParams WidgetControllerParams(PC, PC->PlayerState, AuraCharacter->GetAbilitySystemComponent(), AuraCharacter->GetAttributeSet());
		return AuraHUD->GetOverlayWidgetController(WidgetControllerParams);
	}
	return nullptr;
}

UAuraAttributeMenuController* UAuraAbilitySystemLibrary::GetAttributeMenuController(const UObject* UWorldContextObject)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(UWorldContextObject, 0);
	check(PC);
	if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(PC->GetHUD())) {
		/* ASC/AttributeSet 现在挂在角色自身 */
		AAuraCharacterBase* AuraCharacter = Cast<AAuraCharacterBase>(PC->GetPawn());
		check(AuraCharacter);
		const FWidgetControllerParams WidgetControllerParams(PC, PC->PlayerState, AuraCharacter->GetAbilitySystemComponent(), AuraCharacter->GetAttributeSet());
		return AuraHUD->GetAttributeMenuController(WidgetControllerParams);
	}
	return nullptr;
}

void UAuraAbilitySystemLibrary::GetLivePlayerWithinRadius(const UObject* WorldContextObject,
	TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, const FVector& SphereLocation,
	float Radius)
{
	FCollisionQueryParams SphereCollision;
	SphereCollision.AddIgnoredActors(ActorsToIgnore);

	TArray<FOverlapResult> Overlaps;
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	World->OverlapMultiByObjectType(Overlaps, SphereLocation, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(Radius), SphereCollision);
	for (FOverlapResult Overlap : Overlaps)
	{
		if (Overlap.GetActor())
		{
			if (const bool Implements_Combat = Overlap.GetActor()->Implements<UCombatInterface>())
			{

				if (Implements_Combat)
				{
					const bool isAlive = !ICombatInterface::Execute_isDead(Overlap.GetActor());
					if (isAlive)
					{
						OutOverlappingActors.AddUnique(Overlap.GetActor());
					}
				}
				
			}
			

			
		}
	}
}

FTaggedMontage UAuraAbilitySystemLibrary::GetRandomTaggedMontage(TArray<FTaggedMontage> AttackMontages)
{
	if (int num = AttackMontages.Num(); num > 0)
	{
		const int32 Index = FMath::RandRange(0, num - 1);
		return AttackMontages[Index];
	}
	return {};
}

bool UAuraAbilitySystemLibrary::isEnemy(AActor* First, AActor* Second)
{
	return First->ActorHasTag("Enemy")? Second->ActorHasTag("Player") : Second->ActorHasTag("Enemy");
}

FVector UAuraAbilitySystemLibrary::GetMouseTargetLocation(const UObject* WorldContextObject)
{
	const APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	if (!PC)
	{
		return FVector::ZeroVector;
	}

	FHitResult HitResult;
	if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		return HitResult.Location;
	}
	return FVector::ZeroVector;
}
