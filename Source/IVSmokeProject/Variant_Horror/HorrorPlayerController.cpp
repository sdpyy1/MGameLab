// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Horror/IVHorrorPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "IVSmokeProjectCameraManager.h"
#include "IVHorrorCharacter.h"
#include "IVHorrorUI.h"
#include "IVSmokeProject.h"
#include "Widgets/Input/SVirtualJoystick.h"

AIVHorrorPlayerController::AIVHorrorPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AIVSmokeProjectCameraManager::StaticClass();
}

void AIVHorrorPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogIVSmokeProject, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AIVHorrorPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	// only spawn UI on local player controllers
	if (IsLocalPlayerController())
	{
		// set up the UI for the character
		if (AIVHorrorCharacter* IVHorrorCharacter = Cast<AIVHorrorCharacter>(aPawn))
		{
			// create the UI
			if (!IVHorrorUI)
			{
				IVHorrorUI = CreateWidget<UIVHorrorUI>(this, HorrorUIClass);
				IVHorrorUI->AddToViewport(0);
			}

			IVHorrorUI->SetupCharacter(IVHorrorCharacter);
		}
	}
	
}

void AIVHorrorPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}	
}

bool AIVHorrorPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
