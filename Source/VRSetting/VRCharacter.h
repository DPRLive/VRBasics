// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HandController.h"

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VRCharacter.generated.h"

UCLASS()
class VRSETTING_API AVRCharacter : public ACharacter
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

protected:
	
private:
	UPROPERTY() 
		class AHandController* LeftController;

	UPROPERTY() 
		class AHandController* RightController;

	UPROPERTY(VisibleAnywhere)
		class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere) //EditAnywhere는 포인터를 편집할수도 있다는 말이 될수 있어서 이거로함?
		class UStaticMeshComponent* DestinationMarker; // 텔레포트의 목적지, 콜리전 설정에서 visibility는 무시 해주어야함

	UPROPERTY(VisibleAnywhere)
		class USplineComponent* TeleportPath;

	UPROPERTY() //언리얼이 메모리 관리해줌ㅋ
		class USceneComponent* VRRoot;
	
	UPROPERTY() //언리얼이 메모리 관리해줌ㅋ
		class UMaterialInstanceDynamic* BlinkerMaterialInstance;

	UPROPERTY(EditAnywhere)
		class UMaterialInterface* BlinkerMaterialBase;

	UPROPERTY() // Blinker를 위한 포스트프로세스 컴포넌트
		class UPostProcessComponent* PostProcessComponent;

	UPROPERTY(EditAnywhere) //동적인 Blink를 위한 커브 
		class UCurveFloat* RadiusVsVelocity;

	UPROPERTY()
		TArray<class USplineMeshComponent*> TeleportPathMeshPool;

	UPROPERTY(EditDefaultsOnly) // StaticMeshComponent의 SetStaticMesh를 위해
		class UStaticMesh* TeleportArchMesh;

	UPROPERTY(EditDefaultsOnly) // StaticMeshComponent의 SetStaticMesh를 위해
		class UMaterialInterface* TeleportArchMaterial;

	UPROPERTY(EditDefaultsOnly) //BP_HandController를 담는다.
		TSubclassOf<AHandController> HandControllerClass;

	UPROPERTY(EditAnywhere)
		float TeleportProjectileRadius = 10;

	UPROPERTY(EditAnywhere)
		float TeleportProjectileSpeed = 800;

	UPROPERTY(EditAnywhere)
		float TeleportSimulationTime = 3;

	UPROPERTY(EditAnywhere)
		float TeleportFadeTime = 1;

	UPROPERTY(EditAnywhere)
		FVector TeleportProjectionExtent = FVector(100, 100, 100);

	bool FindTeleportDestination(TArray<FVector> &OutPath, FVector& OutLocation);
	void UpdateDestinationMarker();
	void UpdateBlinkers();
	void UpdateSpline(const TArray<FVector>& Path);
	void DrawTeleportPath(const TArray<FVector>& Path);

	void MoveForward(float Throttle);
	void MoveRight(float Throttle);
	void BeginTeleport();
	void FinishTeleport();
	void StartFade(float FromAlpha, float ToAlpha);
	FVector2D GetBlinkerCentre();
};

