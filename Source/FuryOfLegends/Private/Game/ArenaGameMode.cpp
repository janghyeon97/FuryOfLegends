#include "Game/ArenaGameMode.h"
#include "Game/AOSGameInstance.h"
#include "Game/ArenaGameState.h"
#include "Game/ArenaPlayerState.h"
#include "Game/PlayerStateSave.h"
#include "Characters/AOSCharacterBase.h"
#include "Characters/MinionBase.h"
#include "Controllers/AOSPlayerController.h"
#include "Controllers/MinionAIController.h"
#include "Components/SplineComponent.h"
#include "Components/StatComponent.h"
#include "Components/ActionStatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CrowdControls/CrowdControlManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Item/Item.h"
#include "NavigationSystem.h"
#include "Props/Nexus.h"
#include "Plugins/UniqueCodeGenerator.h"


AArenaGameMode::AArenaGameMode()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> ITEM_DATATABLE(TEXT("/Game/FuryOfLegends/DataTables/DT_ItemList.DT_ItemList"));
	if (ITEM_DATATABLE.Succeeded()) ItemTable = ITEM_DATATABLE.Object;
	else ItemTable = nullptr;

	static ConstructorHelpers::FObjectFinder<UDataTable> GAMEPLAY_DATATABLE(TEXT("/Game/FuryOfLegends/DataTables/DT_ItemList.DT_ItemList"));
	if (GAMEPLAY_DATATABLE.Succeeded()) GameplayConfigTable = GAMEPLAY_DATATABLE.Object;
	else GameplayConfigTable = nullptr;


	bUseSeamlessTravel = true;

	NumberOfPlayer = 0;
	ConnectedPlayer = 0;
	InitialCharacterLevel = 1;

	GameInstance = Cast<UAOSGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (::IsValid(GameInstance) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: Invalid GameInstance."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	NumberOfPlayer = GameInstance->NumberOfPlayer != -1 ? GameInstance->NumberOfPlayer : 0;
	UE_LOG(LogTemp, Warning, TEXT("[%s] NumberOfPlayer set to %u."), ANSI_TO_TCHAR(__FUNCTION__), NumberOfPlayer);
}


void AArenaGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	GameInstance = Cast<UAOSGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (::IsValid(GameInstance) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: Invalid GameInstance."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	LoadGameData();
	LoadItemData();
	LoadMinionData();
}

void AArenaGameMode::LoadItemData()
{
	if (!ItemTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: ItemTable is not initialized."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	TArray<FItemTableRow*> Items;
	ItemTable->GetAllRows<FItemTableRow>(TEXT("GENERAL"), Items);

	LoadedItems.Empty();
	LoadedItems.Shrink();
	RequiredSubItems.Empty();

	if (Items.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] No items found in ItemDataTable."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	LoadedItems.Reserve(Items.Num());

	for (const auto& ItemRow : Items)
	{
		if (!ItemRow)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Invalid ItemRow."), ANSI_TO_TCHAR(__FUNCTION__));
			continue;
		}

		if (!ItemRow->ItemClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Invalid ItemClass for ItemCode: %d"), ANSI_TO_TCHAR(__FUNCTION__), ItemRow->ItemCode);
			continue;
		}

		if (LoadedItems.Contains(ItemRow->ItemCode))
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Duplicate ItemCode found: %d"), ANSI_TO_TCHAR(__FUNCTION__), ItemRow->ItemCode);
			continue;
		}

		AItem* ItemInstance = NewObject<AItem>(this, ItemRow->ItemClass);
		if (!ItemInstance)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] Failed to create ItemInstance for ItemCode: %d, Class: %s"), ANSI_TO_TCHAR(__FUNCTION__), ItemRow->ItemCode, *ItemRow->ItemClass->GetName());
			continue;
		}

		ItemInstance->ItemCode = ItemRow->ItemCode;
		ItemInstance->Name = ItemRow->Name;
		ItemInstance->Price = ItemRow->Price;
		ItemInstance->Icon = ItemRow->Icon;
		ItemInstance->Classification = ItemRow->Classification;
		ItemInstance->Description = ItemRow->Description;
		ItemInstance->MaxStackPerSlot = ItemRow->MaxStackPerSlot;
		ItemInstance->MaxInventoryQuantity = ItemRow->MaxInventoryQuantity;
		ItemInstance->MaxConcurrentUses = ItemRow->MaxConcurrentUses;
		ItemInstance->StatModifiers = ItemRow->StatModifiers;
		ItemInstance->UniqueAttributes = ItemRow->UniqueAttributes;
		ItemInstance->RequiredItems = ItemRow->RequiredItems;
		ItemInstance->Initialize();

		// 새로운 아이템을 LoadedItems에 추가 후 하위 아이템을 재귀적으로 추가.
		LoadedItems.Add(ItemInstance->ItemCode, ItemInstance);
		GenerateSubItem(ItemInstance->ItemCode);

		UE_LOG(LogTemp, Log, TEXT("Loaded Item: %d, %s, %d, %s"),
			ItemInstance->ItemCode,
			*ItemInstance->Name,
			ItemInstance->Price,
			*ItemInstance->Description);
	}
}

