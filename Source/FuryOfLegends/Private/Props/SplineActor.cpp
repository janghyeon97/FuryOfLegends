// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/SplineActor.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"


ASplineActor::ASplineActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;
	bNetLoadOnClient = true;

	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));

	UpdateInterval = 0.05f;
	SegmentLength = 50.f;
	SplineLength = 0.0f;
	Duration = 0.5;
	ElapsedTime = 0.0f;
	DistanceAlongSpline = 0.0f;
	PreviousDistanceAlongSpline = 0.0f;
	CurrentMeshIndex = 0;
	PreviousMeshIndex = 0;

	bInClosedLoop = false;
	bIsBeginState = false;
	bIsInitialized = false;

	SplineUpdateTimer = FTimerHandle();
}

void ASplineActor::InitializeSpline(const TArray<FVector>& InPath, const float InDuration, const float InSegmentLength)
{
	if (HasAuthority() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Initialization skipped: Function called on client. This function requires server authority."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (InPath.Num() <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Initialization failed: Input path is empty."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (!SplineComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Initialization failed: SplineComponent is null. Ensure it is properly initialized."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	Path = InPath;
	Duration = InDuration;
	SegmentLength = InSegmentLength;

	for (auto& Point : Path)
	{
		SplineComponent->AddSplinePoint(Point, ESplineCoordinateSpace::World, true);
	}

	SplineLength = SplineComponent->GetSplineLength();

	ElapsedTime = 0.0f;
	PreviousDistanceAlongSpline = 0.0f;
	DistanceAlongSpline = 0.0f;

	MulticastSetSpline(Path, Duration, SegmentLength);
	GenerateMeshes();
}

void ASplineActor::MulticastSetSpline_Implementation(const TArray<FVector>& InPath, const float InDuration, const float InSegmentLength)
{
	if (HasAuthority()) return;

	if (InPath.Num() <= 0)
	{
		return;
	}

	Path = InPath;
	Duration = InDuration;
	SegmentLength = InSegmentLength;

	for (auto& Point : Path)
	{
		SplineComponent->AddSplinePoint(Point, ESplineCoordinateSpace::World, true);
	}

	SplineLength = SplineComponent->GetSplineLength();

	ElapsedTime = 0.0f;
	PreviousDistanceAlongSpline = 0.0f;
	DistanceAlongSpline = 0.0f;

	GenerateMeshes();
}

void ASplineActor::GenerateMeshes()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed: UWorld is null."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (::IsValid(StaticMesh) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed: StaticMesh is null. Please assign a valid StaticMesh."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (SplineLength <= 0.f)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid SplineLength: %f. SplineLength must be greater than 0."), ANSI_TO_TCHAR(__FUNCTION__), SplineLength);
		return;
	}

	if (SegmentLength <= 0.f)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid SegmentLength: %f. SegmentLength must be greater than 0. Using default settings."), ANSI_TO_TCHAR(__FUNCTION__), SegmentLength);
		SegmentLength = 50.f;
	}


	// Spline Mesh 생성
	//int32 SegmentCount = FMath::CeilToInt(SplineLength / SegmentLength);

	int32 SegmentCount = FMath::CeilToInt(SplineLength / SegmentLength);
	SegmentCount = FMath::Max(1, SegmentCount);
	SplineMeshes.SetNum(SegmentCount);

	for (int32 i = 0; i < SegmentCount; ++i)
	{
		USplineMeshComponent* SplineMeshComponent = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
		if (::IsValid(SplineMeshComponent) == false)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] Failed to create SplineMeshComponent at index: %d on %s"), ANSI_TO_TCHAR(__FUNCTION__), i,
				HasAuthority() ? TEXT("Server") : (GetNetMode() == NM_Client ? TEXT("Client") : TEXT("Standalone")));
			continue;
		}

		// 1. 복제 가능 설정
		SplineMeshComponent->SetIsReplicated(true);

		// 2. 부모 컴포넌트에 연결
		SplineMeshComponent->SetMobility(EComponentMobility::Movable);
		SplineMeshComponent->AttachToComponent(SplineComponent, FAttachmentTransformRules::KeepRelativeTransform);

		// 3. 컴포넌트 등록
		SplineMeshComponent->RegisterComponent();

		// 4. 속성 설정
		SplineMeshComponent->SetStaticMesh(StaticMesh);
		SplineMeshComponent->SetVisibility(false);
		SplineMeshComponent->SetRelativeScale3D(FVector(1, 1.5, 1));

		// 5. 컴포넌트 추가
		SplineMeshes[i] = SplineMeshComponent;
	}

	bIsInitialized = true;
}

