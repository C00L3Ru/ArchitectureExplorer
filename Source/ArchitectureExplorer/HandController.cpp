// Fill out your copyright notice in the Description page of Project Settings.


#include "HandController.h"
#include <MotionControllerComponent.h>
#include <XRMotionControllerBase.h>

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

}

// Called every frame
void AHandController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHandController::ActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	bool bNewCanClimb = CanClimb();
	if (!bCanClimb && bNewCanClimb)
	{
		UE_LOG(LogTemp, Warning, TEXT("Object is Climbable"))
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

