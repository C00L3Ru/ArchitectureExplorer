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
//------------------------------------------------------------------------------------------------------------------------------------------------------
	// Functions used for Teleportation
	bool FindTeleportDestination(FVector& OutLocation);
	void UpdateDestinationMarker();
	void StartFade(float FromAlpha, float ToAlpha);
	void UpdateBlinkers();
	FVector2D GetBlinkersCenter();

//------------------------------------------------------------------------------------------------------------------------------------------------------
	// Functions for Input Bindings
	void MoveForward(float Throttle);
	void MoveRight(float Throttle);
	void BeginTelePort();
	void EndTeleport();

//------------------------------------------------------------------------------------------------------------------------------------------------------

private:
	// Forward Declarations
	class USceneComponent* VRRoot = nullptr;
	class APlayerController* PlayerController;
	
	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* VRCamera = nullptr;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* TeleportDesinationMarker = nullptr;

//------------------------------------------------------------------------------------------------------------------------------------------------------
	// Forward Declarations used to create our material on our post processing to create out Blinkers
	UPROPERTY(EditAnywhere)
	class UMaterialInterface* BlinkerMaterialBase = nullptr; // Base material used to create a Dynamic Material Instance

	class UPostProcessComponent* PostProcessComponent = nullptr;
	class UMaterialInstanceDynamic* BlinkerInstanceDynamic = nullptr;

	UPROPERTY(EditAnywhere)
	class UCurveFloat* RadiusVsVelocity = nullptr;	// Curve asset that is used to alter the radius of our Blinkers material based on our movement speed.
//------------------------------------------------------------------------------------------------------------------------------------------------------

private:
	// Variables to use with TelePorting
	UPROPERTY(EditAnywhere)
	float MaxTeleportDistance = 1000.f;	// Distance of line-trace from the VRCamera.
	
	UPROPERTY(EditAnywhere)
	float CameraFadeTime = 1.f;

	UPROPERTY(EditAnywhere)
	FVector TeleportProjectionExtent = FVector(100.f, 100.f, 100.f);

//------------------------------------------------------------------------------------------------------------------------------------------------------
	// Simple booleans for using Blinkers or Enhanced Blinkers
	UPROPERTY(EditAnywhere)
	bool bCanUseBlinkers = false;

	UPROPERTY(EditAnywhere)
	bool bCanUseEnhancedBlinkers = false;

	float Radius = 0.f;	//	Used for setting the Radius of The Blinkers
//------------------------------------------------------------------------------------------------------------------------------------------------------
	
	bool bCanTeleport = false;
	
};
