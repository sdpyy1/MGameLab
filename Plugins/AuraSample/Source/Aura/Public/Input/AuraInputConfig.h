// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
/*
 *	Input 使用的InputAction 关联了一个 GameplayTag
 */
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AuraInputConfig.generated.h"
class UInputAction;

/*
	【输入 → 技能】整条流程

		① IMC_AuraContext（InputMappingContext）
				物理按键（鼠标左键…）→ InputAction（IA_LMB…）

		② DA_InputConfig（本资产 UAuraInputConfig）
				InputAction → GameplayTag
				这个对应关系就配在下面的 AbilityInputActions 里

		③ UAuraEnhancedInputComponent::BindAbilityActions()
				遍历 ② 的每一行逐个绑定，
				并把 InputTag 作为"附加参数(payload)"绑进回调

		④ AAuraPlayerController::AbilityInputTagPress / Release / Held(Tag)
				按 Tag 用 if/else 分派"这个输入要做什么"
				（大多数 Tag 直接转给 ASC；RMB 这类则做角色控制/点地移动）

		⑤ UAuraAbilitySystemComponent::AbilityInputTagHeld / Release(Tag)
				在已授予的能力里找 DynamicAbilityTags 含该 Tag 的 Spec，激活它

		⑥ 具体能力（GA_FireBolt 等）
				它的 UAuraGameplayAbility::InputMappingTag 必须等于 ② 里配的 Tag，否则永远触发不到

	结论：一个输入要能打出能力，需要 ②(IA→Tag) 与 ⑥(能力的 InputMappingTag) 两处对齐。
*/
USTRUCT(BlueprintType)
struct FAuraInputAction
{
	GENERATED_BODY()

	/* 要绑定的输入动作（如 IA_LMB） */
	UPROPERTY(EditDefaultsOnly)
	const class UInputAction* InputAction = nullptr;

	/* 该输入动作对应的 GameplayTag（如 InputTag.LMB），绑定时会随回调一起传进 PlayerController */
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag = FGameplayTag();
};

/**
 *	输入配置：保存"InputAction ↔ GameplayTag"的对应关系（即上面流程的第 ② 步）
 */
UCLASS()
class AURA_API UAuraInputConfig : public UDataAsset
{
	GENERATED_BODY()
public:
	/*
		Return A InputAction By Tag From AbilityInputActions
	*/
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound);

	/* IA ↔ InputTag 对应关系表：一行 = 一个可触发的输入 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FAuraInputAction> AbilityInputActions;
};
