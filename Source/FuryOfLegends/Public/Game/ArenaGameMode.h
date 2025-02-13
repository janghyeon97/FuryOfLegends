
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Structs/MinionData.h"
#include "Structs/GameData.h"
#include "Structs/CharacterData.h"
#include "Item/ItemData.h"
#include "ArenaGameMode.generated.h"

class UDataTable;
class UAOSGameInstance;
class AArenaGameState;
class AAOSPlayerController;
class AArenaPlayerState;
class ACharacterBase;
class AAOSCharacterBase;
class APlayerStart;
class ANexus;
class AItem;

USTRUCT(BlueprintType)
struct FPlayerInformation
{
	GENERATED_BODY()

public:
	// Properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Index;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AAOSCharacterBase> PlayerCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AAOSPlayerController> PlayerController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AArenaPlayerState> PlayerState;

	FPlayerInformation() : Index(-1), PlayerCharacter(nullptr), PlayerController(nullptr) {};
	FPlayerInformation(int32 InIndex, AAOSCharacterBase* InPlayerCharacter, AAOSPlayerController* InPlayerController, AArenaPlayerState* InPlayerState)
		: Index(InIndex)
		, PlayerCharacter(InPlayerCharacter)
		, PlayerController(InPlayerController)
		, PlayerState(InPlayerState)
	{}
};


/**
 * AArenaGameMode 클래스는 플레이어 스포닝 및 게임 시작 로직을 포함한 주요 게임 흐름을 처리합니다.
 */
UCLASS()
class FURYOFLEGENDS_API AArenaGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AArenaGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

private:
	virtual void StartGame();
	virtual void EndGame();
	

public:
	virtual void NotifyNexusDestroyed(ANexus* Nexus);
	virtual void PlayerLoaded(APlayerController* PlayerController);
	virtual void PlayerPawnReady(AAOSCharacterBase* PlayerCharacter);
	
	void ActivateCurrencyIncrement();
	void ActivateSpawnMinion();
	void RequestRespawn(const int32 PlayerIndex);

	void AddCurrencyToPlayer(ACharacterBase* Character, int32 Amount);
	void AddExpToPlayer(ACharacterBase* Character, int32 Amount);

	TArray<FItemTableRow> GetLoadedItems() const;
	int32 GetInitialCharacterLevel() const { return InitialCharacterLevel; };
	const TMap<int32, int32>* GetSubItemsForItem(int32 ItemCode) const;

private:
	void LoadGameData();
	void LoadItemData();
	void LoadMinionData();
	void GenerateSubItem(int32 ItemCode);

	void SpawnMinionsForLane(ELaneType Lane);
	void SpawnMinion(EMinionType MinionType, ELaneType Lane, ETeamSide Team);
	void SpawnCharacter(AAOSPlayerController* PlayerController, const FName& ChampionRowName, ETeamSide Team, const int32 PlayerIndex);
	void RespawnCharacter(const int32 PlayerIndex);

	void CheckAllPlayersLoaded();
	void FindPlayerStart();
	void FindTaggedActors(FName PrimaryTag, TMap<FName, AActor*>& TargetMap);
	void IncrementPlayerCurrency();
	float CalculateRespawnTime(AAOSCharacterBase* Character) const;

