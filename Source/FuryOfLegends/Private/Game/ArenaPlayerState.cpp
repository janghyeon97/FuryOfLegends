#include "Game/ArenaPlayerState.h"
#include "Game/AOSGameInstance.h"
#include "Game/ArenaGameMode.h"
#include "Game/ArenaGameState.h"
#include "Game/PlayerStateSave.h"
#include "Controllers/AOSPlayerController.h"
#include "Characters/AOSCharacterBase.h"
#include "Plugins/UniqueCodeGenerator.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Item/ItemData.h"
#include "Item/Item.h"

constexpr int32 MaxInventorySize = 6;

AArenaPlayerState::AArenaPlayerState()
{
	bReplicates = true;

	TeamSide = ETeamSide::None;
	PlayerIndex = -1;
	UpgradePoints = 0;
	Currency = 10000;
	PlayerUniqueID = FString();
	ChosenChampionName = NAME_None;

	Inventory.SetNum(MaxInventorySize);
}

void AArenaPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GameMode = Cast<AArenaGameMode>(GetWorld()->GetAuthGameMode());
		if (::IsValid(GameMode) == false)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Invalid GameMode."), ANSI_TO_TCHAR(__FUNCTION__));
			return;
		}
	}
}

void AArenaPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, TeamSide);
	DOREPLIFETIME(ThisClass, PlayerIndex);
	DOREPLIFETIME(ThisClass, ChosenChampionName);
	DOREPLIFETIME(ThisClass, UpgradePoints);
	DOREPLIFETIME(ThisClass, Currency);
}

// Accessors

FString AArenaPlayerState::GetPlayerUniqueID() const
{
#if WITH_EDITOR
	// PIE 환경 확인
	if (GIsEditor && GetWorld()->IsPlayInEditor())
	{
		// PIE 인스턴스 ID 가져오기
		return FString::Printf(TEXT("PIE_Player_%s"), *GetPlayerName());
	}
#endif

	// 실제 게임 환경에서는 고유 ID 반환
	const FUniqueNetIdRepl UniqueNetId = GetUniqueId();
	if (UniqueNetId.IsValid())
	{
		return UniqueNetId->ToString();
	}

	UAOSGameInstance* GameInstance = Cast<UAOSGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] GetPlayerUniqueId failed: return %s"), ANSI_TO_TCHAR(__FUNCTION__), *GameInstance->GameInstanceID);
		return GameInstance->GameInstanceID;
	}

	return FString();
}


// Inventory Management

/**
 * 플레이어가 아이템을 구매할 때 서버 측에서 처리하는 함수입니다.
 *
 * 이 함수는 플레이어가 아이템을 구매하려 할 때 호출됩니다.
 * 여러 가지 확인을 수행합니다:
 * - 게임 모드의 로드된 아이템 목록에서 해당 아이템이 존재하는지 확인합니다.
 * - 플레이어의 인벤토리에 아이템 객체를 복제합니다.
 * - 적용 가능한 할인 항목을 적용하고, 필요한 하위 아이템을 플레이어 인벤토리에서 제거합니다.
 * - 플레이어가 아이템을 구매할 충분한 자금을 가지고 있는지 확인합니다.
 * - 플레이어의 인벤토리가 가득 차 있지 않은지, 플레이어가 해당 아이템의 최대 소지 한도를 초과하지 않았는지 확인합니다.
 *
 * 모든 확인을 통과하면 아이템이 플레이어의 인벤토리에 추가되고, 플레이어의 자금이 아이템의 최종 가격만큼 감소합니다.
 * 함수는 디버깅 목적으로 구매 시도의 결과를 로그에 기록합니다.
 *
 * @param ItemCode 플레이어가 구매하려는 아이템의 ID입니다.
 */
void AArenaPlayerState::ServerPurchaseItem_Implementation(const int32 ItemCode)
{
	if (HasAuthority() == false) return;

	if (!::IsValid(GameMode))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] failed: Invalid GameMode."), ANSI_TO_TCHAR(__FUNCTION__));
		ClientNotifyItemPurchased(ItemCode, false);
		return;
	}

	AItem** FoundItem = GameMode->LoadedItems.Find(ItemCode);
	if (!(FoundItem && *FoundItem))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] failed: Invalid ItemCode %d."), ANSI_TO_TCHAR(__FUNCTION__), ItemCode);
		ClientNotifyItemPurchased(ItemCode, false);
		return;
	}

	if (ExceedsMaxPossession(*FoundItem) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot purchase item: %d. Maximum possession limit reached."), ItemCode);
		ClientNotifyItemPurchased(ItemCode, false);
		return;
	}

	// === 트랜잭션 처리 시작 ===
	FInventoryTransactionLog TransactionLog = BeginTransaction();

	bool bSuccess = ProcessPurchaseTransaction(*FoundItem, TransactionLog);
	if (bSuccess)
	{
		CommitTransaction(TransactionLog);
		ClientNotifyItemPurchased(ItemCode, true);
	}
	else
	{
		RollbackTransaction(TransactionLog);
		ClientNotifyItemPurchased(ItemCode, false);
	}
}

