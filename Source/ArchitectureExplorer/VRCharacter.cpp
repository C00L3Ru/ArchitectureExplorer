// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacter.h"
#include <Camera/CameraComponent.h>
#include <Components/InputComponent.h>
#include <Components/SceneComponent.h>
#include <Components/CapsuleComponent.h>

// Sets default values
AVRCharacter::AVRCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a new Root component for our character
	VRRoot = CreateDefaultSubobject<USceneComponent>(FName("VR Root"));
	VRRoot->SetupAttachment(GetRootComponent());

	// Setup  camera
	VRCamera = CreateDefaultSubobject<UCameraComponent>(FName("VR Camera"));
	VRCamera->SetupAttachment(VRRoot);

}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
 
	// Stored variables for calculating how far our charater should move in relation to our lcamera
 	FVector VRCameraOffset = VRCamera->GetComponentLocation() - GetActorLocation();
	VRCameraOffset.Z = 0.f;
	// Move Character to camera location
	AddActorWorldOffset(VRCameraOffset);
	//Move VRRoot back to original location
	VRRoot->AddWorldOffset(-VRCameraOffset);
 
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