void AArenaGameMode::GenerateSubItem(int32 ItemCode)
{
	if (!LoadedItems.Contains(ItemCode))
	{
		UE_LOG(LogTemp, Warning, TEXT("GenerateSubItemMap failed: Invalid ItemCode %d."), ItemCode);
		return;
	}

	TMap<int32, int32>& SubItemMap = RequiredSubItems.FindOrAdd(ItemCode);
	TFunction<void(int32)> FindSubItemsRecursive = [this, &FindSubItemsRecursive, &SubItemMap](int32 CurrentItemCode)
		{
			if (!LoadedItems.Contains(CurrentItemCode))
			{
				return;
			}

			// 현재 아이템의 하위 아이템 가져오기
			for (int32 SubItemCode : LoadedItems[CurrentItemCode]->RequiredItems)
			{
				// 하위 아이템이 이미 존재하면 개수를 증가, 없으면 추가
				SubItemMap.FindOrAdd(SubItemCode)++;

				// 재귀적으로 처리
				FindSubItemsRecursive(SubItemCode);
			}
		};

	// 재귀 탐색 시작
	FindSubItemsRecursive(ItemCode);
}

void AArenaGameMode::LoadGameData()
{
	if (!GameplayConfigTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[AArenaGameMode::LoadGameData] Invalid DataTable."));
		return;
	}

	FGameDataTableRow* DataRow = GameplayConfigTable->FindRow<FGameDataTableRow>(FName(*FString::FromInt(1)), TEXT(""));
	if (!DataRow)
	{
		UE_LOG(LogTemp, Error, TEXT("[AArenaGameMode::LoadGameData] Failed to find row in DataTable."));
		return;
	}

	GameplayConfig = *DataRow;
}

void AArenaGameMode::LoadMinionData()
{
	const UDataTable* DataTable = GameInstance->GetMinionsListTable();
	if (!DataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid DataTable."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	// 기존 데이터 클리어
	MinionsData.Empty();

	TArray<FName> RowNames = DataTable->GetRowNames();
	for (const auto& RowName : RowNames)
	{
		FMinionAttributesRow* DataRow = DataTable->FindRow<FMinionAttributesRow>(RowName, TEXT(""));
		if (!DataRow)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] Failed to find row: %s"), ANSI_TO_TCHAR(__FUNCTION__), *RowName.ToString());
			continue;
		}

		MinionsData.Add(DataRow->MinionType, *DataRow);
	}
}


void AArenaGameMode::BeginPlay()
{
	Super::BeginPlay();

	UWorld* CurrentWorld = GetWorld();
	if (::IsValid(CurrentWorld) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[AArenaGameMode::BeginPlay] Failed to retrieve the World context."));
		return;
	}

	CrowdControlManager = UCrowdControlManager::Get();
	if (!CrowdControlManager)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize CrowdControlManager."));
	}


	FindPlayerStart();
	FindTaggedActors(FName("MinionSplinePath"), MinionPaths);
	FindTaggedActors(FName("OrientationPoint"), OrientationPoints);

	//GetWorldTimerManager().SetTimer(LoadTimerHandle, this, &AArenaGameMode::StartGame, MaxLoadWaitTime, false, MaxLoadWaitTime);
	GetWorldTimerManager().SetTimerForNextTick(this, &AArenaGameMode::CheckAllPlayersLoaded);
}

void AArenaGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] World is null. Skipping cleanup."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();
	auto ClearTimers = [&TimerManager](TMap<uint32, FTimerHandle>& TimerMap)
		{
			for (auto& TimerPair : TimerMap)
			{
				if (TimerManager.IsTimerActive(TimerPair.Value))
				{
					TimerManager.ClearTimer(TimerPair.Value);
				}
			}
			TimerMap.Empty();
		};

	// Clear all timer maps
	ClearTimers(TimerHandles);
	ClearTimers(BroadcastTimerHandles);

	// Clear other maps (if necessary)
	OrientationPoints.Empty();
	PlayerStarts.Empty();

	for (auto& ItemPair : LoadedItems)
	{
		if (ItemPair.Value)
		{
			UE_LOG(LogTemp, Log, TEXT("[%s] Destroying item: %s"), ANSI_TO_TCHAR(__FUNCTION__), *ItemPair.Value->Name);
			ItemPair.Value->Destroy(true);
		}
	}

	LoadedItems.Empty(); // 모든 아이템을 제거
	LoadedItems.Shrink(); // 메모리 최적화

	// 다른 시스템에서 참조하고 있는 객체 해제
	UCrowdControlManager::Release();

	// 타이머 핸들 정리
	GetWorldTimerManager().ClearAllTimersForObject(this);
}


void AArenaGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (::IsValid(NewPlayer) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid PlayerController."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	AAOSPlayerController* NewPlayerController = Cast<AAOSPlayerController>(NewPlayer);
	if (::IsValid(NewPlayerController) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid AOSPlayerController."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}


	AArenaPlayerState* NewPlayerState = NewPlayerController->GetPlayerState<AArenaPlayerState>();
	if (::IsValid(NewPlayerState) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid NewPlayerState."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	FString SaveSlotName = NewPlayerState->GetPlayerUniqueID().Replace(TEXT("|"), TEXT("_"));
	UE_LOG(LogTemp, Warning, TEXT("[%s] Load PlayerData: Slot: %s"), ANSI_TO_TCHAR(__FUNCTION__), *SaveSlotName);

	UPlayerStateSave* PlayerStateSave = Cast<UPlayerStateSave>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	if (::IsValid(PlayerStateSave) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed to load save slot: %s. Using default settings."), ANSI_TO_TCHAR(__FUNCTION__), *SaveSlotName);
		PlayerStateSave = GetMutableDefault<UPlayerStateSave>();
	}

	ETeamSide TeamSide = PlayerStateSave->TeamSide != ETeamSide::Red ? ETeamSide::Blue : ETeamSide::Red;
	FName ChampionRowName = !PlayerStateSave->ChosenChampionName.IsNone() ? PlayerStateSave->ChosenChampionName : GameplayConfig.DefaultCharacter;
	int32 PlayerIndex = PlayerStateSave->PlayerIndex != -1 ? PlayerStateSave->PlayerIndex : Players.Num() + 1;

	NewPlayerState->TeamSide = TeamSide;
	NewPlayerState->SetPlayerIndex(PlayerIndex);
	NewPlayerState->SetChosenChampionName(ChampionRowName);
	NewPlayerState->SetPlayerUniqueID(PlayerStateSave->PlayerUniqueID);

	if (GameInstance->NumberOfPlayer == -1) NumberOfPlayer++;

	UE_LOG(LogTemp, Log, TEXT("[%s] Player Logged In:\n")
		TEXT("  - Name: %s\n")
		TEXT("  - Index: %d\n")
		TEXT("  - Champion: %s\n")
		TEXT("  - UniqueID: %s\n")
		TEXT("  - Team: %s\n")
		TEXT("  - Connected Players: %u / %u"),
		ANSI_TO_TCHAR(__FUNCTION__),
		*NewPlayerState->GetPlayerName(),
		NewPlayerState->GetPlayerIndex(),
		*NewPlayerState->GetChosenChampionName().ToString(),
		*NewPlayerState->GetPlayerUniqueID(),
		NewPlayerState->TeamSide == ETeamSide::Blue ? TEXT("Blue") : TEXT("Red"),
		ConnectedPlayer,
		NumberOfPlayer);

	if (Players.Contains(PlayerIndex))
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Duplicate PlayerIndex detected: %d"), ANSI_TO_TCHAR(__FUNCTION__), PlayerIndex);
		return;
	}

	Players.Add(PlayerIndex, FPlayerInformation(PlayerIndex, nullptr, NewPlayerController, NewPlayerState));
	//SpawnCharacter(NewPlayerController, ChampionRowName, TeamSide, PlayerIndex);
}

void AArenaGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	AAOSPlayerController* ExitingPlayerController = Cast<AAOSPlayerController>(Exiting);
	if (::IsValid(ExitingPlayerController) == false)
	{
		return;
	}

	AArenaPlayerState* ExitingPlayerState = ExitingPlayerController->GetPlayerState<AArenaPlayerState>();
	if (::IsValid(ExitingPlayerState) == false)
	{
		return;
	}

	int32 PlayerIndex = ExitingPlayerState->GetPlayerIndex();
	if (Players.Find(PlayerIndex))
	{
		ExitingPlayers.Add(Players[PlayerIndex]);
		Players.Remove(PlayerIndex);
	}
}


void AArenaGameMode::PlayerLoaded(APlayerController* PlayerController)
{
	if (::IsValid(PlayerController) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid AOSPlayerController."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	AAOSPlayerController* LoadedPlayerController = Cast<AAOSPlayerController>(PlayerController);
	if (::IsValid(LoadedPlayerController) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid AOSPlayerController."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	AArenaPlayerState* LoadedPlayerState = PlayerController->GetPlayerState<AArenaPlayerState>();
	if (::IsValid(LoadedPlayerState) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid ArenaPlayerState."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	ETeamSide TeamSide = LoadedPlayerState->TeamSide;
	FName ChampionRowName = LoadedPlayerState->GetChosenChampionName();
	int32 PlayerIndex = LoadedPlayerState->GetPlayerIndex();

	if (ChampionRowName.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] ChampionRowName is None. Ensure DefaultCharacter is set."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	SpawnCharacter(LoadedPlayerController, ChampionRowName, TeamSide, PlayerIndex);
}

void AArenaGameMode::PlayerPawnReady(AAOSCharacterBase* PlayerCharacter)
{
	if (::IsValid(PlayerCharacter) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] PlayerCharacter is invalid (nullptr). Cannot mark as ready."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	ConnectedPlayer++;

	UE_LOG(LogTemp, Log, TEXT("[%s] Player '%s' is ready. Total connected players: %u/%u"),
		ANSI_TO_TCHAR(__FUNCTION__), *PlayerCharacter->GetName(), ConnectedPlayer, NumberOfPlayer);
}


void AArenaGameMode::CheckAllPlayersLoaded()
{
	bool bAllPlayersLoaded = (NumberOfPlayer > 0 && NumberOfPlayer == ConnectedPlayer);

	if (bAllPlayersLoaded)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] All players have loaded. Starting the game."), ANSI_TO_TCHAR(__FUNCTION__));
		GetWorldTimerManager().ClearTimer(LoadTimerHandle);
		StartGame();
	}
	else
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &AArenaGameMode::CheckAllPlayersLoaded);
	}
}

