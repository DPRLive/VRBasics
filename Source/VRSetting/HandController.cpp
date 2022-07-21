// Fill out your copyright notice in the Description page of Project Settings.

#include "Haptics/HapticFeedbackEffect_Base.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HandController.h"

// Sets default values
AHandController::AHandController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	UE_LOG(LogTemp, Warning, TEXT("Call Constructer"));
	MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController"));
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

	if (bIsClimbing)
	{
		FVector HandControllerDelta = GetActorLocation() - ClimbingStartLocation;
		GetAttachParentActor()->AddActorWorldOffset(-HandControllerDelta);
	}
}

void AHandController::SetHand(EControllerHand Hand)
{
	if (Hand == EControllerHand::Left)
		UE_LOG(LogTemp, Warning, TEXT("Left Contoller"));

	if (Hand == EControllerHand::Right)
		UE_LOG(LogTemp, Warning, TEXT("Right Contoller"));

	if (MotionController != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Motion Controller init"));
		MotionController->SetTrackingSource(Hand);
		// 알아서 컨트롤러에 맞춰 스태틱 메시 보이게함
		MotionController->bDisplayDeviceModel = true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Motion Controller is null"));
	}
}

void AHandController::PairHandController(AHandController* Controller)
{
	OtherController = Controller;
	OtherController->OtherController = this; //상대한테는 나를 설정
}

void AHandController::ActorBeginOverlap(AActor* OverlappedActor, AActor* OhterActor)
{
	bool bNewCanClimb = CanClimb();
	if (!bCanClimb && bNewCanClimb)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can Climb"));

		// 진동 효과
		if (HapticFeedbackEffect != nullptr)
		{
			APawn* Pawn = Cast<APawn>(GetAttachParentActor());
			if (Pawn != nullptr)
			{
				APlayerController* Controller = Cast<APlayerController>(Pawn->GetController());
				if (Controller != nullptr)
				{
					Controller->PlayHapticEffect(HapticFeedbackEffect, MotionController->GetTrackingSource());
				}
			}
		}
	}
	bCanClimb = bNewCanClimb;
}

void AHandController::ActorEndOverlap(AActor* OverlappedActor, AActor* OhterActor)
{
	bCanClimb = CanClimb();
}

bool AHandController::CanClimb() const
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors);

	for (AActor* OverlappingActor : OverlappingActors)
	{
		if (OverlappingActor->ActorHasTag(TEXT("Climbable")))
		{
			return true;
		}
	}
	return false;
}

void AHandController::Grip()
{
	if (!bCanClimb) return;

	if (!bIsClimbing)
	{
		bIsClimbing = true;
		ClimbingStartLocation = GetActorLocation();
	
		OtherController->bIsClimbing = false;

		ACharacter* Character = Cast<ACharacter>(GetAttachParentActor());
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
		ACharacter* Character = Cast<ACharacter>(GetAttachParentActor());
		if (Character != nullptr)
		{
			Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		}
	}
}
