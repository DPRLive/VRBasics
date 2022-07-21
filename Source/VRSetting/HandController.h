// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MotionControllerComponent.h"
#include "HandController.generated.h"

UCLASS()
class VRSETTING_API AHandController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHandController();

	void SetHand(EControllerHand Hand);
	void PairHandController(AHandController* Controller);

	void Grip();
	void Release();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION()
		void ActorBeginOverlap(AActor* OverlappedActor, AActor* OhterActor);
	
	UFUNCTION()
		void ActorEndOverlap(AActor* OverlappedActor, AActor* OhterActor);

	UPROPERTY(VisibleAnywhere)
		UMotionControllerComponent* MotionController;

	//Parameters
	UPROPERTY(EditDefaultsOnly)
		 class UHapticFeedbackEffect_Base* HapticFeedbackEffect;

	bool CanClimb() const;
	bool bCanClimb = false;
	bool bIsClimbing = false;
	FVector ClimbingStartLocation;

	AHandController* OtherController;
};