void AArenaGameMode::SpawnCharacter(AAOSPlayerController* PlayerController, const FName& ChampionRowName, ETeamSide Team, const int32 PlayerIndex)
{
	// 플레이어 시작 지점을 결정합니다.
	FName PlayerStartName = Team == ETeamSide::Blue ? FName(*FString::Printf(TEXT("Blue%d"), PlayerIndex)) : FName(*FString::Printf(TEXT("Red%d"), PlayerIndex));
	FName TeamName = (Team == ETeamSide::Blue) ? FName(TEXT("Blue")) : FName(TEXT("Red"));

	// 챔피언 데이터 테이블에서 캐릭터 데이터를 가져옵니다.
	const FCharacterAttributesRow* CharacterData = GameInstance->GetChampionListTableRow(ChampionRowName);
	if (!CharacterData || !CharacterData->CharacterClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid CharacterIndex or CharacterClass."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	// 기본값으로 팀에 따라 스폰 위치 초기화
	FVector SpawnLocation = (Team == ETeamSide::Blue) ? FVector(14000.f, 14000.f, 430.f) : FVector(-14000.f, -14000.f, 430.f);

	if (PlayerStarts.Contains(PlayerStartName) && ::IsValid(PlayerStarts[PlayerStartName]))
	{
		SpawnLocation = PlayerStarts[PlayerStartName]->GetActorLocation() + FVector(0, 0, 95);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FTransform SpawnTransform(FRotator(0), SpawnLocation, FVector::OneVector);
	AAOSCharacterBase* Character = GetWorld()->SpawnActor<AAOSCharacterBase>(CharacterData->CharacterClass, SpawnTransform, SpawnParams);
	if (!::IsValid(Character))
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed to spawn character."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	AArenaGameState* ArenaGameState = Cast<AArenaGameState>(GameState);
	if (::IsValid(ArenaGameState))
	{
		ArenaGameState->AddPlayerCharacter(Character, Team);
	}

	Character->SetOwner(PlayerController);
	Character->SetActorTickEnabled(false);
	Character->ClientDisableInput();
	PlayerController->Possess(Character);

	if (Players.Contains(PlayerIndex))
	{
		Players[PlayerIndex].PlayerCharacter = Character;
	}
}


void AArenaGameMode::StartGame()
{
	AArenaGameState* ArenaGameState = Cast<AArenaGameState>(GameState);
	if (::IsValid(ArenaGameState) == false)
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &AArenaGameMode::StartGame);
		return;
	}

	bool bLoadTimerActicate = GetWorld()->GetTimerManager().IsTimerActive(LoadTimerHandle);
	if (bLoadTimerActicate)
	{
		GetWorld()->GetTimerManager().ClearTimer(LoadTimerHandle);
	}

	for (const TPair<int32, FPlayerInformation>& PlayerEntry : Players)
	{
		const FPlayerInformation& PlayerInfo = PlayerEntry.Value;
		if (PlayerInfo.Index == -1)
		{
			continue;
		}

		AAOSPlayerController* PlayerController = PlayerInfo.PlayerController;
		if (::IsValid(PlayerController) == false)
		{
			continue;
		}

		AAOSCharacterBase* PlayerCharacter = PlayerInfo.PlayerCharacter;
		if (::IsValid(PlayerCharacter) == false)
		{
			continue;
		}

		FName TeamName = (PlayerCharacter->TeamSide == ETeamSide::Blue) ? FName(TEXT("Blue")) : FName(TEXT("Red"));
		FVector OrientationPoint = (PlayerCharacter->TeamSide == ETeamSide::Blue) ? FVector(12500.f, 12500.f, 335.f) : FVector(-12500.f, -12500.f, 335.f);
		FVector CurrentLocation = PlayerCharacter->GetActorLocation();

		if (OrientationPoints.Contains(TeamName) && ::IsValid(OrientationPoints[TeamName]))
		{
			OrientationPoint = OrientationPoints[TeamName]->GetActorLocation();
		}

		FVector LookVector = (OrientationPoint - CurrentLocation).GetSafeNormal();
		LookVector.Z = 0.f;
		FRotator SpawnRotation = UKismetMathLibrary::MakeRotFromX(LookVector);

		PlayerController->SetControlRotation(SpawnRotation);
		PlayerCharacter->ClientSetControlRotation(SpawnRotation);
		
		PlayerController->RemoveLoadingScreen();
		PlayerCharacter->SetActorTickEnabled(true);
		PlayerCharacter->ClientEnableInput();
	}

	UE_LOG(LogTemp, Warning, TEXT("[%s] Start Game."), ANSI_TO_TCHAR(__FUNCTION__));

	ArenaGameState->StartGame();

	FTimerHandle NewTimerHandle;
	GetWorldTimerManager().SetTimer(NewTimerHandle, this, &ThisClass::ActivateSpawnMinion, GameplayConfig.MinionSpawnInterval, true, GameplayConfig.MinionSpawnTime);
	ActivateCurrencyIncrement();
}

void AArenaGameMode::EndGame()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &AArenaGameMode::EndGame);
		return;
	}

	UGameplayStatics::OpenLevel(World, TEXT("Title"), true);
}

void AArenaGameMode::NotifyNexusDestroyed(ANexus* Nexus)
{
	if (!Nexus)
	{
		return;
	}

	if (bHasNexusDestroyed)
	{
		return;
	}

	bHasNexusDestroyed = true;
	ETeamSide DefeatedTeam = Nexus->TeamSide;

	for (const TPair<int32, FPlayerInformation>& PlayerEntry : Players)
	{
		const FPlayerInformation& PlayerInfo = PlayerEntry.Value;
		if (PlayerInfo.Index == -1)
		{
			continue;
		}

		AAOSPlayerController* PlayerController = PlayerInfo.PlayerController;
		if (::IsValid(PlayerController) == false)
		{
			continue;
		}

		AAOSCharacterBase* PlayerCharacter = PlayerInfo.PlayerCharacter;
		if (::IsValid(PlayerCharacter) == false)
		{
			continue;
		}

		bool bIsVictorious = PlayerCharacter->TeamSide != DefeatedTeam;
		PlayerController->ClientHandleGameEnd(Nexus, bIsVictorious);
	}

	FTimerHandle NewTimerHandle;
	GetWorldTimerManager().SetTimer(NewTimerHandle, this, &AArenaGameMode::EndGame, MaxEndWaitTimer, false);
}


