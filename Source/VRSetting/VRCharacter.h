// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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

private:
	UPROPERTY(VisibleAnywhere)
		class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere) //EditAnywhere�� �����͸� �����Ҽ��� �ִٴ� ���� �ɼ� �־ �̰ŷ���?
		class UStaticMeshComponent* DestinationMarker; // �ڷ���Ʈ�� ������, �ݸ��� �������� visibility�� ���� ���־����

	UPROPERTY() //�𸮾��� �޸� �������ܤ�
		class USceneComponent* VRRoot;

	UPROPERTY(EditAnywhere)
		float MaxTeleportDistance = 1000;

	UPROPERTY(EditAnywhere)
		float TeleportFadeTime = 1;

	UPROPERTY(EditAnywhere)
		FVector TeleportProjectionExtent = FVector(100, 100, 100);

	bool FindTeleportDestination(FVector& OutLocation);
	void UpdateDestinationMarker();
	void MoveForward(float Throttle);
	void MoveRight(float Throttle);
	void BeginTeleport();
	void FinishTeleport();
	void StartFade(float FromAlpha, float ToAlpha);
};
