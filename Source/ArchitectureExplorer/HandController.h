// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <MotionControllerComponent.h>

#include "HandController.generated.h"

UCLASS()
class ARCHITECTUREEXPLORER_API AHandController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHandController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
//------------------------------------------------------------------------------------------------------------------------------------------------------

public:
	// Public Functions for setting up our controller
	void SetHand(EControllerHand Hand) { MotionController->SetTrackingSource(Hand); }
	void PairController(AHandController* Controller);
	void Grip();
	void Release();
//------------------------------------------------------------------------------------------------------------------------------------------------------

private:
	// Functions for setting up whether we can climb
	UFUNCTION()
	void ActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);
	
	UFUNCTION()
	void ActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

	bool CanClimb() const;
//------------------------------------------------------------------------------------------------------------------------------------------------------

private:
	UPROPERTY(VisibleAnywhere)
	UMotionControllerComponent* MotionController = nullptr;

	UPROPERTY(EditAnywhere)
	class UHapticFeedbackEffect_Base* HapticEffect;

	class APlayerController* PlayerController = nullptr;
	class ACharacter* Character = nullptr;

//------------------------------------------------------------------------------------------------------------------------------------------------------

private:
	AHandController* OtherController;

	bool bCanClimb = false;
	bool bIsClimbing = false;
	FVector CLimbingStartingLocation;

};
