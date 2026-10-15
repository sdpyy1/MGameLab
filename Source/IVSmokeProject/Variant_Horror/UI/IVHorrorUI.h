// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "IVHorrorUI.generated.h"

class AIVHorrorCharacter;

/**
 *  Simple UI for a first person horror game
 *  Manages character sprint meter display
 */
UCLASS(abstract)
class IVSMOKEPROJECT_API UIVHorrorUI : public UUserWidget
{
	GENERATED_BODY()
	
public:

	/** Sets up delegate listeners for the passed character */
	void SetupCharacter(AIVHorrorCharacter* IVHorrorCharacter);

	/** Called when the character's sprint meter is updated */
	UFUNCTION()
	void OnSprintMeterUpdated(float Percent);

	/** Called when the character's sprint state changes */
	UFUNCTION()
	void OnSprintStateChanged(bool bSprinting);

protected:

	/** Passes control to Blueprint to update the sprint meter widgets */
	UFUNCTION(BlueprintImplementableEvent, Category="Horror", meta = (DisplayName = "Sprint Meter Updated"))
	void BP_SprintMeterUpdated(float Percent);

	/** Passes control to Blueprint to update the sprint meter status */
	UFUNCTION(BlueprintImplementableEvent, Category="Horror", meta = (DisplayName = "Sprint State Changed"))
	void BP_SprintStateChanged(bool bSprinting);
};
