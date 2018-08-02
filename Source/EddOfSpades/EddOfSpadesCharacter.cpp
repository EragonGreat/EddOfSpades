// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "EddOfSpadesCharacter.h"
#include "EddOfSpadesProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameConstants.h"
#include "EddOfSpadesPlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AEddOfSpadesCharacter

AEddOfSpadesCharacter::AEddOfSpadesCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	SetReplicates(true);

	Reach = 4.f;

	// Set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FP_Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FP_Camera"));
	FP_Camera->SetupAttachment(GetCapsuleComponent());
	FP_Camera->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FP_Camera->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	FP_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Mesh"));
	FP_Mesh->SetupAttachment(FP_Camera);
	FP_Mesh->bCastDynamicShadow = false;
	FP_Mesh->CastShadow = false;
	FP_Mesh->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	FP_Mesh->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	FP_Gun->SetupAttachment(RootComponent);

}

void AEddOfSpadesCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(FP_Mesh, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

}

//////////////////////////////////////////////////////////////////////////
// Input

void AEddOfSpadesCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("PerformAction", IE_Pressed, this, &AEddOfSpadesCharacter::OnPerformAction);
	PlayerInputComponent->BindAction("PerformSecondaryAction", IE_Pressed, this, &AEddOfSpadesCharacter::OnPerformSecondaryAction);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AEddOfSpadesCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AEddOfSpadesCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AEddOfSpadesCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AEddOfSpadesCharacter::LookUpAtRate);
}

bool AEddOfSpadesCharacter::LineTraceForBlock(const FVector& Start, const FVector& End, FIntVector& OutHitBlock, FIntVector& OutNormalBlock)
{
	// Line trace for blocks
	FHitResult OutHit;
	FCollisionObjectQueryParams QueryParams;
	QueryParams.AddObjectTypesToQuery(ECC_GameTraceChannel2);
	bool bBlockHit = GetWorld()->LineTraceSingleByObjectType(OutHit, Start, End, QueryParams);

	if(bBlockHit)
	{
		// Get the block location
		FVector BlockLocation = OutHit.ImpactPoint / GameConstants::BlockSize - (OutHit.ImpactNormal / 2.f);

		OutHitBlock = FIntVector(BlockLocation);
		OutNormalBlock = FIntVector(BlockLocation + OutHit.ImpactNormal);

		return true;
	}

	return false;
}

void AEddOfSpadesCharacter::OnPerformAction()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			const FRotator SpawnRotation = GetControlRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = FP_Camera->GetComponentLocation();

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			// spawn the projectile at the muzzle
			World->SpawnActor<AEddOfSpadesProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);

		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = FP_Mesh->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}


	// Trace for block to damage
	FVector Start = FP_Camera->GetComponentLocation();
	FVector End = Start + (FP_Camera->GetForwardVector() * Reach * GameConstants::BlockSize);
	FIntVector OutHitBlock, OutNormalBlock;

	if(LineTraceForBlock(Start, End, OutHitBlock, OutNormalBlock))
	{
		Cast<AEddOfSpadesPlayerController>(Controller)->ServerDamageBlock(OutHitBlock);
	}
}

void AEddOfSpadesCharacter::OnPerformSecondaryAction()
{

	// Trace for block to place
	FVector Start = FP_Camera->GetComponentLocation();
	FVector End = Start + (FP_Camera->GetForwardVector() * Reach * GameConstants::BlockSize);
	FIntVector OutHitBlock, OutNormalBlock;

	if(LineTraceForBlock(Start, End, OutHitBlock, OutNormalBlock))
	{

		AEddOfSpadesPlayerController* Controller = Cast<AEddOfSpadesPlayerController>(this->Controller);
			
		Controller->ServerPlaceBlock(OutNormalBlock, Controller->SelectedColor);
	}

}

void AEddOfSpadesCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AEddOfSpadesCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AEddOfSpadesCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AEddOfSpadesCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}