void AArenaGameMode::ActivateCurrencyIncrement()
{
	GetWorldTimerManager().SetTimer(AddCurrencyTimerHandle, this, &AArenaGameMode::IncrementPlayerCurrency, 1.0f, true);
}


void AArenaGameMode::IncrementPlayerCurrency()
{
	for (const TPair<int32, FPlayerInformation>& Player : Players)
	{
		const FPlayerInformation& PlayerInformation = Player.Value;

		if (!::IsValid(PlayerInformation.PlayerState))
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to add currency: Invalid PlayerState for Player ID %d."), ANSI_TO_TCHAR(__FUNCTION__), Player.Key);
			continue;
		}

		PlayerInformation.PlayerState->AddCurrency(GameplayConfig.IncrementCurrencyAmount);
	}
}



void AArenaGameMode::ActivateSpawnMinion()
{
	GetWorldTimerManager().SetTimer(SpawnMinionTimerHandle, [this]()
		{
			SpawnCount++;
			SpawnMinionsForLane(ELaneType::Top);
			SpawnMinionsForLane(ELaneType::Mid);
			SpawnMinionsForLane(ELaneType::Bottom);
		}, GameplayConfig.MinionSpawnInterval, true, 0.0f); // 90초마다 반복 실행

	UE_LOG(LogTemp, Log, TEXT("[%s] Timer set for minion spawning every 90 seconds."), ANSI_TO_TCHAR(__FUNCTION__));
}


void AArenaGameMode::SpawnMinionsForLane(ELaneType Lane)
{
	const int32 MinionsPerWave = (SpawnCount % GameplayConfig.SuperMinionSpawnInterval == 0) ? GameplayConfig.MinionsPerWave + 1 : GameplayConfig.MinionsPerWave;
	uint32 UniqueCode = UUniqueCodeGenerator::GenerateUniqueCode(EObjectType::System, 0, ETimerCategory::Spawn, static_cast<uint8>(EObjectType::Minion), static_cast<uint8>(Lane));

	TSharedPtr<int32> CurrentIndex = MakeShareable(new int32(0));
	auto TimerCallback = [this, Lane, MinionsPerWave, CurrentIndex, UniqueCode]()
		{
			if (*CurrentIndex >= MinionsPerWave)
			{
				// 모든 미니언 스폰이 완료되었으므로 타이머를 해제
				ClearTimer(UniqueCode);
				return;
			}

			EMinionType MinionType;

			if (SpawnCount % GameplayConfig.SuperMinionSpawnInterval == 0) // 3번째 소환마다
			{
				if (*CurrentIndex < GameplayConfig.MinionsPerWave / 2)
				{
					MinionType = EMinionType::Melee; // 처음 3마리는 Melee
				}
				else if (*CurrentIndex == GameplayConfig.MinionsPerWave / 2)
				{
					MinionType = EMinionType::Super; // 4번째는 Super Minion
				}
				else
				{
					MinionType = EMinionType::Ranged; // 나머지는 Ranged
				}
			}
			else
			{
				MinionType = (*CurrentIndex < GameplayConfig.MinionsPerWave / 2) ? EMinionType::Melee : EMinionType::Ranged; // 평소에는 Melee 3, Ranged 3
				const TCHAR* MinionTypeStr = (*CurrentIndex < 3) ? TEXT("Melee") : TEXT("Ranged");
			}

			SpawnMinion(MinionType, Lane, ETeamSide::Blue);
			SpawnMinion(MinionType, Lane, ETeamSide::Red);
			(*CurrentIndex)++;
		};

	SetTimer(UniqueCode, TimerCallback, GameplayConfig.SpawnInterval, true);
}


