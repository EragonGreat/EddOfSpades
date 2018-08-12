// Fill out your copyright notice in the Description page of Project Settings.

#include "IMGameMode.h"
#include "IMPlayerController.h"

AIMGameMode::AIMGameMode()
{

	PlayerControllerClass = AIMPlayerController::StaticClass();

}
