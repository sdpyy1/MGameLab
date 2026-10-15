// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "IVHorrorPlayerController.generated.h"

class UInputMappingContext;
class UIVHorrorUI;

/**
 *  Player Controller for a first person horror game
 *  Manages input mappings
 *  Manages UI
 */
UCLASS(abstract, config="Game")
class IVSMOKEPROJECT_API AIVHorrorPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Type of UI widget to spawn */
	UPROPERTY(EditAnywhere, Category="Horror|UI")
	TSubclassOf<UIVHorrorUI> HorrorUIClass;

	/** Pointer to the UI widget */
	UPROPERTY()
	TObjectPtr<UIVHorrorUI> IVHorrorUI;

public:

	/** Constructor */
	AIVHorrorPlayerController();

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Gameplay Initialization */
	virtual void BeginPlay() override;

	/** Possessed pawn initialization */
	virtual void OnPossess(APawn* aPawn) override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;
};
