#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase.h"
#include "InputActionValue.h"
#include "AOSCharacterBase.generated.h"

class UActionStatComponent;
class UCharacterRotatorComponent;
class UCharacterDataProviderBase;
class UChampionDataProvider;
class APostProcessVolume;

USTRUCT(BlueprintType)
struct FParticleTransformInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Scale;

	FParticleTransformInfo() : Location(FVector::ZeroVector), Rotation(FRotator::ZeroRotator), Scale(FVector::ZeroVector) {}
	FParticleTransformInfo(FVector InLocation, FRotator InRotation, FVector InScale) : Location(InLocation), Rotation(InRotation), Scale(InScale) {}
};


UCLASS()
class FURYOFLEGENDS_API AAOSCharacterBase : public ACharacterBase
{
	GENERATED_BODY()

	friend class UPlayerAnimInstance;
	friend class UActionStatComponent;

public:
	AAOSCharacterBase();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetWidget(UUserWidgetBase* InUserWidgetBase) override;
	virtual void InitializeCharacterResources() override;

	virtual void PostCharacterSpawn();

	virtual void Move_Started(const FInputActionValue& InValue);
	virtual void Move(const FInputActionValue& InValue) {};
	virtual void Look(const FInputActionValue& InValue) {};

	void InitializeDataStatus();
	void SetDataStatus(EActionSlot InActionSlot, EDataStatus NewStatus);

	UFUNCTION()
	virtual void ExecuteSomethingSpecial() {};

	virtual void DisplayMouseCursor(bool bShowCursor);

public:
	// Getters and Setters
	float GetForwardInputValue() const { return ForwardInputValue; }
	float GetRightInputValue()	 const { return RightInputValue; }
	float GetAimPitchValue()	 const { return CurrentAimPitch; }
	float GetAimYawValue()		 const { return CurrentAimYaw; }

protected:
	void SetWidgetVisibility(bool Visibility);
	void CheckOutOfSight();
	void UpdateOverlayMaterial();
	FHitResult GetImpactPoint(const float TraceRange = 10000.f);
	FHitResult GetSweepImpactPoint(const float TraceRange = 10000.f);
	FHitResult SweepTraceFromAimAngles(const float TraceRange);
	const FName GetAttackMontageSection(const int32& Section);
	
	void UpdateCameraPositionWithAim();
	void ToggleItemShop();
	void SaveCharacterTransform();
	void DisableJump();
	void EnableJump();

	void ProcessActionStarted(EActionSlot InActionSlot);
	void ProcessActionOngoing(EActionSlot InActionSlot);
	void ProcessActionReleased(EActionSlot InActionSlot);
	void HandleActive(EActionSlot Slot, ETriggerEvent TriggerEvent);
	void HandleToggle(EActionSlot Slot, ETriggerEvent TriggerEvent);
	void HandleChanneling(EActionSlot Slot, ETriggerEvent TriggerEvent);
	void HandleCharged(EActionSlot Slot, ETriggerEvent TriggerEvent);
	void HandleTargeted(EActionSlot Slot, ETriggerEvent TriggerEvent);
	void HandleRangedTargeted(EActionSlot Slot, ETriggerEvent TriggerEvent);

public:
	UFUNCTION(Client, Reliable)
	void ClientEnableInput();

	UFUNCTION(Client, Reliable)
	void ClientDisableInput();

	UFUNCTION(Client, Reliable)
	void ClientSetControllerRotationYaw(bool bEnableYawRotation);

	// Ctrl key input handling
	UFUNCTION()
	void HandleCtrlKeyInput(bool bPressed);

	UFUNCTION()
	void OnCtrlKeyPressed();

	UFUNCTION()
	void OnCtrlKeyReleased();

	// Recall
	UFUNCTION()
	void Recall();

	UFUNCTION(Server, Reliable)
	void ServerNotifyActionUse(EActionSlot InActionSlot, ETriggerEvent TriggerEvent, float KeyElapsedTime);

	virtual	void ServerProcessActionUse(EActionSlot InActionSlot, ETriggerEvent TriggerEvent, float KeyElapsedTime);

	UFUNCTION(Server, Reliable)
	void ServerUpgradeAction(EActionSlot InActionSlot);

	// Item usage
	UFUNCTION(Server, Reliable)
	void UseItemSlot(int32 SlotIndex);

public:
	// HUD functions
	UFUNCTION(Server, Unreliable)
	void DecreaseHP_Server();

	UFUNCTION(Server, Unreliable)
	void DecreaseMP_Server();

	UFUNCTION(Server, Unreliable)
	void IncreaseEXP_Server();

	UFUNCTION(Server, Unreliable)
	void IncreaseLevel_Server();

	UFUNCTION(Server, Unreliable)
	void IncreaseCriticalChance_Server();

	UFUNCTION(Server, Unreliable)
	void IncreaseAttackSpeed_Server();

	UFUNCTION(Server, Unreliable)
	void ChangeTeamSide_Server(ETeamSide InTeamSide);

protected:
	// Character state updates
	UFUNCTION(Server, Unreliable)
	void UpdateCharacterState_Server(ECharacterState InCharacterState);

