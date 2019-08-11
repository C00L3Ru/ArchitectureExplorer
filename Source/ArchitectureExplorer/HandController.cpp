// Fill out your copyright notice in the Description page of Project Settings.


#include "HandController.h"
#include <XRMotionControllerBase.h>
#include <GameFramework/MovementComponent.h>
#include <GameFramework/PlayerController.h>
#include "GameFramework/CharacterMovementComponent.h"
#include <GameFramework/Character.h>

// Sets default values
AHandController::AHandController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController"));
	//MotionController->SetTrackingMotionSource(FXRMotionControllerBase::RightHandSourceId);
	MotionController->SetShowDeviceModel(true);
	SetRootComponent(MotionController);
}

// Called when the game starts or when spawned
void AHandController::BeginPlay()
{
	Super::BeginPlay();
	OnActorBeginOverlap.AddDynamic(this, &AHandController::ActorBeginOverlap);
	OnActorEndOverlap.AddDynamic(this, &AHandController::ActorEndOverlap);
	PlayerController = GetWorld()->GetFirstPlayerController();
	ACharacter* Character = Cast<ACharacter>(GetAttachParentActor());
}

// Called every frame
void AHandController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsClimbing)
	{
		FVector HandControllerDelta = GetActorLocation() - CLimbingStartingLocation;
		GetAttachParentActor()->AddActorWorldOffset(-HandControllerDelta);
	}
}

void AHandController::PairController(AHandController* Controller)
{
	OtherController = Controller;
	OtherController->OtherController = this;
}

void AHandController::Grip()
{
	if (!bCanClimb) { return; }

	if (!bIsClimbing)
	{
		bIsClimbing = true;
		CLimbingStartingLocation = GetActorLocation();
		OtherController->bIsClimbing = false;

		if (Character != nullptr)
		{
			Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
		}
	}
}

void AHandController::Release()
{
	if (bIsClimbing)
	{
		bIsClimbing = false;
		if (Character != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("should be falling!!!"))
			Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		}
	}
}

void AHandController::ActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!PlayerController) { UE_LOG(LogTemp, Warning, TEXT("No Player Controller")) return; }
	
	bool bNewCanClimb = CanClimb();
	if (!bCanClimb && bNewCanClimb)
	{
		PlayerController->PlayHapticEffect(HapticEffect, MotionController->GetTrackingSource());
	}
	bCanClimb = bNewCanClimb;
}

void AHandController::ActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	bCanClimb = CanClimb();
}

bool AHandController::CanClimb() const
{
	TArray<AActor*> OverLappingActors;
	GetOverlappingActors(OverLappingActors);
	for (AActor* Actor : OverLappingActors)
	{
		if (Actor->ActorHasTag(FName("Climbable")))
		{
			return true;
		}
	}
	return false;
}