private:
	/** ------------------------------------------------------ Timer Management ------------------------------------------------------ */
	template< class UserClass >
	FORCEINLINE void SetTimer(const uint32 UniqueCode, UserClass* InObj, typename FTimerDelegate::TMethodPtr< UserClass > InTimerMethod, float InRate, bool InbLoop = false, float InFirstDelay = -1.f, bool InbBroadcast = false)
	{
		InternalSetTimer(UniqueCode, FTimerUnifiedDelegate(FTimerDelegate::CreateUObject(InObj, InTimerMethod)), InRate, InbLoop, InFirstDelay, InbBroadcast);
	}
	template< class UserClass >
	FORCEINLINE void SetTimer(const uint32 UniqueCode, UserClass* InObj, typename FTimerDelegate::TConstMethodPtr< UserClass > InTimerMethod, float InRate, bool InbLoop = false, float InFirstDelay = -1.f, bool InbBroadcast = false)
	{
		InternalSetTimer(UniqueCode, FTimerUnifiedDelegate(FTimerDelegate::CreateUObject(InObj, InTimerMethod)), InRate, InbLoop, InFirstDelay, InbBroadcast);
	}

	FORCEINLINE void SetTimer(const uint32 UniqueCode, FTimerDelegate const& InDelegate, float InRate, bool InbLoop, float InFirstDelay = -1.f, bool InbBroadcast = false)
	{
		InternalSetTimer(UniqueCode, FTimerUnifiedDelegate(InDelegate), InRate, InbLoop, InFirstDelay, InbBroadcast);
	}

	FORCEINLINE void SetTimer(const uint32 UniqueCode, FTimerDynamicDelegate const& InDynDelegate, float InRate, bool InbLoop, float InFirstDelay = -1.f, bool InbBroadcast = false)
	{
		InternalSetTimer(UniqueCode, FTimerUnifiedDelegate(InDynDelegate), InRate, InbLoop, InFirstDelay, InbBroadcast);
	}

	FORCEINLINE void SetTimer(const uint32 UniqueCode, TFunction<void(void)>&& Callback, float InRate, bool InbLoop, float InFirstDelay = -1.f, bool InbBroadcast = false)
	{
		InternalSetTimer(UniqueCode, FTimerUnifiedDelegate(MoveTemp(Callback)), InRate, InbLoop, InFirstDelay, InbBroadcast);
	}

	void InternalSetTimer(const uint32 UniqueCode, FTimerUnifiedDelegate&& InDelegate, float InRate, bool bInLoop, float InFirstDelay, bool bBroadcast);

	void ClearTimer(const uint32 UniqueCode);
	bool IsTimerActive(const uint32 UniqueCode) const;
	float GetTimerRemaining(const uint32 UniqueCode) const;
	float GetTimerElapsedTime(const uint32 UniqueCode) const;
	void BroadcastRemainingTime(const uint32 UniqueCode, const float RemainingTime, const float ElapsedTime);

private:
	/** ------------------------------------------------------ Obejct Ref ------------------------------------------------------ */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAOSGameInstance> GameInstance;

	/** ------------------------------------------------------ Player Informatins ------------------------------------------------------ */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player", Meta = (AllowPrivateAccess = "true"))
	TMap<int32, FPlayerInformation> Players;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player", Meta = (AllowPrivateAccess = "true"))
	TArray<FPlayerInformation> ExitingPlayers;

public:
	/** ------------------------------------------------------ InGame Data ------------------------------------------------------ */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay", Meta = (AllowPrivateAccess = "true"))
	UDataTable* ItemTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay", Meta = (AllowPrivateAccess = "true"))
	UDataTable* MinionTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay", Meta = (AllowPrivateAccess = "true"))
	UDataTable* GameplayConfigTable;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Gameplay", Meta = (AllowPrivateAccess = "true"))
	TMap<EMinionType, FMinionAttributesRow> MinionsData;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Gameplay", Meta = (AllowPrivateAccess = "true"))
	FGameDataTableRow GameplayConfig;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Gameplay", Meta = (AllowPrivateAccess = "true"))
	TMap<FName, AActor*> MinionPaths;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Gameplay", Meta = (AllowPrivateAccess = "true"))
	TMap<int32, AItem*> LoadedItems;

	/**  ItemCode -> (SubItemCode -> RequiredCount) */
	TMap<int32, TMap<int32, int32>> RequiredSubItems;
	
private:
	float MaxEndWaitTimer = 20.f;
	float MaxLoadWaitTime = 30.f;
	uint8 NumberOfPlayer = 0; // 게임 내 플레이어 수
	uint8 ConnectedPlayer = 0; // 접속한 플레이어 수
	int32 InitialCharacterLevel = 1;

	TMap<FName, AActor*> OrientationPoints;
	TMap<FName, APlayerStart*> PlayerStarts;

	TMap<uint32, FTimerHandle> TimerHandles;				// <PlayerIndex, TimerHandle>
	TMap<uint32, FTimerHandle> BroadcastTimerHandles;	// <PlayerIndex, TimerHandle>

	FTimerHandle AddCurrencyTimerHandle;
	FTimerHandle SpawnMinionTimerHandle;
	FTimerHandle LoadTimerHandle;

	int32 SpawnCount = 0;
	bool bHasNexusDestroyed = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "CrowdControl", meta = (AllowPrivateAccess = "true"))
	class UCrowdControlManager* CrowdControlManager;
};