	UFUNCTION(Server, Unreliable)
	void UpdateInputValue_Server(const float& InForwardInputValue, const float& InRightInputValue);

	UFUNCTION(Server, Unreliable)
	void UpdateAimValue_Server(const float& InAimPitchValue, const float& InAimYawValue);

protected:
	UFUNCTION()
	void ActivateHealthRegenTimer();
	UFUNCTION()
	void ActivateManaRegenTimer();
	UFUNCTION()
	void DeactivateHealthRegenTimer();
	UFUNCTION()
	void DeactivateManaRegenTimer();

public:
	// Character death
	UFUNCTION()
	virtual void Respawn(const float RestoreRatio);

	UFUNCTION()
	virtual void OnCharacterDeath();

	UFUNCTION(Client, Reliable)
	void ActivatePostProcessEffect_Client();

	UFUNCTION(Client, Reliable)
	void DeActivatePostProcessEffect_Client();
	
protected:
	UFUNCTION()
	virtual void MontageEnded(UAnimMontage* Montage, bool bInterrupted);

	virtual void RestoreRootedState() override;

	virtual void RestoreSwitchActionState() override;

	UFUNCTION()
	void EnableCharacterMove();

	UFUNCTION()
	void EnableSwitchAction();

	UFUNCTION()
	void EnableUseControllerRotationYaw();

	UFUNCTION()
	void EnableGravity();

	UFUNCTION()
	void DisableGravity();

	UFUNCTION()
	void SpawnLevelUpParticle(int32 OldLevel, int32 NewLevel);

	UFUNCTION()
	virtual void HandleActionNotifyEvent(EActionSlot InActionSlot, int32 EventID) {};

	UFUNCTION(Server, Reliable)
	void ServerUpdateTargetLocation(EActionSlot InActionSlot, const FVector& InTargetLocation);

	UFUNCTION(Server, Reliable)
	void ServerUpdateTarget(EActionSlot InActionSlot, AActor* InTarget);

	UFUNCTION(Server, Reliable)
	void ServerUpdateCameraLocation(const FVector& InCameraLocation);

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<class AArenaPlayerState> ArenaPlayerState;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<class AArenaGameState> ArenaGameState;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<class UParticleSystemComponent> ScreenParticleSystem;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|Components", Meta = (AllowPrivateAccess))
	TArray<class UStaticMeshComponent*> StaticMeshComponents;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<class USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<class UCameraComponent> CameraComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|InputConfig", Meta = (AllowPrivateAccess))
	TObjectPtr<class UEnhancedInputComponent> EnhancedInputComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|InputConfig", Meta = (AllowPrivateAccess))
	TObjectPtr<class UInputConfigData> PlayerCharacterInputConfigData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|InputConfig", Meta = (AllowPrivateAccess))
	TObjectPtr<class UInputMappingContext> PlayerInputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|InputConfig", Meta = (AllowPrivateAccess))
	TObjectPtr<class UInputMappingContext> PlayerCtrlInputMappingContext;

protected:
	// Character state
	UPROPERTY(Replicated, Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Character|Movement", Meta = (AllowPrivateAccess))
	float ForwardInputValue;

	float PreviousForwardInputValue;

	UPROPERTY(Replicated, Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Character|Movement", Meta = (AllowPrivateAccess))
	float RightInputValue;

	float PreviousRightInputValue;

	UPROPERTY(Replicated, Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Character|Movement", Meta = (AllowPrivateAccess))
	float CurrentAimPitch;

	float PreviousAimPitch;

	UPROPERTY(Replicated, Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Character|Movement", Meta = (AllowPrivateAccess))
	float CurrentAimYaw;

	float PreviousAimYaw;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Character|Movement", Meta = (AllowPrivateAccess))
	FVector CurrentCameraLocation;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<class UUW_StateBar> StateBar;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|GamePlay", Meta = (AllowPrivateAccess))
	TObjectPtr<AActor> CurrentTarget;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|GamePlay", Meta = (AllowPrivateAccess))
	TMap<EActionSlot, FVector> TargetLocations;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|GamePlay", Meta = (AllowPrivateAccess))
	UMaterialInterface* OverlayMaterial_Ally;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|GamePlay", Meta = (AllowPrivateAccess))
	UMaterialInterface* OverlayMaterial_Enemy;

protected:
	TMap<EActionSlot, float> KeyInputTimestamps;
	TMap<EActionSlot, float> KeyElapsedTimes;

	APostProcessVolume* PostProcessVolume;
	UMaterialInterface* OriginalMaterial;

	FTimerHandle CheckEnemyOnScreenTimer;
	FTimerHandle TargetMaterialChangeTimer;
	FTimerHandle HealthRegenTimer;
	FTimerHandle ManaRegenTimer;

	FRotator LastCharacterRotation;
	FVector LastCharacterLocation;
	FVector LastForwardVector;
	FVector LastRightVector;
	FVector LastUpVector;

	bool bCtrlKeyPressed;

	int32 SelectedCharacterIndex;
	
	float BasicAttackAnimPlayRate;
	float BasicAttackAnimLength;
};
