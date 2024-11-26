// Copyright Epic Games, Inc. All Rights Reserved.

#include "ElectiveXGameMode.h"
#include "ElectiveXCharacter.h"
#include "UObject/ConstructorHelpers.h"

AElectiveXGameMode::AElectiveXGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
