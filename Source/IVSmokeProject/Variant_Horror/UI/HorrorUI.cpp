// Copyright Epic Games, Inc. All Rights Reserved.


#include "IVHorrorUI.h"
#include "IVHorrorCharacter.h"

void UIVHorrorUI::SetupCharacter(AIVHorrorCharacter* IVHorrorCharacter)
{
	IVHorrorCharacter->OnSprintMeterUpdated.AddDynamic(this, &UIVHorrorUI::OnSprintMeterUpdated);
	IVHorrorCharacter->OnSprintStateChanged.AddDynamic(this, &UIVHorrorUI::OnSprintStateChanged);
}

void UIVHorrorUI::OnSprintMeterUpdated(float Percent)
{
	// call the BP handler
	BP_SprintMeterUpdated(Percent);
}

void UIVHorrorUI::OnSprintStateChanged(bool bSprinting)
{
	// call the BP handler
	BP_SprintStateChanged(bSprinting);
}
