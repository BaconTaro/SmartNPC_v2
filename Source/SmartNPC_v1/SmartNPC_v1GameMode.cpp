// Copyright Epic Games, Inc. All Rights Reserved.

#include "SmartNPC_v1GameMode.h"
#include "SmartNPC_v1HUD.h"
#include "SmartNPC_v1Character.h"
#include "UObject/ConstructorHelpers.h"

ASmartNPC_v1GameMode::ASmartNPC_v1GameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ASmartNPC_v1HUD::StaticClass();
}
