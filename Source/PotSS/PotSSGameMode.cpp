// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "PotSS.h"
#include "PotSSGameMode.h"
#include "PotSSHUD.h"
#include "PotSSCharacter.h"

APotSSGameMode::APotSSGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Pirates/Blueprints/Player_FirstPerson_BP"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = APotSSHUD::StaticClass();
}
