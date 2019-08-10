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
#include <Kismet/GameplayStatics.h>
#include <Components/SplineComponent.h>
#include <Components/SplineMeshComponent.h>
#include "HandController.h"

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

	TeleportPath = CreateDefaultSubobject<USplineComponent>(TEXT("Teleport Path"));
	TeleportPath->SetupAttachment(VRRoot); // TODO attach to our controllers

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

	// Setup of our MotionControllers using our HandController actor.
	LeftMotionController = GetWorld()->SpawnActor<AHandController>(HandControllerClass);
	LeftMotionController->SetOwner(this);

	if (LeftMotionController != nullptr)
	{
		LeftMotionController->AttachToComponent(VRRoot,FAttachmentTransformRules::KeepRelativeTransform);
		LeftMotionController->SetHand(EControllerHand::Left);
	}

	RightMotionController = GetWorld()->SpawnActor<AHandController>(HandControllerClass);
	RightMotionController->SetOwner(this);
	if (RightMotionController != nullptr)
	{
		RightMotionController->AttachToComponent(VRRoot, FAttachmentTransformRules::KeepRelativeTransform);
		RightMotionController->SetHand(EControllerHand::Right);
	}

	// Setup of our Blinker Material
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

void AVRCharacter::UpdateDestinationMarker()
{
	FVector Location;
	TArray<FVector> Path;
	bool bHasDeistinastion = FindTeleportDestination(Path, Location);

	// If we hit something and were on the NavMesh
	if (bHasDeistinastion)
	{
		bCanTeleport = true;
		TeleportDesinationMarker->SetVisibility(true);
		TeleportDesinationMarker->SetWorldLocation(Location);		// Move our marker
		DrawTeleportPath(Path);
	}
	else
	{
		TArray<FVector> EmptyPath;
		bCanTeleport = false;
		TeleportDesinationMarker->SetVisibility(false);		// Turn off our marker
		DrawTeleportPath(EmptyPath);
	}
}

// Use PredictProjectilePath() to get a Parabolic curve for a visual guide for teleporting onto our NavMesh
bool AVRCharacter::FindTeleportDestination(TArray<FVector>& OutPath, FVector& OutLocation)
{
	FVector Start = LeftMotionController->GetActorLocation();
	FVector Look = LeftMotionController->GetActorForwardVector();
	
	FPredictProjectilePathParams PredictParams(
		TeleportProjectileRadius,
		Start,
		Look * TeleportProjectileSpeed,
		TeleportSimulationTime,
		ECollisionChannel::ECC_Camera,
		this);
	
	FPredictProjectilePathResult PredictResult;
	bool bHit = UGameplayStatics::PredictProjectilePath(this, PredictParams, PredictResult);

	if (!bHit) return false;

	for (FPredictProjectilePathPointData PointData: PredictResult.PathData)
	{
		OutPath.Add(PointData.Location);
	}

	UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());
	FNavLocation NavLocation;
	bool bOnNavMesh = NavigationSystem->ProjectPointToNavigation(PredictResult.HitResult.Location, NavLocation, TeleportProjectionExtent);

	if (!bOnNavMesh) return false;

	OutLocation = NavLocation.Location;
	return true;
}

void AVRCharacter::DrawTeleportPath(const TArray<FVector>& Path)
{
	UpdateSpline(Path);

	for (USplineMeshComponent* SplineMesh : TeleportPathMeshPool)
	{
		SplineMesh->SetVisibility(false);
	}

	int32 SegmentNum = Path.Num() - 1;

	for (int32 i = 0; i < SegmentNum; ++i)
	{
		if (TeleportPathMeshPool.Num() <= i)
		{
			USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
			SplineMesh->SetMobility(EComponentMobility::Movable);
			SplineMesh->AttachToComponent(TeleportPath, FAttachmentTransformRules::KeepRelativeTransform);
			SplineMesh->SetStaticMesh(TeleportArcMesh);
			SplineMesh->SetMaterial(0, TeleportArcMaterial);
			SplineMesh->RegisterComponent();
			TeleportPathMeshPool.Add(SplineMesh);
		}
		USplineMeshComponent* SplineMesh = TeleportPathMeshPool[i];
		SplineMesh->SetVisibility(true);

		FVector StartPosition, StartTangent, EndPosition, EndTangent;

		TeleportPath->GetLocalLocationAndTangentAtSplinePoint(i, StartPosition, StartTangent);
		TeleportPath->GetLocalLocationAndTangentAtSplinePoint(i+1, EndPosition, EndTangent);

		SplineMesh->SetStartAndEnd(StartPosition, StartTangent, EndPosition, EndTangent, true);
	}
}

void AVRCharacter::UpdateSpline(const TArray<FVector>& Path)
{
	TeleportPath->ClearSplinePoints(false);

	for (int32 i = 0; i < Path.Num(); ++i)
	{
		FVector LocalPosition = TeleportPath->GetComponentTransform().InverseTransformPosition(Path[i]);
		FSplinePoint Point(i, LocalPosition, ESplinePointType::Curve);
		TeleportPath->AddPoint(Point, false);
	}
	TeleportPath->UpdateSpline();
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

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveLeft_Y"), this, &AVRCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveLeft_X"), this, &AVRCharacter::MoveRight);
	PlayerInputComponent->BindAction(TEXT("TelePortLeft"), IE_Released, this, &AVRCharacter::BeginTelePort);
	PlayerInputComponent->BindAction(TEXT("GrabLeft"), IE_Pressed, this, &AVRCharacter::GripLeft);
	PlayerInputComponent->BindAction(TEXT("GrabRight"), IE_Pressed, this, &AVRCharacter::GripRight);
	PlayerInputComponent->BindAction(TEXT("GrabLeft"), IE_Released, this, &AVRCharacter::ReleaseLeft);
	PlayerInputComponent->BindAction(TEXT("GrabRight"), IE_Released, this, &AVRCharacter::ReleaseRight);

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
	FVector Destination = TeleportDesinationMarker->GetComponentLocation();
	Destination += GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * GetActorUpVector();

	StartFade(1, 0);	// Fade camera in
	SetActorLocation( Destination );
}

// Function to Fade in or out when Teleporting
void AVRCharacter::StartFade(float FromAlpha, float ToAlpha)
{
	if (PlayerController != nullptr)
	{
		PlayerController->PlayerCameraManager->StartCameraFade(FromAlpha, ToAlpha, CameraFadeTime, FLinearColor::Black, false, true);
	}
}