void AArenaGameMode::SpawnMinion(EMinionType MinionType, ELaneType Lane, ETeamSide Team)
{
	FName LaneName = *StaticEnum<ELaneType>()->GetNameStringByValue(static_cast<int64>(Lane));

	// SplinePaths 에 해당 라인이 있는지 확인
	if (!MinionPaths.Contains(LaneName))
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] SplinePath for line: %s not found."), ANSI_TO_TCHAR(__FUNCTION__), *LaneName.ToString());
		return;
	}

	FMinionAttributesRow* MinionDataPtr = MinionsData.Find(MinionType);
	if (!MinionDataPtr)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid MinionType: %d"), ANSI_TO_TCHAR(__FUNCTION__), (int32)MinionType);
		return;
	}

	// SkeletalMeshComponent 유효성 확인
	if (!MinionDataPtr->SkeletalMesh_Down || !MinionDataPtr->SkeletalMesh_Dusk)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Missing SkeletalMesh for MinionType: %d"), ANSI_TO_TCHAR(__FUNCTION__), (int32)MinionType);
		return;
	}

	FVector SpawnLocation = FVector(0);
	FRotator SpawnRotation;
	USplineComponent* Spline = MinionPaths[LaneName]->FindComponentByClass<USplineComponent>();
	if (Spline)
	{
		// 팀에 따라 스플라인 시작 위치 또는 끝 위치 선택
		float SplineDistanceKey = Team == ETeamSide::Blue ? 0 : Spline->GetSplineLength();
		FVector SplineStartLocation = Spline->GetLocationAtDistanceAlongSpline(SplineDistanceKey, ESplineCoordinateSpace::World);

		// 스플라인 시작 위치에서 50만큼 떨어진 지점 계산
		float TargetDistance = SplineDistanceKey + (Team == ETeamSide::Blue ? 50 : -50);
		FVector TargetLocation = Spline->GetLocationAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);

		// 스폰 위치 및 회전 설정
		SpawnLocation = SplineStartLocation;
		SpawnRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, TargetLocation);
	}

	FTransform SpawnTransform(SpawnRotation, SpawnLocation);

	// Minion 스폰
	AMinionBase* NewMinion = Cast<AMinionBase>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, MinionDataPtr->MinionClass, SpawnTransform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn));
	if (!NewMinion)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed to spawn Minion of type: %d"), ANSI_TO_TCHAR(__FUNCTION__), (int32)MinionType);
		return;
	}

	NewMinion->ReplicatedSkeletalMesh = Team == ETeamSide::Blue ? MinionDataPtr->SkeletalMesh_Down : MinionDataPtr->SkeletalMesh_Dusk;
	NewMinion->ObjectType = EObjectType::Minion;
	NewMinion->TeamSide = Team;

	NewMinion->ExperienceShareRadius = GameplayConfig.ExperienceShareRadius;
	NewMinion->ShareFactor.Add(1, 1.0f);
	NewMinion->ShareFactor.Add(2, GameplayConfig.ExpShareFactorTwoPlayers);
	NewMinion->ShareFactor.Add(3, GameplayConfig.ExpShareFactorThreePlayers);
	NewMinion->ShareFactor.Add(4, GameplayConfig.ExpShareFactorFourPlayers);
	NewMinion->ShareFactor.Add(5, GameplayConfig.ExpShareFactorFivePlayers);

	NewMinion->SetExpBounty(MinionDataPtr->ExpBounty);
	NewMinion->SetGoldBounty(MinionDataPtr->GoldBounty);

	NewMinion->ChaseThreshold = GameplayConfig.ChaseThreshold;

	// SplineActor 설정
	NewMinion->SplineActor = MinionPaths[LaneName];
	UGameplayStatics::FinishSpawningActor(NewMinion, SpawnTransform);

	// AI Controller 수동 생성 및 미니언 빙의
	AMinionAIController* MinionAIController = GetWorld()->SpawnActor<AMinionAIController>(AMinionAIController::StaticClass(), SpawnTransform);
	if (!MinionAIController)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s][Character: %s][Reason: Failed to spawn AIController]"), ANSI_TO_TCHAR(__FUNCTION__), *NewMinion->GetName());
		return;
	}

	MinionAIController->Possess(NewMinion);
}


void AArenaGameMode::FindPlayerStart()
{
	UWorld* CurrentWorld = GetWorld();
	if (::IsValid(CurrentWorld) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[AArenaGameMode::FindPlayerStart] Failed to retrieve the World context."));
		return;
	}

	// 플레이어 시작 지점을 미리 확보할 수 있도록 배열을 예약합니다.
	constexpr int32 ExpectedPlayerStarts = 30;
	PlayerStarts.Reserve(ExpectedPlayerStarts);

	// 플레이어 시작 지점을 일괄적으로 검색합니다.
	TArray<AActor*> PlayerStartActors;
	UGameplayStatics::GetAllActorsOfClass(CurrentWorld, APlayerStart::StaticClass(), PlayerStartActors);

	// 검색된 플레이어 시작 지점이 없는 경우 로그 출력
	if (PlayerStartActors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FindPlayerStart: No PlayerStart actors found in the level."));
		return;
	}

	for (AActor* Actor : PlayerStartActors)
	{
		if (!Actor || Actor->Tags.Num() == 0)
		{
			continue; // 유효하지 않은 Actor나 Tag가 없는 경우 건너뜁니다.
		}

		APlayerStart* PlayerStart = Cast<APlayerStart>(Actor);
		if (!PlayerStart)
		{
			continue;
		}

		// 첫 번째 태그를 가져옵니다.
		FName Tag = Actor->Tags[0];
		PlayerStarts.Add(PlayerStart->PlayerStartTag, PlayerStart);
	}
}


/**
 * FindTaggedActors 함수는 주어진 PrimaryTag를 가진 액터들을 찾고,
 * 해당 액터의 두 번째 태그(SecondaryTag)를 기준으로 맵(TargetMap)에 저장하는 역할을 합니다.
 *
 * 기능 개요:
 * - World 내의 모든 AActor들을 검색하여 특정 PrimaryTag를 가지고 있는 액터들을 필터링합니다.
 * - 필터링된 액터는 두 번째 태그(SecondaryTag)를 추출하여, 이를 키로 사용하여 TMap에 저장합니다.
 * - 이미 동일한 SecondaryTag가 맵에 존재하는 경우, 중복을 방지하고 경고 메시지를 출력합니다.
 * - 태그가 두 개 미만인 액터는 무시하며, 로그 메시지를 통해 태그가 부족한 액터에 대한 경고를 남깁니다.
 *
 * @param PrimaryTag: 검색할 첫 번째 태그로, 이 태그가 액터의 첫 번째 태그에 있어야 필터링됩니다.
 * @param TargetMap: 필터링된 액터들을 저장할 TMap으로, SecondaryTag를 키로 사용하고, 해당 액터를 값으로 저장합니다.
 */