void ASplineActor::Activate_Implementation()	
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed: UWorld is null."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	World->GetTimerManager().SetTimer(SplineUpdateTimer, this, &ThisClass::ExpandSpline, UpdateInterval, true);
	bIsBeginState = true;
}

void ASplineActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}


void ASplineActor::BeginPlay()
{
	Super::BeginPlay();

}

void ASplineActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (SplineMeshes.Num() > 0)
	{
		for (auto& SplineMesh : SplineMeshes)
		{
			if (::IsValid(SplineMesh) == false)
			{
				continue;
			}

			SplineMesh->DestroyComponent();
		}
	}

	SplineMeshes.Empty();
	Path.Empty();
}


void ASplineActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);


	/*
	if (!bIsBeginState || !bIsInitialized)
	{
		return;
	}
	
	if (SplineLength <= 0.f || SegmentLength <= 0.f)
	{
		bIsBeginState = false;
		return;
	}

	// 시간 업데이트
	ElapsedTime += DeltaSeconds;

	if (ElapsedTime >= Duration)
	{
		ElapsedTime = Duration;
		DistanceAlongSpline = SplineLength;
		GetWorld()->GetTimerManager().ClearTimer(SplineUpdateTimer);
	}
	else
	{
		const float Alpha = FMath::Clamp(ElapsedTime / Duration, 0.0f, 1.0f);
		DistanceAlongSpline = FMath::Lerp(0.0f, SplineLength, Alpha);
	}

	// 현재 활성화해야 하는 메시 개수 계산
	int32 ActiveMeshCount = FMath::Clamp(FMath::CeilToInt(DistanceAlongSpline / SegmentLength), 0, SplineMeshes.Num());

	// 활성화된 메시 개수만큼만 업데이트하여 점진적으로 생성
	for (int32 i = 0; i < ActiveMeshCount; ++i)
	{
		if (!SplineMeshes.IsValidIndex(i))
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Invalid Mesh Index: %d. Max: %d"), ANSI_TO_TCHAR(__FUNCTION__), i, SplineMeshes.Num());
			continue;
		}

		USplineMeshComponent* SplineMeshComponent = SplineMeshes[i];
		if (!::IsValid(SplineMeshComponent))
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] SplineMeshComponent is null at index %d."), ANSI_TO_TCHAR(__FUNCTION__), i);
			continue;
		}

		// 시작 및 끝 위치 설정
		FVector StartLocation = SplineComponent->GetLocationAtDistanceAlongSpline(i * SegmentLength, ESplineCoordinateSpace::Local);
		FVector StartTangent = SplineComponent->GetTangentAtDistanceAlongSpline(i * SegmentLength, ESplineCoordinateSpace::Local);
		FVector EndLocation = SplineComponent->GetLocationAtDistanceAlongSpline((i + 1) * SegmentLength, ESplineCoordinateSpace::Local);
		FVector EndTangent = SplineComponent->GetTangentAtDistanceAlongSpline((i + 1) * SegmentLength, ESplineCoordinateSpace::Local);

		// 스플라인 메시 업데이트
		SplineMeshComponent->SetStartAndEnd(StartLocation, StartTangent, EndLocation, EndTangent, true);

		// 메시 활성화 (처음 활성화될 때만)
		if (!SplineMeshComponent->IsVisible())
		{
			SplineMeshComponent->SetCollisionProfileName("BlockAllDynamic");
			SplineMeshComponent->SetVisibility(true);
			SplineMeshComponent->UpdateOverlaps();
		}
	}*/
}



