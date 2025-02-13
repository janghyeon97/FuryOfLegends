// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Structs/CustomCombatData.h"
#include "Structs/CharacterData.h"
#include "Structs/ActionData.h"
#include "CharacterBase.generated.h"

class UCrowdControlEffect;
class UCrowdControlManager;
class UUserWidgetBase;
class UWidgetComponent;
class UDamageNumberWidget;
class UCharacterWidgetComponent;
class UStatComponent;
class UActionStatComponent;
class UAnimInstance;
class UInputComponent;
class UNavArea;

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnPreApplyDamageDelegate, AActor*, DamageReceiver, AActor*, DamageCauser, AController*, InstigatorActor, FDamageInformation&, DamageInformation);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReceiveDamageEnteredDelegate, bool&, bResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnPreReceiveDamageDelegate, AActor*, DamageReceiver, AActor*, DamageCauser, AController*, InstigatorActor, FDamageInformation&, DamageInformation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnPostReceiveDamageDelegate, AActor*, DamageReceiver, AActor*, DamageCauser, AController*, InstigatorActor, FDamageInformation&, DamageInformation);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPreDeathDelegate, bool&, bDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPostDeathDelegate, AActor*, Character, AActor*, Eliminator);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReceiveCrowdControlEnteredDelegate, bool&, bResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnPreReceiveCrowdControlDelegate, AActor*, DamageReceiver, AActor*, DamageCauser, AController*, InstigatorActor, FCrowdControlInformation&, CrowdControlInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnPostReceiveCrowdControlDelegate, AActor*, DamageReceiver, AActor*, DamageCauser, AController*, InstigatorActor, FCrowdControlInformation&, CrowdControlInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnPostApplyCrowdControlDelegate, bool, bResult, AActor*, DamageReceiver, AActor*, DamageCauser, AController*, InstigatorActor, FCrowdControlInformation&, CrowdControlInfo);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitEventTriggeredDelegate, FDamageInformation&, DamageInformation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackEventTriggeredDelegate, FDamageInformation&, DamageInformation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityEffectsEventTriggeredDelegate, FDamageInformation&, DamageInformation);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActionEndedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRootedStateEndedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSwitchActionStateEndedDelegate);

UENUM(BlueprintType)
enum class ECharacterStateOperation : uint8
{
	Add,
	Remove
};

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EDataStatus : uint8
{
	Ready		= 0,       // 모든 데이터가 준비된 상태를 의미 (아무 플래그도 설정되지 않음)
	Position	= 1 << 0,  // 위치 정보 필요
	Rotation	= 1 << 1,  // 회전 정보 필요
	Target		= 1 << 2   // 타켓 정보 필요
};
ENUM_CLASS_FLAGS(EDataStatus)


UCLASS()
class FURYOFLEGENDS_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ACharacterBase();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Initialization and Resource Functions
	virtual void InitializeCharacterResources();
	virtual void SetWidget(UUserWidgetBase* InUserWidgetBase);

	virtual UAnimMontage* GetOrLoadMontage(const FName& Key, const TCHAR* Path);
	virtual UParticleSystem* GetOrLoadParticle(const FName& Key, const TCHAR* Path);
	virtual UStaticMesh* GetOrLoadMesh(const FName& Key, const TCHAR* Path);
	virtual UMaterialInstance* GetOrLoadMaterial(const FName& Key, const TCHAR* Path);
	virtual UClass* GetOrLoadClass(const FName& Key, const TCHAR* Path);
	virtual UTexture* GetOrLoadTexture(const FName& Key, const TCHAR* Path);
	virtual UParticleSystem* GetOrLoadSharedParticle(const FName& Key, const TCHAR* Path);

	template<typename T>
	T* GetOrLoadResource(TMap<FName, T*>& ResourceMap, const FName& Key, const TCHAR* Path);

	float GetUniqueAttribute(EActionSlot SlotID, const FName& Key, float DefaultValue) const;
	float AdjustAnimPlayRate(const float AnimLength);

	UStatComponent* GetStatComponent() const;
	UActionStatComponent* GetActionStatComponent() const;

	FName GetCharacterName() const { return CharacterName; };

	virtual void ChangeNavModifierAreaClass(TSubclassOf<UNavArea> NewAreaClass) {};

	virtual void RotateWidgetToLocalPlayer();

	// ---------------   Damage-related Functions on Server   --------------- 

	UFUNCTION(Server, Reliable)
	void ServerApplyDamage(ACharacterBase* Enemy, AActor* DamageCauser, AController* EventInstigator, FDamageInformation DamageInformation);

	UFUNCTION(Server, Reliable)
	void ServerApplyCrowdControl(ACharacterBase* Enemy, AActor* DamageCauser, AController* EventInstigator, FCrowdControlInformation CrowdControlInfo);

