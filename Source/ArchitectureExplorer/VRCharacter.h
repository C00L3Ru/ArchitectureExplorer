// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VRCharacter.generated.h"

UCLASS()
class ARCHITECTUREEXPLORER_API AVRCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVRCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	void UpdateDestinationMarker();

	// Functions for Input Bindings
	void MoveForward(float Throttle);
	void MoveRight(float Throttle);
	void BeginTelePort();
	void EndTeleport();
	

private:
	// Forward Declarations
	class USceneComponent* VRRoot = nullptr;

	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* VRCamera = nullptr;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* TeleportDesinationMarker = nullptr;

private:
	// Variables
	UPROPERTY(EditAnywhere)
	float MaxTeleportDistance = 1000.f;	// Distance of line-trace out to.
	
	UPROPERTY(EditAnywhere)
	float CameraFadeTime = 1.f;
};
