// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Ability/AuraRangeAttackAbility.h"
#include <Interaction/CombatInterface.h>
#include <Actor/AuraProjectileActor.h>
#include <AbilitySystemBlueprintLibrary.h>
#include "AbilitySystemComponent.h"
#include "AuraGameplayTags.h"

void UAuraRangeAttackAbility::SpawnProjectile(const FVector& SpawnLocation, const FVector& TargetLocation)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	ICombatInterface* CombatAPI = Cast<ICombatInterface>(Avatar);
	if (CombatAPI) {
		FRotator SocketRotation = (TargetLocation - SpawnLocation).Rotation();
		SocketRotation.Pitch = 0.f;
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SpawnLocation);
		SpawnTransform.SetRotation(SocketRotation.Quaternion());
		AAuraProjectileActor* SpawnActor = GetWorld()->SpawnActorDeferred<AAuraProjectileActor>(ProjectileActorClass, SpawnTransform, GetOwningActorFromActorInfo(), Cast<APawn>(GetOwningActorFromActorInfo()), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Avatar);
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), ASC->MakeEffectContext());

		FAuraGameplayTags Tags = FAuraGameplayTags::Get();
		const float DamageEval = DamageComputer.GetValueAtLevel(GetAbilityLevel());
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Tags.Damage, DamageEval);
		SpawnActor->DamageEffectSpecHandle = SpecHandle;


		// 忽略持有者（Avatar）的碰撞，不然飞弹一出生就和自己碰上了
		UPrimitiveComponent* ProjectileCollision = Cast<UPrimitiveComponent>(SpawnActor->GetRootComponent());
		if (ProjectileCollision && IsValid(ProjectileCollision))
		{
			TArray<UPrimitiveComponent*> AttackerComponents;
			Avatar->GetComponents<UPrimitiveComponent>(AttackerComponents);

			for (UPrimitiveComponent* Comp : AttackerComponents)
			{
				if (IsValid(Comp) && Comp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
				{
					ProjectileCollision->IgnoreComponentWhenMoving(Comp, true);
					Comp->IgnoreComponentWhenMoving(ProjectileCollision, true);
				}
			}
		}
		SpawnActor->FinishSpawning(SpawnTransform);


	}
}