protected:
	virtual bool ReceiveDamage(AActor* DamageCauser, AController* EventInstigator, FDamageInformation DamageInformation);

	virtual void ProcessDamageCauser(AActor* DamageCauser);

	virtual bool ReceiveCrowdControl(AActor* DamageCauser, AController* EventInstigator, FCrowdControlInformation CrowdControlInfo);

	UFUNCTION()
	virtual void ProcessCriticalHit(FDamageInformation& DamageInformation);

	UFUNCTION()
	virtual void OnPreCalculateDamage(float& AttackDamage, float& AbilityPower);

	UFUNCTION()
	virtual void OnPreDamageReceived(float FinalDamage);

public:
	// Particle and mesh spawning
	UFUNCTION()
	UParticleSystemComponent* SpawnEmitterAtLocation(UParticleSystem* Particle, FTransform Transform, bool bAutoDestory = true, EPSCPoolMethod PoolingMethod = EPSCPoolMethod::None, bool bAutoActivate = true);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpawnEmitterAtLocation(UParticleSystem* Particle, FTransform Transform, bool bAutoDestory = true, EPSCPoolMethod PoolingMethod = EPSCPoolMethod::None, bool bAutoActivate = true);

	UFUNCTION(Server, Reliable)
	void ServerSpawnEmitterAttached(UParticleSystem* Particle, USceneComponent* AttachToComponent, FTransform Transform, EAttachLocation::Type LocationType);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpawnEmitterAttached(UParticleSystem* Particle, USceneComponent* AttachToComponent, FTransform Transform, EAttachLocation::Type LocationType);

	UFUNCTION(Server, Reliable)
	void ServerSpawnMeshAttached(UStaticMesh* MeshToSpawn, USceneComponent* AttachToComponent, float Duration);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpawnMeshAttached(UStaticMesh* MeshToSpawn, USceneComponent* AttachToComponent, float Duration);

	UFUNCTION(Server, Reliable)
	void ServerSpawnActorAtLocation(UClass* SpawnActor, FTransform SpawnTransform);

	UFUNCTION(Client, Reliable)
	void ClientSpawnDamageWidget(AActor* Target, const float DamageAmount, const FDamageInformation& DamageInformation);

	// Montage-related functions
	void PlayMontage(const FString& MontageName, float PlayRate = 1.0f, FName StartSectionName = NAME_None, const TCHAR* Path = nullptr);

	UFUNCTION(Server, Reliable)
	void ServerPlayMontage(UAnimMontage* Montage, float PlayRate = 1.0f, FName StartSectionName = NAME_None, bool bApplyToLocalPlayer = false);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayMontage(UAnimMontage* Montage, float PlayRate = 1.0f, FName StartSectionName = NAME_None, bool bApplyToLocalPlayer = false);

	UFUNCTION(Server, Reliable)
	void ServerStopAllMontages(float BlendOut, bool bApplyToLocalPlayer = false);

	UFUNCTION(Server, Reliable)
	void ServerStopMontage(float BlendOut, UAnimMontage* Montage, bool bApplyToLocalPlayer = false);

	UFUNCTION(Server, Reliable)
	void MulticastStopMontage(float BlendOut, UAnimMontage* Montage, bool bApplyToLocalPlayer = false);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStopAllMontages(float BlendOut, bool bApplyToLocalPlayer = false);

	UFUNCTION(Server, Reliable)
	void ServerMontageJumpToSection(const UAnimMontage* Montage, FName SectionName, bool bApplyToLocalPlayer = false);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastMontageJumpToSection(const UAnimMontage* Montage, FName SectionName, bool bApplyToLocalPlayer = false);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPauseMontage();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastResumeMontage();

	UFUNCTION(Client, Reliable)
	void ClientSetControlRotation(FRotator NewRotation);

	UFUNCTION(Server, Reliable)
	void ServerModifyCharacterState(ECharacterStateOperation Operation, ECharacterState StateFlag);