bool AArenaPlayerState::ProcessPurchaseTransaction(AItem* ItemToPurchase, FInventoryTransactionLog& TransactionLog)
{
	if (!::IsValid(ItemToPurchase))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Item."));
		return false;
	}

	// 하위 아이템 제거 및 가격 조정
	const TMap<int32, int32>* SubItems = GameMode->GetSubItemsForItem(ItemToPurchase->ItemCode);
	if (!SubItems)
	{
		UE_LOG(LogTemp, Warning, TEXT("No sub-items found for ItemCode: %d"), ItemToPurchase->ItemCode);
		return false;
	}

	int32 FinalPrice = ItemToPurchase->Price;
	TMap<int32, int32> ItemsToRemove = *SubItems;
	int32 EmptySlot = -1;

	for (int32 i = 0; i < Inventory.Num(); ++i)
	{
		AItem* CurrentItem = Inventory[i];
		if (!CurrentItem)
		{
			if (EmptySlot == -1) EmptySlot = i;
			continue;
		}

		int32 ItemCode = CurrentItem->ItemCode;

		// 하위 아이템 제거
		if (int32* RequiredCount = ItemsToRemove.Find(ItemCode))
		{
			int32 AvailableCount = CurrentItem->CurrentStackPerSlot;
			int32 RemovableCount = FMath::Min(*RequiredCount, AvailableCount);

			FinalPrice = FMath::Max(0, FinalPrice - (RemovableCount * CurrentItem->Price));

			// 트랜잭션 로그 기록
			TransactionLog.RemovedItems.Add(i, FRemovedItemData(ItemCode, RemovableCount, CurrentItem));

			CurrentItem->CurrentStackPerSlot -= RemovableCount;
			CurrentItem->RemoveAbilitiesFromCharacter();
			if (CurrentItem->CurrentStackPerSlot <= 0)
			{
				ClientInventoryChanged(i, -1, 0);
				Inventory[i] = nullptr;
				if (EmptySlot == -1) EmptySlot = i;
			}
			else
			{
				ClientInventoryChanged(i, ItemCode, CurrentItem->CurrentStackPerSlot);
			}

			*RequiredCount -= RemovableCount;
			if (*RequiredCount <= 0)
			{
				ItemsToRemove.Remove(ItemCode);
			}
		}

		// 스택 가능한 슬롯 찾기
		if (CurrentItem->ItemCode == ItemToPurchase->ItemCode && CurrentItem->CurrentStackPerSlot < CurrentItem->MaxStackPerSlot)
		{
			int32 SpaceLeft = CurrentItem->MaxStackPerSlot - CurrentItem->CurrentStackPerSlot;
			int32 AddCount = FMath::Min(1, SpaceLeft); // 상위 아이템은 1개씩만 추가 가능

			CurrentItem->CurrentStackPerSlot += AddCount;
			CurrentItem->ApplyAbilitiesToCharacter();

			// 트랜잭션 로그에 추가
			TransactionLog.AddedStacks.Add(i, AddCount);
			ClientInventoryChanged(i, ItemToPurchase->ItemCode, CurrentItem->CurrentStackPerSlot);

			if (AddCount > 0)
			{
				return true;
			}
		}
	}

	// 2인벤토리 공간 및 자금 확인
	if (Currency < FinalPrice)
	{
		UE_LOG(LogTemp, Warning, TEXT("Insufficient funds for ItemCode: %d"), ItemToPurchase->ItemCode);
		return false;
	}

	// 빈 슬롯에 상위 아이템 추가
	if (EmptySlot != -1)
	{
		AItem* NewItem = FindOrDuplicateItem(ItemToPurchase);
		if (!NewItem)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to duplicate item for ItemCode: %d"), ItemToPurchase->ItemCode);
			return false;
		}

		NewItem->CurrentStackPerSlot = 1;
		Inventory[EmptySlot] = NewItem;
		BindItemToPlayer(NewItem);

		TransactionLog.CurrencyChange = -FinalPrice;
		Currency -= FinalPrice;

		ClientInventoryChanged(EmptySlot, NewItem->ItemCode, NewItem->CurrentStackPerSlot);

		return true;
	}

	// 4인벤토리 공간 부족
	UE_LOG(LogTemp, Warning, TEXT("Inventory full. Cannot add ItemCode: %d"), ItemToPurchase->ItemCode);
	return false;
}



