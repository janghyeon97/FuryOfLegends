#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Structs/CharacterData.h"
#include "ArenaPlayerState.generated.h"

class AItem;
class AArenaGameMode;


USTRUCT()
struct FWidgetState
{
	GENERATED_BODY()

public:
	FWidgetState() : Index(-1), CurrentStack(0), CooldownRatio(0.0f), ConcurrentUses(0) {}

	FWidgetState(int32 InIndex, int32 InCurrentStack, float InCooldownRatio, int32 InConcurrentUses)
		: Index(InIndex)
		, CurrentStack(InCurrentStack)
		, CooldownRatio(InCooldownRatio)
		, ConcurrentUses(InConcurrentUses)
	{
	}

	UPROPERTY()
	int32 Index;

	UPROPERTY()
	int32 CurrentStack;

	UPROPERTY()
	float CooldownRatio;

	UPROPERTY()
	int32 ConcurrentUses;
};

USTRUCT()
struct FRemovedItemData
{
	GENERATED_BODY()

public:
	/*FRemovedItemData() : InventoryIndex(-1), ItemCode(-1), RemovedCount(0), ItemInstance(nullptr) {}
	FRemovedItemData(int32 InInventoryIndex, int32 InItemCode, int32 InRemovedCount, AItem* InItemInstance)
		: InventoryIndex(InInventoryIndex), ItemCode(InItemCode), RemovedCount(InRemovedCount), ItemInstance(InItemInstance)
	{
	}*/

	FRemovedItemData() : ItemCode(-1), RemovedCount(0), ItemInstance(nullptr) {}
	FRemovedItemData(int32 InItemCode, int32 InRemovedCount, AItem* InItemInstance)
		: ItemCode(InItemCode), RemovedCount(InRemovedCount), ItemInstance(InItemInstance)
	{
	}

	UPROPERTY()
	int32 ItemCode;

	UPROPERTY()
	int32 RemovedCount;

	UPROPERTY()
	AItem* ItemInstance;
};

USTRUCT()
struct FInventoryTransactionLog
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<int32, int32> AddedStacks;                // {SlotIndex, AddedCount}

	UPROPERTY()
	TMap<int32, FRemovedItemData> RemovedItems;    // {SlotIndex, RemovedItemData}

	UPROPERTY()
	int32 CurrencyChange = 0;                    

	void Reset()
	{
		AddedStacks.Empty();
		RemovedItems.Empty();
		CurrencyChange = 0;
	}
};



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrencyUpdatedDelegate, const int32, NewCurrency);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemPurchasedDelegate, int32, ItemCode, bool, bSucessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnInventoryUpdatedDelegate, int32, InventoryIndex, int32, ItemCode, int32, CurrentStack);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimerUpdatedDelegate, uint32, UniqueCode, int32, ConcurrentUses);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRemainingTimeChangedDelegate, uint32, UniqueCode, float, RemainingTime, float, ElapsedTime);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWidgetStateChangedDelegate, uint32, UniqueCode, const FWidgetState&, WidgetState);



UCLASS()
class FURYOFLEGENDS_API AArenaPlayerState : public APlayerState
{
	GENERATED_BODY()

	friend class ALobbyGameMode;

public:
	AArenaPlayerState();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void SetOwner(AActor* NewOwner);

public:
	// Accessors
	bool UseUpgradePoints();
	int32 GetUpgradePoints() const { return UpgradePoints; }

	ETeamSide GetTeamSide() const { return TeamSide; }
	void SetTeamSide(ETeamSide NewTeamSide) { TeamSide = NewTeamSide; }

	FString GetPlayerUniqueID() const;
	void SetPlayerUniqueID(const FString& NewPlayerUniqueID) { PlayerUniqueID = NewPlayerUniqueID; }

	int32 GetPlayerIndex() const { return PlayerIndex; }
	void SetPlayerIndex(int32 NewPlayerIndex) { PlayerIndex = NewPlayerIndex; }

	FName GetChosenChampionName() const { return ChosenChampionName; }
	void SetChosenChampionName(const FName& NewChampionName) { ChosenChampionName = NewChampionName; }