protected:
	// Delegate Functions
	UFUNCTION()
	virtual void OnRep_CharacterStateChanged();

	UFUNCTION()
	virtual void OnRep_CrowdControlStateChanged();

	UFUNCTION()
	void OnMovementSpeedChanged(float InOldMS, float InNewMS);

	UFUNCTION()
	void OnDamageWidgetAnimationFinished();

public:
	// Ability Start Functions
	UFUNCTION()
	virtual void Q_Started() {};
	UFUNCTION()
	virtual void E_Started() {};
	UFUNCTION()
	virtual void R_Started() {};
	UFUNCTION()
	virtual void LMB_Started() {};
	UFUNCTION()
	virtual void RMB_Started() {};

	void ActionStarted(EActionSlot SlotID);

	// Ability Ongoing Functions
	UFUNCTION()
	virtual void Q_Ongoing() {};
	UFUNCTION()
	virtual void E_Ongoing() {};
	UFUNCTION()
	virtual void R_Ongoing() {};
	UFUNCTION()
	virtual void LMB_Ongoing() {};
	UFUNCTION()
	virtual void RMB_Ongoing() {};

	// Ability Released Functions
	UFUNCTION()
	virtual void Q_Released() {};
	UFUNCTION()
	virtual void E_Released() {};
	UFUNCTION()
	virtual void R_Released() {};
	UFUNCTION()
	virtual void LMB_Released() {};
	UFUNCTION()
	virtual void RMB_Released() {};

	void ActionReleased(EActionSlot SlotID);

	// Ability Execution Functions
	UFUNCTION()
	virtual void Q_Executed() {};
	UFUNCTION()
	virtual void E_Executed() {};
	UFUNCTION()
	virtual void R_Executed() {};
	UFUNCTION()
	virtual void LMB_Executed() {};
	UFUNCTION()
	virtual void RMB_Executed() {};

	void ExecuteAction(EActionSlot SlotID);

	// Ability Cancel Functions
	UFUNCTION()
	virtual void RestoreRootedState() {};
	UFUNCTION()
	virtual void RestoreSwitchActionState() {};

	UFUNCTION()
	virtual void CancelAction() {};
	UFUNCTION()
	virtual void Q_Canceled() {};
	UFUNCTION()
	virtual void E_Canceled() {};
	UFUNCTION()
	virtual void R_Canceled() {};
	UFUNCTION()
	virtual void LMB_Canceled() {};
	UFUNCTION()
	virtual void RMB_Canceled() {};

	// Ability Check Functions
	UFUNCTION()
	virtual void Q_CheckHit() {};
	UFUNCTION()
	virtual void E_CheckHit() {};
	UFUNCTION()
	virtual void R_CheckHit() {};
	UFUNCTION()
	virtual void LMB_CheckHit() {};
	UFUNCTION()
	virtual void RMB_CheckHit() {};