void AArenaPlayerState::RollbackTransaction(const FInventoryTransactionLog& TransactionLog)
{
	UE_LOG(LogTemp, Warning, TEXT("Transaction failed. Rolling back changes."));

	// 금액 복원
	Currency -= TransactionLog.CurrencyChange;

	// 스택 롤백
	for (const auto& Entry : TransactionLog.AddedStacks)
	{
		int32 Index = Entry.Key;
		int32 RevertCount = Entry.Value;

		if (Inventory.IsValidIndex(Index) && Inventory[Index])
		{
			Inventory[Index]->CurrentStackPerSlot -= RevertCount;
			Inventory[Index]->ApplyAbilitiesToCharacter();
			ClientInventoryChanged(Index, Inventory[Index]->ItemCode, Inventory[Index]->CurrentStackPerSlot);
		}
	}

	// 제거된 아이템 복원
	for (const auto& Entry : TransactionLog.RemovedItems)
	{
		int32 Index = Entry.Key;
		const FRemovedItemData& Data = Entry.Value;

		if (Inventory.IsValidIndex(Index))
		{
			if (!Inventory[Index]) Inventory[Index] = Data.ItemInstance;
			Inventory[Index]->CurrentStackPerSlot += Data.RemovedCount;
			BindItemToPlayer(Data.ItemInstance);
			ClientInventoryChanged(Index, Data.ItemCode, Inventory[Index]->CurrentStackPerSlot);
		}
	}
}

FInventoryTransactionLog AArenaPlayerState::BeginTransaction()
{
	FInventoryTransactionLog TransactionLog;
	UE_LOG(LogTemp, Log, TEXT("Transaction started."));
	return TransactionLog;
}



void AArenaPlayerState::CommitTransaction(FInventoryTransactionLog& TransactionLog)
{
	UE_LOG(LogTemp, Log, TEXT("Transaction committed successfully."));
	TransactionLog.Reset();
}




