// Fill out your copyright notice in the Description page of Project Settings.

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "VRCharacter.h"
#include "NavigationSystem.h" // build.cs ���Ͽ� NavigationSystem �߰��ؾ���
#include "Components/CapsuleComponent.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
AVRCharacter::AVRCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot")); // vr ��ġ ������ ���� ��������Ʈ
	VRRoot->SetupAttachment(GetRootComponent()); 

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera")); // ī�޶� ������Ʈ �߰�
	Camera->SetupAttachment(VRRoot); // vr��Ʈ ������Ʈ�� �ڽ����� ���̱�

	DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMarker"));
	DestinationMarker->SetupAttachment(GetRootComponent());

	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
	PostProcessComponent->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (BlinkerMaterialBase != nullptr)
	{
		// �θ� �������� �ϴ� ���� Material ����
		BlinkerMaterialInstance = UMaterialInstanceDynamic::Create(BlinkerMaterialBase, this);
		// ���Ͱ� ����ִ� PostProcess Component�� ����
		PostProcessComponent->AddOrUpdateBlendable(BlinkerMaterialInstance);

	}
}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// �÷��̾�� �������� ������ �����ϼ� �ִµ� camera�� ��帶��Ʈ ���÷��̸� ���󰡱� ������
	// ĸ�� �ݶ��̴� ��ġ�� ���� �� ī�޶� ��ġ�� �ٽ� ĸ�������� �����;���(VRRoot�� �����صΰ� ���������� �����δ�)
	FVector NewCameraOffset = Camera->GetComponentLocation() - GetActorLocation();
	NewCameraOffset.Z = 0;
	AddActorWorldOffset(NewCameraOffset);
	VRRoot->AddWorldOffset(-NewCameraOffset);

	UpdateDestinationMarker();

	UpdateBlinkers();
}

bool AVRCharacter::FindTeleportDestination(FVector& OutLocation)
{
	FHitResult HitResult;
	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + Camera->GetForwardVector() * MaxTeleportDistance;

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility);
	if (!bHit) return false;

	// Nav Mesh Volume�� ���� �����ִ� ���� �ڷ���Ʈ
	FNavLocation NavLocation;
	bool bOnNavMesh = UNavigationSystemV1::GetCurrent(GetWorld())->
		ProjectPointToNavigation(HitResult.Location, NavLocation, TeleportProjectionExtent);
	if (!bOnNavMesh) return false;

	OutLocation = NavLocation.Location;
	return true;
}

void AVRCharacter::UpdateDestinationMarker()
{
	FVector Location;
	bool bHasDestination = FindTeleportDestination(Location);

	if (bHasDestination)
	{
		DestinationMarker->SetVisibility(true);
		DestinationMarker->SetWorldLocation(Location);
	}
	else
	{
		DestinationMarker->SetVisibility(false);
	}
}

void AVRCharacter::UpdateBlinkers()
{
	if (RadiusVsVelocity == nullptr) return;
	// ���� �ν��Ͻ� ��ȯ
	float Speed = GetVelocity().Size();
	float Radius = RadiusVsVelocity->GetFloatValue(Speed);

	BlinkerMaterialInstance->SetScalarParameterValue(TEXT("Radius"), Radius);
	
	FVector2D Centre = GetBlinkerCentre();
	UE_LOG(LogTemp, Warning, TEXT("%f %f"), Centre.X, Centre.Y);
	BlinkerMaterialInstance->SetVectorParameterValue(TEXT("Centre"), FLinearColor(Centre.X, Centre.Y, 0));
}

FVector2D AVRCharacter::GetBlinkerCentre()
{
	FVector MovementDirection = GetVelocity().GetSafeNormal();
	if (MovementDirection.IsNearlyZero()) // 0�� ������� ó������ ����. ������ 0���� �����ϱ� ����� ����
	{
		return FVector2D(0.5, 0.5);
	}

	FVector WorldStationaryLocation;
	//������ ���Ͽ� Ÿ���� ��ġ�� �˾Ƴ�. (��, ��) guriguri882.tistory.com/45
	if (FVector::DotProduct(Camera->GetForwardVector(), MovementDirection) > 0) 
	{
		WorldStationaryLocation = Camera->GetComponentLocation() + MovementDirection * 1000;
	}
	else
	{
		WorldStationaryLocation = Camera->GetComponentLocation() - MovementDirection * 1000;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC == nullptr) return FVector2D(0.5, 0.5);

	FVector2D ScreenStationaryLocation;
	//3d ��ġ�� ��ũ���� 2d��ġ�� ��ȯ (����)
	PC->ProjectWorldLocationToScreen(WorldStationaryLocation, ScreenStationaryLocation);
	
	int32 SizeX, SizeY;
	PC->GetViewportSize(SizeX, SizeY);
	ScreenStationaryLocation.X /= SizeX;
	ScreenStationaryLocation.Y /= SizeY;

	return ScreenStationaryLocation;
}

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("Forward"), this, &AVRCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("Right"), this, &AVRCharacter::MoveRight);
	PlayerInputComponent->BindAction(TEXT("Teleport"), IE_Pressed,this, &AVRCharacter::BeginTeleport);
}

void AVRCharacter::MoveForward(float Throttle)
{
	AddMovementInput(Throttle * Camera->GetForwardVector());
}

void AVRCharacter::MoveRight(float Throttle)
{
	AddMovementInput(Throttle * Camera->GetRightVector());
}

void AVRCharacter::BeginTeleport()
{
	StartFade(0, 1);
	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(Handle, this, &AVRCharacter::FinishTeleport, TeleportFadeTime, false);
}

void AVRCharacter::FinishTeleport()
{
	//���̰� Ʋ���Ŷ� ���߿� ��ģ����
	SetActorLocation(DestinationMarker->GetComponentLocation() + GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	StartFade(1, 0);
}

void AVRCharacter::StartFade(float FromAlpha, float ToAlpha)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC != nullptr)
	{
		// �ڷ���Ʈ�� ���̵� ����, TeleportFadeTime�� ���� �������� 1���� 0��ŭ ����
		PC->PlayerCameraManager->StartCameraFade(FromAlpha, ToAlpha, TeleportFadeTime, FLinearColor::Black);
	}
}
