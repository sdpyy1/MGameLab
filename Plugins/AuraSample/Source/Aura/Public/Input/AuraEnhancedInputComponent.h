// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "AuraInputConfig.h"
#include "AuraEnhancedInputComponent.generated.h"

/**
 *
 */
UCLASS()
class AURA_API UAuraEnhancedInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
public:
	/*
		把 InputConfig 里配置的每一行 (IA → InputTag) 都绑到 Object（通常是 PlayerController）的回调上。

		同一个 InputAction 会绑三次，对应三种触发时机：
			Started   → pressFunc   （刚按下）
			Completed → releaseFunc （松开）
			Triggered → HeldFunc    （按住期间持续触发）

		InputTag 是作为"附加参数(payload)"绑进去的：Enhanced Input 会把它存进绑定里，
		事件触发时原样传给回调 —— 所以回调才能知道"这次触发的是哪个 InputTag"。
		（也正因为 payload 是实参，回调的参数必须按值接收，写成 const FGameplayTag& 会推导冲突。）
	*/
	template<class userClass, typename pressFuncType, typename releaseFuncType, typename HeldFuncType >
	void BindAbilityActions(const UAuraInputConfig* InputConfig, userClass* Object, pressFuncType pressFunc, releaseFuncType releaseFunc, HeldFuncType HeldFunc);
};

template<class userClass, typename pressFuncType, typename releaseFuncType, typename HeldFuncType>
void UAuraEnhancedInputComponent::BindAbilityActions(const UAuraInputConfig* InputConfig, userClass* Object, pressFuncType pressFunc, releaseFuncType releaseFunc, HeldFuncType HeldFunc)
{
	check(InputConfig);

	for (const FAuraInputAction Action : InputConfig->AbilityInputActions) {
		if (Action.InputAction) {
			if (pressFunc) {
				BindAction(Action.InputAction, ETriggerEvent::Started, Object, pressFunc, Action.InputTag);
			}
			if (releaseFunc) {
				BindAction(Action.InputAction, ETriggerEvent::Completed, Object, releaseFunc, Action.InputTag);
			}
			if (HeldFunc) {
				BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, HeldFunc, Action.InputTag);
			}
		}
	}
}