void ASplineActor::ExpandSpline()
{
	if (!bIsInitialized || !bIsBeginState)
	{
		return;
	}

	if (SplineLength <= 0.f || SegmentLength <= 0.f)
	{
		bIsBeginState = false;
		GetWorld()->GetTimerManager().ClearTimer(SplineUpdateTimer);
		return;
	}

	// 시간 업데이트
	ElapsedTime += UpdateInterval;

	if (ElapsedTime >= Duration)
	{
		ElapsedTime = Duration;
		DistanceAlongSpline = SplineLength;
		GetWorld()->GetTimerManager().ClearTimer(SplineUpdateTimer);
	}
	else
	{
		const float Alpha = FMath::Clamp(ElapsedTime / Duration, 0.0f, 1.0f);
		DistanceAlongSpline = FMath::Lerp(0.0f, SplineLength, Alpha);
	}

	// 현재 활성화해야 하는 메시 개수 계산
	int32 ActiveMeshCount = FMath::Clamp(FMath::CeilToInt(DistanceAlongSpline / SegmentLength), 0, SplineMeshes.Num());

	// 활성화된 메시 개수만큼만 업데이트하여 점진적으로 생성
	for (int32 i = 0; i < ActiveMeshCount; ++i)
	{
		if (!SplineMeshes.IsValidIndex(i))
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Invalid Mesh Index: %d. Max: %d"), ANSI_TO_TCHAR(__FUNCTION__), i, SplineMeshes.Num());
			continue;
		}

		USplineMeshComponent* SplineMeshComponent = SplineMeshes[i];
		if (!::IsValid(SplineMeshComponent))
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] SplineMeshComponent is null at index %d."), ANSI_TO_TCHAR(__FUNCTION__), i);
			continue;
		}

		// 시작 및 끝 위치 설정
		FVector StartLocation = SplineComponent->GetLocationAtDistanceAlongSpline(i * SegmentLength, ESplineCoordinateSpace::Local);
		FVector StartTangent = SplineComponent->GetTangentAtDistanceAlongSpline(i * SegmentLength, ESplineCoordinateSpace::Local);
		FVector EndLocation = SplineComponent->GetLocationAtDistanceAlongSpline((i + 1) * SegmentLength, ESplineCoordinateSpace::Local);
		FVector EndTangent = SplineComponent->GetTangentAtDistanceAlongSpline((i + 1) * SegmentLength, ESplineCoordinateSpace::Local);

		// 스플라인 메시 업데이트
		SplineMeshComponent->SetStartAndEnd(StartLocation, StartTangent, EndLocation, EndTangent, true);

		// 메시 활성화 (처음 활성화될 때만)
		if (!SplineMeshComponent->IsVisible())
		{
			SplineMeshComponent->SetCollisionProfileName("BlockAllDynamic");
			SplineMeshComponent->SetVisibility(true);
			SplineMeshComponent->UpdateOverlaps();
		}
	}

	/*ElapsedTime += UpdateInterval;

	if (ElapsedTime >= Duration)
	{
		ElapsedTime = Duration;
		DistanceAlongSpline = SplineLength;
		GetWorld()->GetTimerManager().ClearTimer(SplineUpdateTimer);
	}
	else
	{
		const float Alpha = FMath::Clamp(ElapsedTime / Duration, 0.0f, 1.0f);
		DistanceAlongSpline = FMath::Lerp(0.0f, SplineLength, Alpha);
	}

	CurrentMeshIndex = FMath::FloorToInt(DistanceAlongSpline / SegmentLength);
	if (SplineMeshes.IsValidIndex(CurrentMeshIndex) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Invalid Mesh Index: %d. Valid range is [0, %d)."), ANSI_TO_TCHAR(__FUNCTION__), CurrentMeshIndex, SplineMeshes.Num());
		GetWorld()->GetTimerManager().ClearTimer(SplineUpdateTimer);
		return;
	}

	USplineMeshComponent* SplineMeshComponent = SplineMeshes[CurrentMeshIndex];
	if (::IsValid(SplineMeshComponent) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] SplineMeshComponent is null at index %d."), ANSI_TO_TCHAR(__FUNCTION__), CurrentMeshIndex);
		GetWorld()->GetTimerManager().ClearTimer(SplineUpdateTimer);
		return;
	}

	FVector StartLocation = SplineComponent->GetLocationAtDistanceAlongSpline(PreviousDistanceAlongSpline, ESplineCoordinateSpace::Local);
	FVector StartTangent = SplineComponent->GetTangentAtDistanceAlongSpline(PreviousDistanceAlongSpline, ESplineCoordinateSpace::Local);
	FVector EndLocation = SplineComponent->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::Local);
	FVector EndTangent = SplineComponent->GetTangentAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::Local);

	SplineMeshComponent->SetStartAndEnd(StartLocation, StartTangent, EndLocation, EndTangent, true);
	SplineMeshComponent->SetCollisionProfileName("BlockAllDynamic");
	SplineMeshComponent->SetVisibility(true);
	SplineMeshComponent->UpdateOverlaps();

	if (PreviousMeshIndex != CurrentMeshIndex)
	{
		PreviousDistanceAlongSpline = DistanceAlongSpline;
	}*/
}






