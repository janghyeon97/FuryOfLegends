// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/DataTable.h"
#include "AOSGameInstance.generated.h"

class UDataTable;
struct FSharedGameplay;
struct FMinionAttributesRow;
struct FCharacterAttributesRow;
struct FCharacterAnimationAttribute;
struct FCharacterParticleEffectAttribute;
struct FCharacterStaticMeshAttribute;


UCLASS()
class FURYOFLEGENDS_API UAOSGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UAOSGameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;

	const UDataTable* GetChampionListTable() const;
	const FCharacterAttributesRow* GetChampionListTableRow(const FName& RowName) const;

	const UDataTable* GetMinionsListTable() const;
	const FMinionAttributesRow* GetMinionsListTableRow(const FName& RowName) const;

	const UDataTable* GetSharedGamePlayParticlesDataTable();
	FSharedGameplay* GetSharedGamePlayParticles();

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AOSGameInstance", Meta = (AllowPrivateAccess))
	int32 NumberOfPlayer;

	UPROPERTY()
	FString GameInstanceID;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AOSGameInstance", Meta = (AllowPrivateAccess))
	class UDataTable* ChampionList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AOSGameInstance", Meta = (AllowPrivateAccess))
	class UDataTable* MinionsList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AOSGameInstance", Meta = (AllowPrivateAccess))
	class UDataTable* SharedGamePlayParticlesDataTable;

};