/**
 * @brief 구매하려는 아이템과 해당 아이템에 필요한 하위 아이템을 처리하여 인벤토리에 추가합니다.
 *
 * 이 함수는 다음 작업을 수행합니다:
 * 1. 구매하려는 아이템의 정보를 `GameMode`에서 검색합니다.
 * 2. 하위 아이템 목록을 가져와 인벤토리에서 필요한 수량만큼 제거합니다.
 * 3. 하위 아이템의 가격을 최종 가격에서 차감합니다.
 * 4. 남은 자금이 충분한지 확인합니다.
 * 5. 구매가 가능한 경우 아이템을 인벤토리에 추가합니다.
 *
 * @param PurchaseItemCode 구매하려는 아이템의 ID입니다.
 * @return 아이템이 성공적으로 추가되었으면 해당 인벤토리 슬롯 인덱스를 반환하고,
 *         실패하면 -1을 반환합니다.
 */
 //int32 AArenaPlayerState::AddItemWithSubItems(const AItem* OriginalItem)
 //{
 //	const TMap<int32, int32>* SubItems = GameMode->GetSubItemsForItem(OriginalItem->ItemCode);
 //	if (!SubItems)
 //	{
 //		UE_LOG(LogTemp, Warning, TEXT("[%s] failed: No cached sub-item data for ItemCode %d."), ANSI_TO_TCHAR(__FUNCTION__), OriginalItem->ItemCode);
 //		return -1;
 //	}
 //
 //	TMap<int32, int32> ItemsToRemove = *SubItems;          // 필요한 하위 아이템 목록
 //	TMap<int32, FRemovedItemData> RemovalLog;                   // 실제 제거를 위한 정보
 //	int32 FinalPrice = OriginalItem->Price;                // 초기 가격 설정
 //
 //	// 1. 검증 단계: 하위 아이템 제거 시뮬레이션 및 공간 확인
 //	for (int32 i = 0; i < Inventory.Num(); ++i)
 //	{
 //		if (!Inventory[i]) continue;
 //
 //		int32 ItemCode = Inventory[i]->ItemCode;
 //		if (!ItemsToRemove.Contains(ItemCode)) continue;
 //
 //		int32& RequiredCount = ItemsToRemove[ItemCode];
 //		int32 AvailableCount = Inventory[i]->CurrentStackPerSlot;
 //		int32 RemovableCount = FMath::Min(RequiredCount, AvailableCount);
 //
 //		// 가격 시뮬레이션
 //		FinalPrice = FMath::Max(0, FinalPrice - (RemovableCount * Inventory[i]->Price));
 //
 //		// 제거할 정보만 기록 (실제 수량 감소 X)
 //		RemovalLog.Add(i, FRemovedItemData(ItemCode, RemovableCount, Inventory[i]));
 //
 //		RequiredCount -= RemovableCount;
 //		if (RequiredCount <= 0)
 //		{
 //			ItemsToRemove.Remove(ItemCode);
 //		}
 //	}
 //
 //	// 상위 아이템을 추가할 수 있는지 확인
 //	if (Currency < FinalPrice)
 //	{
 //		UE_LOG(LogTemp, Warning, TEXT("[%s] failed: Not enough space or insufficient funds."), ANSI_TO_TCHAR(__FUNCTION__));
 //		return -1;
 //	}
 //
 //	// 3️. 최종 아이템 추가 시도
 //	Currency -= FinalPrice;
 //	AItem* DuplicatedItem = FindOrDuplicateItem(OriginalItem);
 //	DuplicatedItem->CurrentStackPerSlot = 1;
 //
 //	int32 AddResult = AddItemToInventory(DuplicatedItem, RemovalLog);
 //
 //	// 4️. 실패 시 복구
 //	if (AddResult == -1)
 //	{
 //		UE_LOG(LogTemp, Warning, TEXT("[%s] failed: Final item addition failed."), ANSI_TO_TCHAR(__FUNCTION__));
 //		Currency += FinalPrice;
 //		return -1;
 //	}
 //
 //	return AddResult;
 //}


 /*void AArenaPlayerState::RollbackInventory(const TArray<FRemovedItemData>& RemovalLog)
 {
	 for (const auto& RemovedData : RemovalLog)
	 {
		 int32 InventoryIndex = RemovedData.InventoryIndex;

		 if (Inventory.IsValidIndex(InventoryIndex))
		 {
			 if (!Inventory[InventoryIndex])
			 {
				 // 슬롯이 비어있으면 아이템 복원
				 Inventory[InventoryIndex] = RemovedData.ItemInstance;
				 Inventory[InventoryIndex]->CurrentStackPerSlot = RemovedData.RemovedCount;
			 }
			 else
			 {
				 // 기존 슬롯에 수량 복원
				 Inventory[InventoryIndex]->CurrentStackPerSlot += RemovedData.RemovedCount;
			 }

			 // 클라이언트와 동기화
			 Inventory[InventoryIndex]->ApplyAbilitiesToCharacter();
			 ClientInventoryChanged(InventoryIndex, RemovedData.ItemCode, Inventory[InventoryIndex]->CurrentStackPerSlot);
		 }
	 }
 }


 void AArenaPlayerState::CommitItemRemovals(const TArray<FRemovedItemData>& RemovalLog)
 {
	 for (const FRemovedItemData& RemovedData : RemovalLog)
	 {
		 int32 InventoryIndex = RemovedData.InventoryIndex;
		 int32 RemoveCount = RemovedData.RemovedCount;

		 // 인벤토리 유효성 검증
		 if (!Inventory.IsValidIndex(InventoryIndex) || !Inventory[InventoryIndex])
		 {
			 UE_LOG(LogTemp, Warning, TEXT("Invalid inventory index: %d"), InventoryIndex);
			 continue;
		 }

		 AItem* CurrentItem = Inventory[InventoryIndex];

		 if (CurrentItem != RemovedData.ItemInstance)
		 {
			 // 슬롯에 덮어씌워진 경우: 상위 아이템 유지, 하위 아이템은 따로 제거
			 if (::IsValid(RemovedData.ItemInstance) == false)
			 {
				 continue;
			 }

			 RemovedData.ItemInstance->RemoveAbilitiesFromCharacter();
			 RemovedData.ItemInstance->ConditionalBeginDestroy();

			 UE_LOG(LogTemp, Log, TEXT("Removed overwritten item: %s (ItemCode: %d) from inventory (originally from slot %d)"),
				 *RemovedData.ItemInstance->Name, RemovedData.ItemCode, InventoryIndex);

			 continue;
		 }

		 // 스택 수량 감소
		 CurrentItem->CurrentStackPerSlot -= RemoveCount;

		 //스택이 0 이하일 경우 아이템 제거
		 if (CurrentItem->CurrentStackPerSlot <= 0)
		 {
			 RemoveItemFromInventory(InventoryIndex);
		 }
		 // 능력 재적용.
		 else
		 {
			 CurrentItem->ApplyAbilitiesToCharacter();
		 }

		 // 클라이언트와 인벤토리 동기화
		 ClientInventoryChanged(
			 InventoryIndex,
			 Inventory[InventoryIndex] ? Inventory[InventoryIndex]->ItemCode : -1,
			 Inventory[InventoryIndex] ? Inventory[InventoryIndex]->CurrentStackPerSlot : 0
		 );
	 }
 }*/


