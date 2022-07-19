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
#include "MotionControllerComponent.h"
#include "Kismet/GamePlayStatics.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshcomponent.h"

// Sets default values
AVRCharacter::AVRCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot")); // vr ��ġ ������ ���� ��������Ʈ
	VRRoot->SetupAttachment(GetRootComponent()); 

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera")); // ī�޶� ������Ʈ �߰�
	Camera->SetupAttachment(VRRoot); // vr��Ʈ ������Ʈ�� �ڽ����� ���̱�

	LeftController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftController"));
	LeftController->SetupAttachment(VRRoot);
	LeftController->SetTrackingSource(EControllerHand::Left); // �޼տ� ����

	RightController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightController"));
	RightController->SetupAttachment(VRRoot);
	RightController->SetTrackingSource(EControllerHand::Right); // �����տ� ����
	 
	TeleportPath = CreateDefaultSubobject<USplineComponent>(TEXT("TeleportPath"));
	TeleportPath->SetupAttachment(RightController);

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

	if (LeftController != nullptr)
	{
		LeftController->bDisplayDeviceModel = true;// �˾Ƽ� ��Ʈ�ѷ��� ���� ����ƽ �޽� ���̰���
	}
	if (RightController != nullptr)
	{
		RightController->bDisplayDeviceModel = true;// �˾Ƽ� ��Ʈ�ѷ��� ���� ����ƽ �޽� ���̰���
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

bool AVRCharacter::FindTeleportDestination(TArray<FVector>& OutPath, FVector& OutLocation)
{
	FHitResult HitResult;
	FVector Look = RightController->GetForwardVector();
	FVector Start = RightController->GetComponentLocation();

	FPredictProjectilePathParams Params( // �������� ���� �ڷ���Ʈ ��ġ ����
		TeleportProjectileRadius,
		Start,
		Look * TeleportProjectileSpeed,
		TeleportSimulationTime,
		ECollisionChannel::ECC_Visibility,
		this);
	//Params.DrawDebugType = EDrawDebugTrace::ForOneFrame;
	Params.bTraceComplex = true;
	FPredictProjectilePathResult Result;

	bool bHit = UGameplayStatics::PredictProjectilePath(this, Params, Result);
	if (!bHit) return false;

	for (const FPredictProjectilePathPointData& PointData : Result.PathData)
	{
		OutPath.Add(PointData.Location);
	}

	// Nav Mesh Volume�� ���� �����ִ� ���� �ڷ���Ʈ
	FNavLocation NavLocation;
	bool bOnNavMesh = UNavigationSystemV1::GetCurrent(GetWorld())->
		ProjectPointToNavigation(Result.HitResult.Location, NavLocation, TeleportProjectionExtent);
	if (!bOnNavMesh) return false;

	OutLocation = NavLocation.Location;
	return true;
}

void AVRCharacter::UpdateDestinationMarker()
{
	TArray<FVector> Path;
	FVector Location;
	bool bHasDestination = FindTeleportDestination(Path, Location);

	if (bHasDestination)
	{
		DestinationMarker->SetVisibility(true);
		DestinationMarker->SetWorldLocation(Location);
		DrawTeleportPath(Path);
	}
	else
	{
		DestinationMarker->SetVisibility(false);

		TArray<FVector> Empty; // �̵����ϸ� � �Ⱥ��̰���
		DrawTeleportPath(Empty);
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

void AVRCharacter::DrawTeleportPath(const TArray<FVector>& Path)
{
	UpdateSpline(Path);

	for (USplineMeshComponent* SplineMesh : TeleportPathMeshPool)
	{
		SplineMesh->SetVisibility(false);
	}

	int32 SegmentNum = Path.Num() - 1;
	for (int32 i = 0; i < SegmentNum; i++)
	{
		// �ʿ��Ҷ��� �߰��ϱ����� Pool�� ���, �׷��� �������� ���� ��ġ ���游
		if (TeleportPathMeshPool.Num() <= i)
		{
			// ��Ÿ�ӿ� ����
			USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
			// ����� ��ġ ��ȭ ����
			SplineMesh->SetMobility(EComponentMobility::Movable);
			SplineMesh->AttachToComponent(TeleportPath, FAttachmentTransformRules::KeepRelativeTransform);
			SplineMesh->SetStaticMesh(TeleportArchMesh);
			SplineMesh->SetMaterial(0, TeleportArchMaterial);
			SplineMesh->RegisterComponent(); // �̰� �߿�

			TeleportPathMeshPool.Add(SplineMesh);
		}
		USplineMeshComponent* SplineMesh = TeleportPathMeshPool[i];
		SplineMesh->SetVisibility(true);

		FVector StartPos, StartTangent, EndPos, EndTangent;
		TeleportPath->GetLocalLocationAndTangentAtSplinePoint(i, StartPos, StartTangent);
		TeleportPath->GetLocalLocationAndTangentAtSplinePoint(i + 1, EndPos, EndTangent);
		SplineMesh->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);
	}
}

void AVRCharacter::UpdateSpline(const TArray<FVector>& Path)
{
	TeleportPath->ClearSplinePoints(false);
	for (int32 i = 0; i < Path.Num(); i++)
	{
		// ������ ���� ������ ���� ��ǥ�� ������ ���� �������� ���� ��ǥ�� ����
		FVector LocalPosition = TeleportPath->GetComponentTransform().InverseTransformPosition(Path[i]);
		FSplinePoint Point(i, LocalPosition, ESplinePointType::Curve);
		TeleportPath->AddPoint(Point, false); // �ϴ� �� �߰��� ��
	}

	TeleportPath->UpdateSpline(); // �� ��� �߰� �� ������Ʈ
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
	FVector Destination = DestinationMarker->GetComponentLocation();
	Destination += GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * GetActorUpVector();
	SetActorLocation(Destination);
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