void AArenaGameMode::FindTaggedActors(FName PrimaryTag, TMap<FName, AActor*>& TargetMap)
{
	UWorld* CurrentWorld = GetWorld();
	if (::IsValid(CurrentWorld) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[AArenaGameMode::FindTaggedActors] Failed to retrieve the World context."));
		return;
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(CurrentWorld, AActor::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		// 최소한 두 개 이상의 태그가 있는지 확인
		if (Actor->Tags.Num() < 2)
		{
			continue;
		}

		// 첫 번째 태그가 PrimaryTag인지 확인
		if (Actor->Tags[0] == PrimaryTag)
		{
			FName SecondaryTag = Actor->Tags[1];
			// 이미 해당 SecondaryTag가 TargetMap에 존재하는지 확인
			if (!TargetMap.Contains(SecondaryTag))
			{
				TargetMap.Add(SecondaryTag, Actor);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s Tag %s is already present in the map."), *PrimaryTag.ToString(), *SecondaryTag.ToString());
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Actor search completed. Found %d entries for tag %s."), TargetMap.Num(), *PrimaryTag.ToString());
}


void AArenaGameMode::RequestRespawn(const int32 PlayerIndex)
{
	if (!Players.Contains(PlayerIndex))
	{
		return;
	}

	AAOSCharacterBase* Character = Players[PlayerIndex].PlayerCharacter;
	if (!::IsValid(Character))
	{
		return;
	}

	float RespawnTime = CalculateRespawnTime(Character);
	uint32 UniqueCode = UUniqueCodeGenerator::GenerateUniqueCode(EObjectType::System, 0, ETimerCategory::Respawn, static_cast<uint8>(EObjectType::Player), PlayerIndex);

	FTimerDelegate RespawnDelegate;
	RespawnDelegate.BindLambda([this, PlayerIndex]()
		{
			RespawnCharacter(PlayerIndex);
		});

	SetTimer(UniqueCode, RespawnDelegate, RespawnTime, false, RespawnTime, true);
}


void AArenaGameMode::RespawnCharacter(const int32 PlayerIndex)
{
	if (!Players.Contains(PlayerIndex))
	{
		return;
	}

	AAOSCharacterBase* Character = Players[PlayerIndex].PlayerCharacter;
	if (!::IsValid(Character))
	{
		return;
	}

	FName TeamName = (Character->TeamSide == ETeamSide::Blue) ? FName(TEXT("Blue")) : FName(TEXT("Red"));

	// 팀에 따른 기본 리스폰 위치 결정
	FName RespawnLocationName = (Character->TeamSide == ETeamSide::Blue) ? FName(TEXT("BlueRespawn")) : FName(TEXT("RedRespawn"));

	// 기본 리스폰 지점이 없으면, 하드코딩된 좌표 사용
	FVector SpawnLocation = (Character->TeamSide == ETeamSide::Blue) ? FVector(1070.f, 1640.f, 200.f) : FVector(200, 200, 200);
	if (PlayerStarts.Contains(RespawnLocationName))
	{
		AActor* RespawnPointActor = PlayerStarts[RespawnLocationName];
		if (::IsValid(RespawnPointActor))
		{
			SpawnLocation = RespawnPointActor->GetActorLocation();
		}
	}

	// 팀에 따른 바라보는 방향 지점 설정 (기본값)
	FVector OrientationPoint = (Character->TeamSide == ETeamSide::Blue) ? FVector(1400.f, 1930.f, 200.f) : FVector(250, 250, 250);

	// 유효한 OrientationPoint 위치 확인
	if (OrientationPoints.Contains(TeamName))
	{
		AActor* OrientationActor = OrientationPoints[TeamName];
		if (::IsValid(OrientationActor))
		{
			OrientationPoint = OrientationActor->GetActorLocation();
		}		
	}

	// 캐릭터를 바라보는 방향 설정
	FVector LookVector = (OrientationPoint - SpawnLocation).GetSafeNormal();
	LookVector.Z = 0.f;

	FRotator SpawnRotation = FRotationMatrix::MakeFromX(LookVector).Rotator();
	FTransform SpawnTransform(SpawnRotation, SpawnLocation, FVector::OneVector);

	// 캐릭터를 해당 위치로 이동 후 리스폰
	Character->SetActorLocation(SpawnLocation);
	Character->ClientSetControlRotation(SpawnRotation);
	Character->Respawn(1.f);
}



float AArenaGameMode::CalculateRespawnTime(AAOSCharacterBase* Character) const
{
	AArenaGameState* ArenaGameState = Cast<AArenaGameState>(GameState);
	if (::IsValid(ArenaGameState) == false)
	{
		return 5.0f;
	}

	if (UStatComponent* StatComponent = Character->GetStatComponent())
	{
		float ElapsedTime = ArenaGameState->GetElapsedTime();
		int32 CharacterLevel = StatComponent->GetCurrentLevel();

		// 리스폰 시간을 계산하는 로직
		float BaseRespawnTime = 5.0f;
		float TimeFactor = ElapsedTime / 60.0f; // 분 단위로 증가
		float LevelFactor = CharacterLevel * 2.f; // 레벨 당 2초 증가

		return BaseRespawnTime + TimeFactor + LevelFactor;
	}

	return 5.f;
}







/** --------------------------------------------------------------------------------
 * 타이머 관리 함수들
 *
 * 이 함수들은 플레이어 상태에서 타이머를 관리하는 데 사용됩니다.
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
bool AArenaGameMode::IsTimerActive(const uint32 UniqueCode) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] GetWorld() returned null. UniqueCode: %d"), ANSI_TO_TCHAR(__FUNCTION__), UniqueCode);
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
void AArenaGameMode::InternalSetTimer(const uint32 UniqueCode, FTimerUnifiedDelegate&& InDelegate, float InRate, bool bInLoop, float InFirstDelay, bool bBroadcast)
{
	if (!InDelegate.IsBound())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] TimerDelegate is not bound for UniqueCode: %d"), ANSI_TO_TCHAR(__FUNCTION__), UniqueCode);
		return;
	}

	if (InRate <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Invalid timer rate (%f) for UniqueCode: %d"), ANSI_TO_TCHAR(__FUNCTION__), InRate, UniqueCode);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] GetWorld() returned null. UniqueCode: %d"), ANSI_TO_TCHAR(__FUNCTION__), UniqueCode);
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
				BroadcastRemainingTime(UniqueCode, RemainingTime, ElapsedTime);
			}, 0.05f, true, 0.0f);
	}

	UE_LOG(LogTemp, Log, TEXT("[%s] Timer set successfully. UniqueCode: %u, Rate: %f, Loop: %s, FirstDelay: %f"), ANSI_TO_TCHAR(__FUNCTION__), UniqueCode, InRate, bInLoop ? TEXT("true") : TEXT("false"), InFirstDelay);
}


/**
 * 타이머를 제거합니다.
 *
 * 주어진 ID에 대해 활성화된 타이머와 주기적으로 남은 시간을 브로드캐스트하는 타이머를 제거합니다.
 *
 * @param UniqueCode 제거할 타이머의 ID.
 */
void AArenaGameMode::ClearTimer(const uint32 UniqueCode)
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
 * @param ItemCode 남은 시간을 확인할 타이머의 ID.
 * @return 타이머의 남은 시간(초) 또는 타이머가 없으면 0.
 */
float AArenaGameMode::GetTimerRemaining(const uint32 UniqueCode) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] GetWorld() returned null. UniqueCode: %d"), ANSI_TO_TCHAR(__FUNCTION__), UniqueCode);
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
 * @param UniqueCode 남은 시간을 확인할 타이머의 Timer ID.
 * @return 타이머의 경과 시간(초) 또는 타이머가 없으면 0.
 */
float AArenaGameMode::GetTimerElapsedTime(const uint32 UniqueCode) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] GetWorld() returned null. UniqueCode: %d"), ANSI_TO_TCHAR(__FUNCTION__), UniqueCode);
		return 0.f;
	}

	const FTimerHandle* TimerHandle = TimerHandles.Find(UniqueCode);
	return TimerHandle ? World->GetTimerManager().GetTimerElapsed(*TimerHandle) : 0.f;
}



