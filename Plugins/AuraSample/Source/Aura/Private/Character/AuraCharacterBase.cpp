// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/AuraCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include <AbilitySystem/AuraAbilitySystemComponent.h>
#include <AbilitySystem/AuraAttributeSet.h>

#include "AuraGameplayTags.h"
#include "Aura.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/Controller.h"
#include <Game/AuraGameModeBase.h>
#include <Kismet/GameplayStatics.h>

AAuraCharacterBase::AAuraCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	/* 所有角色的 ASC/AttributeSet 都挂在自己身上 */
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon"));
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

UAuraAbilitySystemComponent* AAuraCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAuraAttributeSet* AAuraCharacterBase::GetAttributeSet() const
{
	return AttributeSet;
}

UAnimMontage* AAuraCharacterBase::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}

int32 AAuraCharacterBase::GetPlayerLevel()
{
	return Level;
}

void AAuraCharacterBase::Die()
{
	HandleDeath();
}

FVector AAuraCharacterBase::GetCombatSocketLocation_Implementation(const FGameplayTag& AttackMethodTag)
{
	const FAuraGameplayTags NativeTags = FAuraGameplayTags::Get();
	if (AttackMethodTag == NativeTags.AttackMethod_Weapon)
	{
		// 武器攻击：取武器尖 Socket（Weapon 可能为空，需判空）
		if (IsValid(Weapon))
		{
			return Weapon->GetSocketLocation(WeaponTipSocketName);
		}
	}
	else if (AttackMethodTag == NativeTags.AttackMethod_LeftHand)
	{
		return  GetMesh()->GetSocketLocation(LeftHandSocketName);
	}
	else if (AttackMethodTag == NativeTags.AttackMethod_RightHand)
	{
		return  GetMesh()->GetSocketLocation(RightHandSocketName);
	}
	else if (AttackMethodTag == NativeTags.AttackMethod_Tail)
	{
		return GetMesh()->GetSocketLocation(TailSocketName);
	}
	return FVector();
}

bool AAuraCharacterBase::isDead_Implementation() const
{
	return bDead;
}

AActor* AAuraCharacterBase::GetAvatar_Implementation()
{
	return this;
}

void AAuraCharacterBase::HandleDeath()
{
	Weapon->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	Weapon->SetSimulatePhysics(true);
	Weapon->SetEnableGravity(true);
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Dissolve();
	bDead = true;
}


void AAuraCharacterBase::Dissolve()
{
	if (IsValid(DissolveMaterialInstance)) {
		UMaterialInstanceDynamic* DynamicMatInst = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicMatInst);
		StartDissolveTimeline(DynamicMatInst);
	}

	if (IsValid(WeaponDissolveMaterialInstance)) {
		UMaterialInstanceDynamic* WeaponDynamicMatInst = UMaterialInstanceDynamic::Create(WeaponDissolveMaterialInstance, this);
		Weapon->SetMaterial(0, WeaponDynamicMatInst);
		StartWeaponDissolveTimeline(WeaponDynamicMatInst);
	}
}

TArray<FTaggedMontage> AAuraCharacterBase::GetAttackMontages_Implementation()
{
	return AttackMontages;
}

UNiagaraSystem* AAuraCharacterBase::GetBloodEffect_Implementation()
{
	return BloodEffect;
}

void AAuraCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitGas();
}

void AAuraCharacterBase::InitGas()
{
	if (bGasInitialized || !IsValid(AbilitySystemComponent)) return;
	bGasInitialized = true;

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	AbilitySystemComponent->AbilityCharacterInfoSet();

	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
	UCharacterClassInfo* ClassInfo = AuraGameMode ? AuraGameMode->CharacterClassInfo : nullptr;
	if (!ClassInfo)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] GameMode 上的 CharacterClassInfo 未设置，角色不会获得初始属性与能力"), *GetName());
		return;
	}

	const FCharacterClassDefaultInfo& DefaultInfo = ClassInfo->GetClassDefaultInfo(CharacterClass);

	/* 初始属性：本角色档案的 Primary + 所有档案共用的 Secondary / Vital */
	AbilitySystemComponent->ApplyEffectToSelf(DefaultInfo.PrimaryAttributes, Level);
	AbilitySystemComponent->ApplyEffectToSelf(ClassInfo->SecondaryAttributes, Level);
	AbilitySystemComponent->ApplyEffectToSelf(ClassInfo->VitalAttributes, Level);

	/* 初始能力：所有档案通用的 + 本角色档案专属的 */
	AbilitySystemComponent->AddCharacterAbilities(ClassInfo->CommonAbilities);
	AbilitySystemComponent->AddCharacterAbilities(DefaultInfo.StartupAbilities);
}