void AArenaPlayerState::RemoveItemFromInventory(int32 Index)
{
	if (Inventory.IsValidIndex(Index) && Inventory[Index])
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] Removing item: %s from inventory slot %d"), ANSI_TO_TCHAR(__FUNCTION__), *Inventory[Index]->Name, Index);


		if (Inventory[Index]->ActivationState == EItemActivationState::Active || Inventory[Index]->ActivationState == EItemActivationState::Pending)
		{
			MoveItemToPendingDeletion(Index);
			ClientInventoryChanged(Index, -1, 0);
		}
		else
		{
			Inventory[Index]->RemoveAbilitiesFromCharacter();
			Inventory[Index]->Destroy();
			Inventory[Index] = nullptr;
			ClientInventoryChanged(Index, -1, 0);
		}
	}
}

void AArenaPlayerState::MoveItemToPendingDeletion(int32 Index)
{
	if (Inventory.IsValidIndex(Index) && Inventory[Index])
	{
		UE_LOG(LogTemp, Log, TEXT("Moving item to PendingDeletion Index:%d ItemCode"), Index, Inventory[Index]->ItemCode);

		PendingDeletionItems.Add(Inventory[Index]);
		Inventory[Index]->CurrentStackPerSlot = 0;
		Inventory[Index] = nullptr;
		ClientInventoryChanged(Index, -1, 0);

		GetWorld()->GetTimerManager().SetTimer(ActivationCheckTimer, this, &AArenaPlayerState::CheckExpiredItems, 0.1f, true);
	}
}

void AArenaPlayerState::CheckExpiredItems()
{
	TArray<int32> ItemsToRemove;  // 삭제할 인덱스를 임시 저장

	for (int32 i = 0; i < PendingDeletionItems.Num(); ++i)
	{
		AItem* Item = PendingDeletionItems[i];

		if (!IsValid(Item)) // 객체가 이미 삭제되었거나 유효하지 않은 경우
		{
			ItemsToRemove.Add(i);
			continue;
		}

		if (Item->ActivationState == EItemActivationState::Expired)
		{
			UE_LOG(LogTemp, Log, TEXT("[%s] Marking expired item for removal: %s (ItemCode: %d)"),
				ANSI_TO_TCHAR(__FUNCTION__), *Item->Name, Item->ItemCode);

			Item->Destroy();
			ItemsToRemove.Add(i);
		}
	}

	// 삭제 대상 제거 (역순으로 안전하게 삭제)
	for (int32 i = ItemsToRemove.Num() - 1; i >= 0; --i)
	{
		PendingDeletionItems.RemoveAt(ItemsToRemove[i]);
	}

	// 타이머 정리
	if (PendingDeletionItems.Num() == 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(ActivationCheckTimer);
	}
}


void AArenaPlayerState::SwapItemsInInventory(int32 Index1, int32 Index2)
{
	if (Inventory.IsValidIndex(Index1) && Inventory.IsValidIndex(Index2))
	{
		Inventory.Swap(Index1, Index2);
		ClientInventoryChanged(Index1, Inventory[Index1]->ItemCode, Inventory[Index1]->CurrentStackPerSlot);
		ClientInventoryChanged(Index2, Inventory[Index2]->ItemCode, Inventory[Index2]->CurrentStackPerSlot);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Invalid inventory indices"));
	}
}


void AArenaPlayerState::UseItem(int32 Index)
{
	if (!Inventory.IsValidIndex(Index) || !Inventory[Index])
	{
		UE_LOG(LogTemp, Warning, TEXT("[AArenaPlayerState::UseItem] Invalid item index or item not found"));
		return;
	}

	Inventory[Index]->Use(this);

	if (Inventory[Index]->CurrentStackPerSlot <= 0)
	{
		ClientInventoryChanged(Index, -1, 0);
		RemoveItemFromInventory(Index);
	}
	else
	{
		ClientInventoryChanged(Index, Inventory[Index]->ItemCode, Inventory[Index]->CurrentStackPerSlot);
	}
}


/**
 * @brief 주어진 ItemCode에 해당하는 아이템의 총 개수를 반환합니다.
 *
 * @param ItemCode 확인할 아이템의 ID.
 * @return 아이템의 총 개수.
 */