	int32 GetCurrency() const { return Currency; }
	void SetCurrency(int32 NewCurrency) { Currency = NewCurrency; }
	void AddCurrency(const int32 Amount);



	/** ------------------------------------------------------ Inventory Management ------------------------------------------------------ */

public:
	void RemoveItemFromInventory(int32 ItemCode);
	void BindItemToPlayer(AItem* Item);
	void SwapItemsInInventory(int32 Index1, int32 Index2);

	/** 최대 소지 한도 확인 */
	bool ExceedsMaxPossession(const AItem* ItemCode) const;

	// 현재 아이템 개수 반환
	int32 GetItemTotalCount(const int32 ItemCode) const;

	AItem* FindOrDuplicateItem(const AItem* OriginalItem);
	void MoveItemToPendingDeletion(int32 ItemCode);
	AItem* FindItemInPendingDeletion(int32 ItemCode);
	void CheckExpiredItems();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void UseItem(int32 Index);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Player")
	void ServerPurchaseItem(const int32 ItemCode);

	UFUNCTION(Client, Reliable)
	void ClientInventoryChanged(const int32 InventortIndex, const int32 ItemCode, const int32 CurrentStack);

	UFUNCTION(Client, Reliable)
	void ClientNotifyItemPurchased(const int32 ItemCode, const bool bSucessful);

private:
	bool ProcessPurchaseTransaction(AItem* ItemToPurchase, FInventoryTransactionLog& TransactionLog);
	void RollbackTransaction(const FInventoryTransactionLog& TransactionLog);
	FInventoryTransactionLog BeginTransaction();
	void CommitTransaction(FInventoryTransactionLog& TransactionLog);


	//int32 AddItemWithSubItems(const AItem* OriginalItem);

	/** ------------------------------------------------------ Timer Management ------------------------------------------------------ */

public:
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

	void ClearTimer(const uint32 UniqueCode);
	bool IsTimerActive(const uint32 UniqueCode) const;
	float GetTimerRemaining(const uint32 UniqueCode) const;
	float GetTimerElapsedTime(const uint32 UniqueCode) const;

	UFUNCTION(Client, Unreliable)
	void ClientNotifyRemainingTime(const uint32 UniqueCode, const float RemainingTime, const float ElapsedTime);

	UFUNCTION(Client, Unreliable)
	void ClientNotifyTimerUpdated(const uint32 UniqueCode, const int32 ConcurrentUses);

private:
	void InternalSetTimer(const uint32 UniqueCode, FTimerUnifiedDelegate&& InDelegate, float InRate, bool bInLoop, float InFirstDelay, bool bBroadcast);

	/** ------------------------------------------------------ Server Functions ------------------------------------------------------ */

public:
	UFUNCTION(Server, Reliable)
	void ServerSetChampionName(const FName& NewChampionName);

	UFUNCTION()
	void OnPlayerLevelChanged(int32 OldLevel, int32 NewLevel);

	UFUNCTION()
	void OnRep_CurrencyUpdated();

	// Delegate
	FOnItemPurchasedDelegate OnItemPurchased;
	FOnInventoryUpdatedDelegate OnInventoryUpdated;
	FOnCurrencyUpdatedDelegate OnCurrencyUpdated;
	FOnTimerUpdatedDelegate OnTimerUpdated;
	FOnRemainingTimeChangedDelegate OnRemainingTimeChanged;

	FOnWidgetStateChangedDelegate OnWidgetStateChanged;

public:
	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	ETeamSide TeamSide;

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FString PlayerUniqueID;

	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 PlayerIndex;

	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FName ChosenChampionName;

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Player", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AArenaGameMode> GameMode;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Player", meta = (AllowPrivateAccess = "true"))
	TArray<AItem*> Inventory;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Player", meta = (AllowPrivateAccess = "true"))
	TArray<AItem*> PendingDeletionItems;
	
	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 UpgradePoints;

	UPROPERTY(ReplicatedUsing = OnRep_CurrencyUpdated, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 Currency;

	// Timer Handles
	TMap<uint32, FTimerHandle> TimerHandles;			// <ItemCode, TimerHandle>
	TMap<uint32, FTimerHandle> BroadcastTimerHandles;	// <ItemCode, TimerHandle>

	FTimerHandle ActivationCheckTimer;
};