// Copyright Epic Games, Inc. All Rights Reserved.

#include "ElectiveXCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "TimeRewindComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AElectiveXCharacter

AElectiveXCharacter::AElectiveXCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

}

void AElectiveXCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

//////////////////////////////////////////////////////////////////////////// Input

void AElectiveXCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AElectiveXCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AElectiveXCharacter::Look);
		
		// Rewind
		EnhancedInputComponent->BindAction(RewindAction, ETriggerEvent::Triggered, this, &AElectiveXCharacter::Rewind);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}



void AElectiveXCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AElectiveXCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AElectiveXCharacter::Rewind()
{
	UE_LOG(LogTemp, Log, TEXT("Rewind start"));

	// Cooldown check
	if (GetWorld()->GetTimerManager().IsTimerActive(RewindCooldownTimerHandle))
	{
		UE_LOG(LogTemp, Warning, TEXT("This is a debug message!"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	// Optional: Limit the search to actors within a certain radius
	FVector PlayerLocation = GetActorLocation();
	float RewindRadius = 5000.0f; // Adjust as needed

	TArray<AActor*> OverlappingActors;
	UGameplayStatics::GetAllActorsWithTag(
		World, 
		FName("Rewindable"),
		OverlappingActors
	);

	int32 RewindedActorsCount = 0;
	for (AActor* Actor : OverlappingActors)
	{
		if (!Actor) continue;

		// Optional: Distance check
		if (FVector::Distance(Actor->GetActorLocation(), PlayerLocation) > RewindRadius)
		{
			continue;
		}

		UTimeRewindComponent* RewindComp = Actor->FindComponentByClass<UTimeRewindComponent>();
		if (RewindComp)
		{
			RewindComp->StartTimeRewind();
			RewindedActorsCount++;
		}
	}

	// Only start cooldown if at least one actor was rewound
	if (RewindedActorsCount > 0)
	{
		// Start cooldown timer
		GetWorld()->GetTimerManager().SetTimer(
			RewindCooldownTimerHandle, 
			RewindCooldownDuration, 
			false  // Does not loop
		);

		// Optional: Add any additional effects (screen flash, sound, etc.)
		OnRewindSuccessful();
	}
}

void AElectiveXCharacter::OnRewindSuccessful()
{
	;
	// Optional: Add screen effects, sounds, etc.
	if (RewindCameraShake)
	{
		GetWorld()->GetFirstPlayerController()->PlayerCameraManager->StartCameraShake(RewindCameraShake);
	}

	// USoundBase* RewindSound;
	// // Play rewind sound
	// if (RewindSound)
	// {
	// 	UGameplayStatics::PlaySound2D(this, RewindSound);
	// }

	// Optional visual feedback
	// This could be a screen flash, particle effect, etc.
}