int32 AArenaPlayerState::GetItemTotalCount(const int32 ItemCode) const
{
	int32 ItemTotalCount = 0;
	for (const auto& InventoryItem : Inventory)
	{
		if (InventoryItem && InventoryItem->ItemCode == ItemCode)
		{
			ItemTotalCount += InventoryItem->CurrentStackPerSlot;
		}
	}
	return ItemTotalCount;
}


// 최대 소지 한도 확인
bool AArenaPlayerState::ExceedsMaxPossession(const AItem* OriginalItem) const
{
	int32 TotalItemCount = 0;
	for (const auto& InventoryItem : Inventory)
	{
		if (::IsValid(InventoryItem) == false)
		{
			continue;
		}

		if (InventoryItem->ItemCode == OriginalItem->ItemCode)
		{
			TotalItemCount += InventoryItem->CurrentStackPerSlot;
		}
	}

	return TotalItemCount < OriginalItem->MaxInventoryQuantity;
}


AItem* AArenaPlayerState::FindItemInPendingDeletion(int32 ItemCode)
{
	for (int32 i = 0; i < PendingDeletionItems.Num(); ++i)
	{
		if (PendingDeletionItems[i] && PendingDeletionItems[i]->ItemCode == ItemCode)
		{
			return PendingDeletionItems[i];
		}
	}
	return nullptr;
}

/**
 * @brief PendingDeletionItems에서 아이템을 찾거나, 없을 경우 새로 복제하여 반환합니다.
 *
 * @param OriginalItem 찾거나 생성할 아이템의 원본.
 * @return AItem* 찾거나 생성된 아이템을 반환.
 */
AItem* AArenaPlayerState::FindOrDuplicateItem(const AItem* OriginalItem)
{
	if (::IsValid(OriginalItem) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("OriginalItem is invalid."));
		return nullptr;
	}

	// ✅ 안전한 Iterator 기반 순회 및 제거
	for (auto It = PendingDeletionItems.CreateIterator(); It; ++It)
	{
		AItem* PendingItem = *It;
		if (::IsValid(PendingItem) && PendingItem->ItemCode == OriginalItem->ItemCode)
		{
			UE_LOG(LogTemp, Log, TEXT("Item found in PendingDeletionItems: %d"), OriginalItem->ItemCode);
			It.RemoveCurrent();
			return PendingItem;
		}
	}

	// 새로운 아이템 복제
	AItem* NewItem = DuplicateObject<AItem>(OriginalItem, this);
	if (NewItem)
	{
		UE_LOG(LogTemp, Log, TEXT("New item duplicated for ItemCode: %d"), OriginalItem->ItemCode);
		return NewItem;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to duplicate item for ItemCode: %d"), OriginalItem->ItemCode);
		return nullptr;
	}
}




// Server Functions

bool AArenaPlayerState::ServerPurchaseItem_Validate(int32 ItemCode)
{
	return true;
}





// Helper Functions ---------------------------------------------------------------------

/**
 * 주어진 아이템을 추가할 때 필요한 하위 아이템이 인벤토리에 있는지 확인하고,
 * 모든 하위 아이템이 존재하면 이를 제거한 후 아이템을 추가할 수 있는지 여부를 반환합니다.
 *
 * @param Item 추가하려는 대상 아이템.
 * @return 필요한 모든 하위 아이템이 인벤토리에 존재하고 제거되면 true, 그렇지 않으면 false.
 */


