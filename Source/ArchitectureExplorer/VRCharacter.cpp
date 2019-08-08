// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacter.h"
#include <Camera/CameraComponent.h>
#include <Components/InputComponent.h>
#include <Components/SceneComponent.h>
#include <Components/CapsuleComponent.h>
#include <Components/StaticMeshComponent.h>
#include <GameFramework/PlayerController.h>
#include "Public/TimerManager.h"
#include <NavigationSystem.h>



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

bool AVRCharacter::FindTeleportDestination(FVector& OutLocation)
{
	FHitResult HitResult;
	FVector Start = VRCamera->GetComponentLocation();
	FVector End = Start + VRCamera->GetForwardVector() * MaxTeleportDistance;

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);

	if (!bHit) return false;

	UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());
	FNavLocation NavLocation;

	bool bOnNavMesh = NavigationSystem->ProjectPointToNavigation(HitResult.Location, NavLocation, TeleportProjectionExtent);

	if (!bOnNavMesh) return false;


	OutLocation = NavLocation.Location;
	return true;
}


void AVRCharacter::UpdateDestinationMarker()
{
	FVector Location;
	bool bHasDeistinastion = FindTeleportDestination(Location);

	// If we hit something and were on the NavMesh
	if (bHasDeistinastion)
	{
		bCanTeleport = true;
		TeleportDesinationMarker->SetVisibility(true);
		TeleportDesinationMarker->SetWorldLocation(Location);		// Move our marker
	}
	else
	{
		bCanTeleport = false;
		TeleportDesinationMarker->SetVisibility(false);		// Turn off our marker
	}
}

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveLeft_Y"), this, &AVRCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveLeft_X"), this, &AVRCharacter::MoveRight);
	PlayerInputComponent->BindAction(TEXT("TelePortLeft"), IE_Released, this, &AVRCharacter::BeginTelePort);
}

void AVRCharacter::MoveForward(float Throttle)
{
	AddMovementInput(VRCamera->GetForwardVector(), Throttle);
}

void AVRCharacter::MoveRight(float Throttle)
{
	AddMovementInput(VRCamera->GetRightVector(), Throttle);
}

void AVRCharacter::BeginTelePort()
{
	if (!bCanTeleport) { return; }
	StartFade(0, 1);	// Fade camera out

	// Timer Setup so we can fade out before we move to new location.
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AVRCharacter::EndTeleport, CameraFadeTime);
}

void AVRCharacter::EndTeleport()
{
	StartFade(1, 0);	// Fade camera in
	SetActorLocation(TeleportDesinationMarker->GetComponentLocation() + FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
}

// Function to Fade in or out when Teleporting
void AVRCharacter::StartFade(float FromAlpha, float ToAlpha)
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (PlayerController != nullptr)
	{
		PlayerController->PlayerCameraManager->StartCameraFade(FromAlpha, ToAlpha, CameraFadeTime, FLinearColor::Black, false, true);
	}

}
