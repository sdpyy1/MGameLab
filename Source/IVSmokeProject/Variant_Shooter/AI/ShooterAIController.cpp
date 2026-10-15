// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/AI/IVShooterAIController.h"
#include "IVShooterNPC.h"
#include "Components/StateTreeAIComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "AI/Navigation/PathFollowingAgentInterface.h"

AIVShooterAIController::AIVShooterAIController()
{
	// create the StateTree component
	StateTreeAI = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));
	StateTreeAI->SetStartLogicAutomatically(false);

	// create the AI perception component. It will be configured in BP
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

	// subscribe to the AI perception delegates
	AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AIVShooterAIController::OnPerceptionUpdated);
	AIPerception->OnTargetPerceptionForgotten.AddDynamic(this, &AIVShooterAIController::OnPerceptionForgotten);
}

void AIVShooterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// ensure we're possessing an NPC
	if (AIVShooterNPC* NPC = Cast<AIVShooterNPC>(InPawn))
	{
		// add the team tag to the pawn
		NPC->Tags.Add(TeamTag);

		// subscribe to the pawn's OnDeath delegate
		NPC->OnPawnDeath.AddDynamic(this, &AIVShooterAIController::OnPawnDeath);

		// start AI logic
		StateTreeAI->StartLogic();
	}
}

void AIVShooterAIController::OnPawnDeath()
{
	// stop movement
	GetPathFollowingComponent()->AbortMove(*this, FPathFollowingResultFlags::UserAbort);

	// stop StateTree logic
	StateTreeAI->StopLogic(FString(""));

	// unpossess the pawn
	UnPossess();

	// destroy this controller
	Destroy();
}

void AIVShooterAIController::SetCurrentTarget(AActor* Target)
{
	TargetEnemy = Target;
}

void AIVShooterAIController::ClearCurrentTarget()
{
	TargetEnemy = nullptr;
}

void AIVShooterAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// pass the data to the StateTree delegate hook
	OnShooterPerceptionUpdated.ExecuteIfBound(Actor, Stimulus);
}

void AIVShooterAIController::OnPerceptionForgotten(AActor* Actor)
{
	// pass the data to the StateTree delegate hook
	OnShooterPerceptionForgotten.ExecuteIfBound(Actor);
}