void AArenaPlayerState::BindItemToPlayer(AItem* Item)
{
	AActor* OwnerActor = GetOwner();
	if (::IsValid(OwnerActor) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] PlayerState's owner is not set"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	// PlayerController로 캐스팅
	APlayerController* PlayerController = Cast<APlayerController>(OwnerActor);
	if (::IsValid(PlayerController) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Owner is not a PlayerController"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	// PlayerController의 Pawn을 가져와서 AAOSCharacterBase로 캐스팅
	AAOSCharacterBase* OwningCharacter = Cast<AAOSCharacterBase>(PlayerController->GetPawn());
	if (::IsValid(OwningCharacter) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to bind item %s: Controlled character is not valid or is not AAOSCharacterBase"), ANSI_TO_TCHAR(__FUNCTION__), *Item->GetName());
		return;
	}

	// 아이템을 캐릭터에 바인딩
	UE_LOG(LogTemp, Log, TEXT("[%s] Binding item %s to character %s"), ANSI_TO_TCHAR(__FUNCTION__), *Item->Name, *OwningCharacter->GetName());
	Item->BindToPlayer(OwningCharacter);
}


// Events

void AArenaPlayerState::ServerSetChampionName_Implementation(const FName& NewChampionName)
{
	if (!NewChampionName.IsNone())
	{
		ChosenChampionName = NewChampionName;
	}
}


void AArenaPlayerState::OnPlayerLevelChanged(int32 OldLevel, int32 NewLevel)
{
	if (HasAuthority() == false)
	{
		return;
	}

	UpgradePoints = UpgradePoints + 1;
}


void AArenaPlayerState::AddCurrency(const int32 Amount)
{
	if (HasAuthority() == false)
	{
		return;
	}

	Currency = Currency + Amount;
}

void AArenaPlayerState::OnRep_CurrencyUpdated()
{
	if (OnCurrencyUpdated.IsBound())
	{
		OnCurrencyUpdated.Broadcast(Currency);
	}
}


void AArenaPlayerState::ClientInventoryChanged_Implementation(const int32 InventortIndex, const int32 ItemCode, const int32 CurrentStack)
{
	if (OnInventoryUpdated.IsBound())
	{
		OnInventoryUpdated.Broadcast(InventortIndex, ItemCode, CurrentStack);
	}
}

void AArenaPlayerState::ClientNotifyItemPurchased_Implementation(const int32 ItemCode, const bool bSucessful)
{
	if (OnItemPurchased.IsBound())
	{
		OnItemPurchased.Broadcast(ItemCode, bSucessful);
	}
}


void AArenaPlayerState::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);


}

bool AArenaPlayerState::UseUpgradePoints()
{
	if (UpgradePoints <= 0)
	{
		return false;
	}

	UpgradePoints = UpgradePoints - 1;
	return true;
}


/** --------------------------------------------------------------------------------
 * 아이템 타이머 관리 함수들
 *
 * 이 함수들은 플레이어 상태에서 아이템의 타이머를 관리하는 데 사용됩니다.
 * 타이머가 활성화되어 있는지 확인하고, 타이머를 설정하거나 제거하며,
 * 타이머의 남은 시간을 주기적으로 브로드캐스트하는 기능을 제공합니다.
 */


 /**
   * 아이템 타이머가 활성화되어 있는지 확인합니다.
   *
   * 주어진 아이템 ID에 해당하는 타이머가 활성화되어 있는지 확인합니다.
   *
   * @param ItemCode 확인할 아이템의 ID.
   * @return 타이머가 활성화되어 있으면 true, 그렇지 않으면 false.
   */
bool AArenaPlayerState::IsTimerActive(const uint32 UniqueCode) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] GetWorld() returned null. UniqueCode: %u"), ANSI_TO_TCHAR(__FUNCTION__), UniqueCode);
		return false;
	}

	const FTimerHandle* TimerHandle = TimerHandles.Find(UniqueCode);
	return TimerHandle && World->GetTimerManager().IsTimerActive(*TimerHandle);
}



/**
 * 아이템 타이머를 설정합니다.
 *
 * 주어진 아이템 ID에 대해 타이머를 설정하고, 타이머가 만료되었을 때 실행할 콜백을 지정합니다.
 * 또한, 주기적으로 남은 시간을 브로드캐스트하는 보조 타이머를 설정합니다.
 *
 * @param ItemCode 타이머를 설정할 아이템의 ID.
 * @param Duration 타이머의 지속 시간.
 * @param Callback 타이머가 만료되었을 때 실행할 콜백 함수.
 */
