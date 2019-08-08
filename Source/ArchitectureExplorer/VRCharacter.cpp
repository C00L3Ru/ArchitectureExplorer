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
#include <Components/PostProcessComponent.h>
#include <Materials/MaterialInstanceDynamic.h>
#include <MotionControllerComponent.h>
#include <XRMotionControllerBase.h>



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

// 	LMotionController = CreateDefaultSubobject<UMotionControllerComponent>(FName("LeftMotionController"));
// 	LMotionController->SetupAttachment(VRRoot);
// 	LMotionController->SetTrackingSource(EControllerHand::Left);
// 	RMotionController = CreateDefaultSubobject<UMotionControllerComponent>(FName("RightMotionController"));
// 	RMotionController->SetupAttachment(VRRoot);
// 	RMotionController->SetTrackingSource(EControllerHand::Right);

	RightMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	RightMotionController->SetTrackingMotionSource(FXRMotionControllerBase::RightHandSourceId);
	RightMotionController->SetShowDeviceModel(true);
	RightMotionController->SetupAttachment(VRRoot);

	LeftMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	LeftMotionController->SetTrackingMotionSource(FXRMotionControllerBase::LeftHandSourceId);
	LeftMotionController->SetShowDeviceModel(true);
	LeftMotionController->SetupAttachment(VRRoot);

	TeleportDesinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(FName("Teleport Destination Marker"));
	TeleportDesinationMarker->SetupAttachment(VRRoot);

	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(FName("Post Processing Component"));
	PostProcessComponent->SetupAttachment(VRRoot);


}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = Cast<APlayerController>(GetController());

	TeleportDesinationMarker->SetVisibility(false);

	
	if (BlinkerMaterialBase != nullptr) 
	{
		BlinkerInstanceDynamic = UMaterialInstanceDynamic::Create(BlinkerMaterialBase, this, FName("Blinker Material Instance"));
		PostProcessComponent->AddOrUpdateBlendable(BlinkerInstanceDynamic);
		return; 
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Blinker Material Base selected in BP_VRCharacter"))
	}
	
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
	if (bCanUseBlinkers == true) { UpdateBlinkers(); }
	
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
	if (PlayerController != nullptr)
	{
		PlayerController->PlayerCameraManager->StartCameraFade(FromAlpha, ToAlpha, CameraFadeTime, FLinearColor::Black, false, true);
	}
}

void AVRCharacter::UpdateBlinkers()
{
	if (RadiusVsVelocity == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Curve Float asset set in BP_VRCharacter!!!!"))
		return;
	}

	// Use our curve float to adjust our Blinkers strengh based on our Speed
	float Speed = GetVelocity().Size();
	Radius = RadiusVsVelocity->GetFloatValue(Speed);
	BlinkerInstanceDynamic->SetScalarParameterValue(FName("Radius"), Radius);

	
	// Used to Enhance our Blinkers
	FVector2D Center = GetBlinkersCenter();
	BlinkerInstanceDynamic->SetVectorParameterValue(FName("Center"), FLinearColor(Center.X, Center.Y, 0));
}

FVector2D AVRCharacter::GetBlinkersCenter()
{
	FVector MovementDirection = GetVelocity().GetSafeNormal();

	if (MovementDirection.IsNearlyZero() || PlayerController == nullptr || !bCanUseEnhancedBlinkers)
	{
		return FVector2D(.5f, .5f);		// Return default values if criteria isn't met 
	}

	// find out if were moving forwards or backwards and store the direction were looking in
	FVector WorldLocation;
	if(FVector::DotProduct(VRCamera->GetForwardVector(), MovementDirection) > 0)	
	{
		 WorldLocation = VRCamera->GetComponentLocation() + MovementDirection * 1000;
	}
	else
	{
		WorldLocation = VRCamera->GetComponentLocation() - MovementDirection * 1000;
	}

	// Convert the direction were looking into 2D
	FVector2D ScreenLocation; // OUT parameter
	PlayerController->ProjectWorldLocationToScreen(WorldLocation, ScreenLocation);
	
	// Divide out ScreenLocation with Out Screen Size
	int32 SizeX, SizeY;		  // OUT parameter
	PlayerController->GetViewportSize(SizeX, SizeY);
	ScreenLocation.X /= SizeX;
	ScreenLocation.Y /= SizeY;

	// Return the result so our Blinkers don't move in relation to our Head Movement (They stay fixed to the center of our viewport)
	return ScreenLocation;
}