public:
	FOnPreApplyDamageDelegate OnPreApplyDamageEvent;
	FOnReceiveDamageEnteredDelegate OnReceiveDamageEnteredEvent;
	FOnPreReceiveDamageDelegate OnPreReceiveDamageEvent;
	FOnPostReceiveDamageDelegate OnPostReceiveDamageEvent;

	FOnHitEventTriggeredDelegate OnHitEventTriggered;
	FOnAttackEventTriggeredDelegate OnAttackEventTriggered;
	FOnAbilityEffectsEventTriggeredDelegate OnAbilityEffectsEventTriggered;

	FOnActionEndedDelegate OnActionEnded;
	FOnRootedStateEndedDelegate OnRootedStateEnded;
	FOnSwitchActionStateEndedDelegate OnSwitchActionStateEnded;

	FOnPreDeathDelegate OnPreDeathEvent;
	FOnPostDeathDelegate OnPostDeathEvent;

	FOnReceiveCrowdControlEnteredDelegate OnReceiveCrowdControlEnteredEvent;
	FOnPreReceiveCrowdControlDelegate OnPreReceiveCrowdControlEvent;
	FOnPostReceiveCrowdControlDelegate OnPostReceiveCrowdControlEvent;
	FOnPostApplyCrowdControlDelegate OnPostApplyCrowdControlEvent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Homing", Meta = (AllowPrivateAccess = "true"))
	USceneComponent* HomingTargetSceneComponent;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<UAnimInstance> AnimInstance;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<UCharacterWidgetComponent> WidgetComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<UStatComponent> StatComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<UActionStatComponent> ActionStatComponent;

public:
	UPROPERTY(ReplicatedUsing = OnRep_CharacterStateChanged, Transient, VisibleAnywhere, Category = "Character|State")
	ECharacterState CharacterState;

	UPROPERTY(ReplicatedUsing = OnRep_CrowdControlStateChanged, Transient, VisibleAnywhere, Category = "Character|State")
	ECrowdControl CrowdControlState;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Character|GamePlay")
	ETeamSide TeamSide;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Character|GamePlay")
	EObjectType ObjectType;

public: // 서버에서 처리
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|GamePlay", meta = (AllowPrivateAccess))
	UCrowdControlManager* CrowdControlManager;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|GamePlay", Meta = (AllowPrivateAccess))
	TMap<ECrowdControl, UCrowdControlEffect*> ActiveEffects;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|GamePlay", Meta = (AllowPrivateAccess))
	TObjectPtr<AActor> LastHitCharacter;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|GamePlay", Meta = (AllowPrivateAccess))
	TMap<AActor*, float> PlayerHitTimestamps;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|GamePlay", Meta = (AllowPrivateAccess))
	int32 TotalAttacks;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|GamePlay", Meta = (AllowPrivateAccess))
	int32 CriticalHits;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|GamePlay", Meta = (AllowPrivateAccess))
	uint8 ComboCount;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|GamePlay", Meta = (AllowPrivateAccess))
	uint8 MaxComboCount;

protected:
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Character|State")
	TMap<ECharacterState, int32> CharacterStateReferenceCount;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|State", Meta = (AllowPrivateAccess))
	TMap<ECrowdControl, int32> ControlStateReferenceCount;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|GamePlay", Meta = (AllowPrivateAccess))
	FName CharacterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Widget", Meta = (AllowPrivateAccess))
	TSubclassOf<UDamageNumberWidget> DamageNumberWidgetClass;

	// 위젯 컴포넌트들을 관리할 큐
	TQueue<UWidgetComponent*> DamageWidgetQueue;

	UPROPERTY()
	TMap<EActionSlot, EDataStatus> DataStatus;
	TMap<EActionSlot, EDataStatus> DefaultDataStatus;

public:
	static TMap<FName, UParticleSystem*> SharedGameplayParticles;
	TMap<FName, UAnimMontage*> GameplayMontages;
	TMap<FName, UParticleSystem*> GameplayParticles;
	TMap<FName, UStaticMesh*> GameplayMeshes;
	TMap<FName, UMaterialInstance*> GameplayMaterials;
	TMap<FName, UClass*> GameplayClasses;
	TMap<FName, UTexture*> GameplayTextures;
};