void AArenaPlayerState::InternalSetTimer(const uint32 UniqueCode, FTimerUnifiedDelegate&& InDelegate, float InRate, bool bInLoop, float InFirstDelay, bool bBroadcast)
{
	if (!InDelegate.IsBound())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] TimerDelegate is not bound for UniqueCode: %u"), ANSI_TO_TCHAR(__FUNCTION__), UniqueCode);
		return;
	}

	if (InRate <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Invalid timer rate (%f) for UniqueCode:: %u"), ANSI_TO_TCHAR(__FUNCTION__), InRate, UniqueCode);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] GetWorld() returned null. UniqueCode: %u"), ANSI_TO_TCHAR(__FUNCTION__), UniqueCode);
		return;
	}

	FTimerHandle& TimerHandle = TimerHandles.FindOrAdd(UniqueCode);
	if (InDelegate.FuncCallback)
	{
		World->GetTimerManager().SetTimer(TimerHandle, MoveTemp(InDelegate.FuncCallback), InRate, bInLoop, InFirstDelay);
	}
	else if (InDelegate.FuncDelegate.IsBound())
	{
		World->GetTimerManager().SetTimer(TimerHandle, InDelegate.FuncDelegate, InRate, bInLoop, InFirstDelay);
	}
	else if (InDelegate.FuncDynDelegate.IsBound())
	{
		World->GetTimerManager().SetTimer(TimerHandle, InDelegate.FuncDynDelegate, InRate, bInLoop, InFirstDelay);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] No valid delegate found for UniqueCode: %u"), ANSI_TO_TCHAR(__FUNCTION__), UniqueCode);
		return;
	}

	if (bBroadcast)
	{
		FTimerHandle& BroadcastTimerHandle = BroadcastTimerHandles.FindOrAdd(UniqueCode);
		World->GetTimerManager().SetTimer(BroadcastTimerHandle, [this, UniqueCode]()
			{
				float RemainingTime = GetTimerRemaining(UniqueCode);
				float ElapsedTime = GetTimerElapsedTime(UniqueCode);
				ClientNotifyRemainingTime(UniqueCode, RemainingTime, ElapsedTime);
			}, 0.05f, true, 0.0f);
	}

	UE_LOG(LogTemp, Log, TEXT("[%s] Timer set successfully. UniqueCode: %u, Rate: %f, Loop: %s, FirstDelay: %f"), ANSI_TO_TCHAR(__FUNCTION__), UniqueCode, InRate, bInLoop ? TEXT("true") : TEXT("false"), InFirstDelay);
}


/**
 * 타이머를 제거합니다.
 *
 * 주어진 ID에 대해 활성화된 타이머와 주기적으로 남은 시간을 브로드캐스트하는 타이머를 제거합니다.
 *
 * @param UniqueCode 제거할 타이머의 이름.
 */
void AArenaPlayerState::ClearTimer(const uint32 UniqueCode)
{
	FTimerHandle* TimerHandle = TimerHandles.Find(UniqueCode);
	if (TimerHandle)
	{
		GetWorld()->GetTimerManager().ClearTimer(*TimerHandle);
		TimerHandles.Remove(UniqueCode);
	}

	// 브로드캐스트 타이머도 제거
	FTimerHandle* BroadcastTimerHandle = BroadcastTimerHandles.Find(UniqueCode);
	if (BroadcastTimerHandle)
	{
		GetWorld()->GetTimerManager().ClearTimer(*BroadcastTimerHandle);
		BroadcastTimerHandles.Remove(UniqueCode);
	}
}


/**
 * 타이머의 남은 시간을 반환합니다.
 *
 * 주어진 ID 에 대해 타이머의 남은 시간을 반환합니다.
 *
 * @param UniqueCode 남은 시간을 확인할 타이머의 이름.
 * @return 타이머의 남은 시간(초) 또는 타이머가 없으면 0.
 */
float AArenaPlayerState::GetTimerRemaining(const uint32 UniqueCode) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] GetWorld() returned null. UniqueCode: %u"), ANSI_TO_TCHAR(__FUNCTION__), UniqueCode);
		return 0.f;
	}

	const FTimerHandle* TimerHandle = TimerHandles.Find(UniqueCode);
	return TimerHandle ? World->GetTimerManager().GetTimerRemaining(*TimerHandle) : 0.f;
}


/**
 * 타이머의 경과 시간을 반환합니다.
 *
 * 주어진 UniqueCode 에 대해 타이머의 경과 시간을 반환합니다.
 *
 * @param UniqueCode 남은 시간을 확인할 타이머의 이름.
 * @return 타이머의 경과 시간(초) 또는 타이머가 없으면 0.
 */
float AArenaPlayerState::GetTimerElapsedTime(const uint32 UniqueCode) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] GetWorld() returned null. UniqueCode: %u"), ANSI_TO_TCHAR(__FUNCTION__), UniqueCode);
		return 0.f;
	}

	const FTimerHandle* TimerHandle = TimerHandles.Find(UniqueCode);
	return TimerHandle ? World->GetTimerManager().GetTimerElapsed(*TimerHandle) : 0.f;
}



void AArenaPlayerState::ClientNotifyRemainingTime_Implementation(const uint32 UniqueCode, const float RemainingTime, const float ElapsedTime)
{
	if (OnRemainingTimeChanged.IsBound())
	{
		OnRemainingTimeChanged.Broadcast(UniqueCode, RemainingTime, ElapsedTime);
	}
}

void AArenaPlayerState::ClientNotifyTimerUpdated_Implementation(const uint32 UniqueCode, const int32 ConcurrentUses)
{
	if (OnTimerUpdated.IsBound())
	{
		OnTimerUpdated.Broadcast(UniqueCode, ConcurrentUses);
	}
}
