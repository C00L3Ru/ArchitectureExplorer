// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacter.h"
#include <Camera/CameraComponent.h>
#include <Components/InputComponent.h>
#include <Components/SceneComponent.h>
#include <Components/CapsuleComponent.h>
#include <Components/StaticMeshComponent.h>

// Sets default values
AVRCharacter::AVRCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a new Root component for our character
	VRRoot = CreateDefaultSubobject<USceneComponent>(FName("VR Root"));
	VRRoot->SetupAttachment(GetRootComponent());

	// Setup  Camera
	VRCamera = CreateDefaultSubobject<UCameraComponent>(FName("VR Camera"));
	VRCamera->SetupAttachment(VRRoot);

	TeleportDesinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(FName("Teleport Destination Marker"));
	TeleportDesinationMarker->SetupAttachment(VRRoot);

}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();
	TeleportDesinationMarker->SetVisibility(false);
}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
 
	//	Move  our capsule component to our VRCamera
 	FVector VRCameraOffset = VRCamera->GetComponentLocation() - GetActorLocation();	
	VRCameraOffset.Z = 0.f;		// Leave the height axis alone
	AddActorWorldOffset(VRCameraOffset);	// Move Character with Capsule Component attached to VRCamera location
	VRRoot->AddWorldOffset(-VRCameraOffset);	// Move VRRoot back to original location (middle of our play space).
 
	UpdateDestinationMarker();
}

//LineTrace to place our TeleportDestinationMarker
void AVRCharacter::UpdateDestinationMarker()
{
	FHitResult HitResult;
	FVector Start = VRCamera->GetComponentLocation();
	FVector End = Start + VRCamera->GetForwardVector() * MaxTeleportDistance;
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Camera);

	// If we hit something
	if (bHit)
	{
		TeleportDesinationMarker->SetWorldLocation(HitResult.Location);		// Move our marker
		TeleportDesinationMarker->SetVisibility(true);
	}
	else
	{
		TeleportDesinationMarker->SetVisibility(false);		// Turn off our marker
	}
}

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveLeft_Y"), this, &AVRCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveLeft_X"), this, &AVRCharacter::MoveRight);
}

void AVRCharacter::MoveForward(float Throttle)
{
	AddMovementInput(VRCamera->GetForwardVector(), Throttle);
}

void AVRCharacter::MoveRight(float Throttle)
{
	AddMovementInput(VRCamera->GetRightVector(), Throttle);
}

