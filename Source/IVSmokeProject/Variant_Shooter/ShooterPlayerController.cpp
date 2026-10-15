// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/IVShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "IVShooterCharacter.h"
#include "IVShooterBulletCounterUI.h"
#include "IVSmokeProject.h"
#include "Widgets/Input/SVirtualJoystick.h"

void AIVShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController())
	{
		if (ShouldUseTouchControls())
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

		// create the bullet counter widget and add it to the screen
		BulletCounterUI = CreateWidget<UIVShooterBulletCounterUI>(this, BulletCounterUIClass);

		if (BulletCounterUI)
		{
			BulletCounterUI->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogIVSmokeProject, Error, TEXT("Could not spawn bullet counter widget."));

		}
		
	}
}

void AIVShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// add the input mapping contexts
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

void AIVShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// subscribe to the pawn's OnDestroyed delegate
	InPawn->OnDestroyed.AddDynamic(this, &AIVShooterPlayerController::OnPawnDestroyed);

	// is this a shooter character?
	if (AIVShooterCharacter* IVShooterCharacter = Cast<AIVShooterCharacter>(InPawn))
	{
		// add the player tag
		IVShooterCharacter->Tags.Add(PlayerPawnTag);

		// subscribe to the pawn's delegates
		IVShooterCharacter->OnBulletCountUpdated.AddDynamic(this, &AIVShooterPlayerController::OnBulletCountUpdated);
		IVShooterCharacter->OnDamaged.AddDynamic(this, &AIVShooterPlayerController::OnPawnDamaged);

		// force update the life bar
		IVShooterCharacter->OnDamaged.Broadcast(1.0f);
	}
}

void AIVShooterPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	// reset the bullet counter HUD
	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_UpdateBulletCounter(0, 0);
	}

	// find the player start
	TArray<AActor*> ActorList;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), ActorList);

	if (ActorList.Num() > 0)
	{
		// select a random player start
		AActor* RandomPlayerStart = ActorList[FMath::RandRange(0, ActorList.Num() - 1)];

		// spawn a character at the player start
		const FTransform SpawnTransform = RandomPlayerStart->GetActorTransform();

		if (AIVShooterCharacter* RespawnedCharacter = GetWorld()->SpawnActor<AIVShooterCharacter>(CharacterClass, SpawnTransform))
		{
			// possess the character
			Possess(RespawnedCharacter);
		}
	}
}

void AIVShooterPlayerController::OnBulletCountUpdated(int32 MagazineSize, int32 Bullets)
{
	// update the UI
	if (BulletCounterUI)
	{
		BulletCounterUI->BP_UpdateBulletCounter(MagazineSize, Bullets);
	}
}

void AIVShooterPlayerController::OnPawnDamaged(float LifePercent)
{
	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_Damaged(LifePercent);
	}
}

bool AIVShooterPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
