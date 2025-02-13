// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SplineActor.generated.h"

class USplineMeshComponent;
class USplineComponent;
class UStaticMesh;

UCLASS()
class FURYOFLEGENDS_API ASplineActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ASplineActor();

protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void InitializeSpline(const TArray<FVector>& InPath, const float InDuration, const float InSegmentLength);

	UFUNCTION(NetMulticast, Reliable)
	void Activate();

private:
	void GenerateMeshes();
	void ExpandSpline();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetSpline(const TArray<FVector>& InPath, const float InDuration, const float InSegmentLength);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline", meta = (AllowPrivateAccess))
	USplineComponent* SplineComponent;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline", meta = (AllowPrivateAccess))
	TArray<USplineMeshComponent*> SplineMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline", meta = (AllowPrivateAccess))
	UStaticMesh* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline", meta = (AllowPrivateAccess))
	float SegmentLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline", meta = (AllowPrivateAccess))
	bool bInClosedLoop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline", meta = (AllowPrivateAccess))
	bool bIsInitialized;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline", meta = (AllowPrivateAccess))
	bool bIsBeginState;

private:
	FTimerHandle SplineUpdateTimer;
	TArray<FVector> Path;

	float UpdateInterval;
	float Duration;
	float ElapsedTime;
	
	float SplineLength;
	float DistanceAlongSpline;
	float PreviousDistanceAlongSpline;

	int32 PreviousMeshIndex;
	int32 CurrentMeshIndex;
};