void AArenaGameMode::BroadcastRemainingTime(const uint32 UniqueCode, const float RemainingTime, const float ElapsedTime)
{
	AArenaGameState* ArenaGameState = Cast<AArenaGameState>(GameState);
	if (::IsValid(ArenaGameState) == false)
	{
		return;
	}

	ArenaGameState->MulticastBroadcastRespawnTime(UniqueCode, RemainingTime, ElapsedTime);
}


void AArenaGameMode::AddCurrencyToPlayer(ACharacterBase* Character, int32 Amount)
{
	if (!::IsValid(Character))
	{
		return;
	}

	UStatComponent* StatComopnent = Character->GetStatComponent();
	if (!StatComopnent)
	{
		return;
	}

	AArenaPlayerState* PlayerState = Character->GetPlayerState<AArenaPlayerState>();
	if (!PlayerState)
	{
		return;
	}

	int32 CurrentGold = PlayerState->GetCurrency();
	PlayerState->SetCurrency(CurrentGold + Amount);
}

void AArenaGameMode::AddExpToPlayer(ACharacterBase* Character, int32 Amount)
{
	if (!::IsValid(Character))
	{
		return;
	}

	UStatComponent* StatComopnent = Character->GetStatComponent();
	if (!StatComopnent)
	{
		return;
	}

	int32 CurrentExp = StatComopnent->GetCurrentEXP();
	StatComopnent->ModifyCurrentEXP(Amount);
}


const TMap<int32, int32>* AArenaGameMode::GetSubItemsForItem(int32 ItemCode) const
{
	const TMap<int32, int32>* FoundSubItems = RequiredSubItems.Find(ItemCode);
	return FoundSubItems ? FoundSubItems : nullptr;
}

TArray<FItemTableRow> AArenaGameMode::GetLoadedItems() const
{
	TArray<FItemTableRow> OutItems;

	for (const auto& ItemPair : LoadedItems)
	{
		if (ItemPair.Value == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("[AArenaGameMode::GetLoadedItems] Item index %d is not valid."), ItemPair.Key);
			continue;  // null 아이템을 건너뜀
		}

		FItemTableRow Item;
		Item.ItemCode = ItemPair.Value->ItemCode;
		Item.Name = ItemPair.Value->Name;
		Item.Price = ItemPair.Value->Price;
		Item.Description = ItemPair.Value->Description;
		Item.Icon = ItemPair.Value->Icon;
		Item.Classification = ItemPair.Value->Classification;
		Item.MaxConcurrentUses = ItemPair.Value->MaxConcurrentUses;
		Item.MaxStackPerSlot = ItemPair.Value->MaxStackPerSlot;
		Item.MaxInventoryQuantity = ItemPair.Value->MaxInventoryQuantity;
		Item.Classification = ItemPair.Value->Classification;
		Item.StatModifiers = ItemPair.Value->StatModifiers;
		Item.RequiredItems = ItemPair.Value->RequiredItems;

		OutItems.Add(Item);
	}

	return OutItems;
}