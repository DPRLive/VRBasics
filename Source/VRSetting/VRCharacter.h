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

	UPROPERTY(VisibleAnywhere) //EditAnywhere�� �����͸� �����Ҽ��� �ִٴ� ���� �ɼ� �־ �̰ŷ���?
		class UStaticMeshComponent* DestinationMarker; // �ڷ���Ʈ�� ������, �ݸ��� �������� visibility�� ���� ���־����

	UPROPERTY(VisibleAnywhere)
		class USplineComponent* TeleportPath;

	UPROPERTY() //�𸮾��� �޸� �������ܤ�
		class USceneComponent* VRRoot;
	
	UPROPERTY() //�𸮾��� �޸� �������ܤ�
		class UMaterialInstanceDynamic* BlinkerMaterialInstance;

	UPROPERTY(EditAnywhere)
		class UMaterialInterface* BlinkerMaterialBase;

	UPROPERTY() // Blinker�� ���� ����Ʈ���μ��� ������Ʈ
		class UPostProcessComponent* PostProcessComponent;

	UPROPERTY(EditAnywhere) //������ Blink�� ���� Ŀ�� 
		class UCurveFloat* RadiusVsVelocity;

	UPROPERTY()
		TArray<class USplineMeshComponent*> TeleportPathMeshPool;

	UPROPERTY(EditDefaultsOnly) // StaticMeshComponent�� SetStaticMesh�� ����
		class UStaticMesh* TeleportArchMesh;

	UPROPERTY(EditDefaultsOnly) // StaticMeshComponent�� SetStaticMesh�� ����
		class UMaterialInterface* TeleportArchMaterial;

	UPROPERTY(EditDefaultsOnly) //BP_HandController�� ��´�.
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

