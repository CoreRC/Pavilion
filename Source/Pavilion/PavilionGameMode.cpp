// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "PavilionGameMode.h"
#include "PavilionPawn.h"
#include "PavilionHud.h"

APavilionGameMode::APavilionGameMode()
{
	DefaultPawnClass = APavilionPawn::StaticClass();
	HUDClass = APavilionHud::StaticClass();
}
