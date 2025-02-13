// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Structs/ActionData.h"
#include "ActionStatComponent.generated.h"

struct FActionStatTableRow;
class UStatComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCooldownTimeChangedDelegate, EActionSlot, SlotID, float, CurrentCooldownTime, float, MaxCooldownTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnReuseTimeChangedDelegate, EActionSlot, SlotID, float, CurrentReuseTime, float, MaxReuseTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActivationChangedDelegate, EActionSlot, SlotID, bool, bIsActivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpgradeStateChangedDelegate, EActionSlot, SlotID, bool, bCanUpgrade);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionLevelChangedDelegate, EActionSlot, SlotID, int, Level);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAlertTextChangedDelegate, const FString&, String);





USTRUCT(BlueprintType)
struct FActiveActionState
{
	GENERATED_BODY()

public:
	FActiveActionState()
		: SlotID(EActionSlot::None)
		, Name(NAME_None)
		, Description(FText())
		, MaxLevel(0)
		, CurrentLevel(0)
		, MaxInstance(0)
		, InstanceIndex(0)
		, bCanCastAction(true)
		, ActiveCrowdControlCount(0)
		, bIsUpgradable(false)
		, LastUseTime(0.f)
		, MaxCooldown(0.f)
		, Cooldown(0.f)
		, MaxReuseDuration(0.f)
		, ReuseDuration(0.f)
		, ActionType(EActionType::None)
		, ActivationType(EActivationType::None)
		, CollisionDetection(ECollisionChannel::ECC_Visibility)
	{
	};

public:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|ActionStat")
	EActionSlot SlotID;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|ActionStat")
	FName Name;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|ActionStat")
	FText Description;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|ActionStat")
	int32 MaxLevel;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|ActionStat")
	int32 CurrentLevel;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|ActionStat")
	int32 MaxInstance;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|ActionStat")
	int32 InstanceIndex;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|ActionStat")
	bool bCanCastAction;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|ActionStat")
	int32 ActiveCrowdControlCount;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|ActionStat")
	bool bIsUpgradable;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|ActionStat")
	float LastUseTime;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|ActionStat")
	float MaxCooldown;
		
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|ActionStat")
	float Cooldown;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|ActionStat")
	float MaxReuseDuration;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|ActionStat")
	float ReuseDuration;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|ActionStat")
	EActionType ActionType;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|ActionStat")
	EActivationType ActivationType;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|ActionStat")
	TEnumAsByte<ECollisionChannel> CollisionDetection;
};



UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FURYOFLEGENDS_API UActionStatComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class AAOSCharacterBase;

public:
	UActionStatComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual bool InitializeActionAtLevel(EActionSlot SlotID, const int32 InLevel);
	virtual void InitActionStatComponent(UDataTable* InDataTable, UStatComponent* InStatComponent);

	FActiveActionState& GetActiveActionState(EActionSlot SlotID);
	const FActionAttributes& GetActionAttributes(EActionSlot SlotID) const;
	float GetUniqueValue(EActionSlot SlotID, const FName& InKey, float DefaultValue);
	bool IsActionReady(EActionSlot SlotID) const;
	TArray<FActiveActionState*> GetActiveActionStatePtrs();

	UFUNCTION(Server, Reliable)
	void HandleActionExecution(EActionSlot SlotID, float CurrentTime);

	UFUNCTION(Server, Reliable)
	void ActivateActionCooldown(EActionSlot SlotID);

	UFUNCTION(Server, Reliable)
	void ServerToggleUpgradeStat(bool Visibility);

	UFUNCTION(Server, Reliable)
	void ServerUpdateUpgradableStatus(int32 InOldCurrentLevel, int32 InNewCurrentLevel);

	UFUNCTION(Client, Reliable)
	void ClientNotifyCooldownChanged(EActionSlot SlotID, const float CurrentCooldownTime, const float MaxCooldownTime);

	UFUNCTION(Client, Reliable)
	void ClientNotifyReuseTimerChanged(EActionSlot SlotID, const float CurrentReuseTime, const float MaxResueTime);

	UFUNCTION(Client, Reliable)
	void ClientNotifyUpgradeStateChanged(EActionSlot SlotID, bool bCanUpgrade);

	UFUNCTION(Client, Reliable)
	void ClientNotifyActivationChanged(EActionSlot SlotID, bool bIsActivated);

	UFUNCTION(Client, Reliable)
	void ClientNotifyLevelChanged(EActionSlot SlotID, int InLevel);

	UFUNCTION(Client, Reliable)
	void ClientNotifyAlertTextChanged(const FString& InString);

public:
	FOnCooldownTimeChangedDelegate OnCooldownTimeChanged;
	FOnReuseTimeChangedDelegate OnReuseTimeChanged;
	FOnActivationChangedDelegate OnActivationChanged;
	FOnUpgradeStateChangedDelegate OnUpgradeStateChanged;
	FOnActionLevelChangedDelegate OnActionLevelChanged;
	FOnAlertTextChangedDelegate OnAlertTextChanged;

private:
	bool InitializeAction(EActionSlot SlotID, FActiveActionState& ActiveActionState, TArray<FActionAttributes>& ActionAttributesSlot, const int32 InLevel);
	void InitializeActiveActionState(EActionSlot SlotID, FActiveActionState& ActiveActionState);

	void ExecuteAction(FActiveActionState& ActiveActionState, TArray<FActionAttributes>& ActionAttributesSlot, EActionSlot SlotID);
	void UpdateUpgradableStatus(EActionSlot SlotID, FActiveActionState& ActiveActionState, int32 InNewCurrentLevel);

	const FAction* GetAction(EActionSlot SlotID, const int32 InLevel) const;
	float GetUniqueValue(EActionSlot SlotID, const FName& InKey);

	UPROPERTY(Transient, VisibleAnywhere, Category = "Components")
	TObjectPtr<class UAOSGameInstance> GameInstance;

	UPROPERTY(Transient)
	TWeakObjectPtr<class UStatComponent> StatComponent;

public:
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Action", meta = (AllowPrivateAccess = "true"))
	FActiveActionState ActiveActionState_Q;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Action", meta = (AllowPrivateAccess = "true"))
	FActiveActionState ActiveActionState_E;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Action", meta = (AllowPrivateAccess = "true"))
	FActiveActionState ActiveActionState_R;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Action", meta = (AllowPrivateAccess = "true"))
	FActiveActionState ActiveActionState_LMB;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Action", meta = (AllowPrivateAccess = "true"))
	FActiveActionState ActiveActionState_RMB;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Action", meta = (AllowPrivateAccess = "true"))
	TArray<FActionAttributes> ActionAttributes_Q;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Action", meta = (AllowPrivateAccess = "true"))
	TArray<FActionAttributes> ActionAttributes_E;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Action", meta = (AllowPrivateAccess = "true"))
	TArray<FActionAttributes> ActionAttributes_R;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Action", meta = (AllowPrivateAccess = "true"))
	TArray<FActionAttributes> ActionAttributes_LMB;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Action", meta = (AllowPrivateAccess = "true"))
	TArray<FActionAttributes> ActionAttributes_RMB;

private:
	TMap<EActionSlot, FTimerHandle> TimerHandles;
	TMap<EActionSlot, FTimerHandle> ReuseTimerHandles;

	TSet<FActiveActionState*> ActiveSlots;

	TArray<float*> ReduceValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", Meta = (AllowPrivateAccess))
	UDataTable* StatTable;
};