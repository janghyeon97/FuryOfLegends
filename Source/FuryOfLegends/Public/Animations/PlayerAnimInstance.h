// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

DECLARE_DELEGATE(FOnEnableSwitchActionNotifyBeginDelegate);
DECLARE_DELEGATE(FOnEnableMoveNotifyBeginDelegate);
DECLARE_DELEGATE(FOnCheckComboAttackNotifyBeginDelegate);
DECLARE_DELEGATE(FOnStopBasicAttackNotifyBeginDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnableGravityNotifyBeginDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDisableGravityNotifyBeginDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionNotifyTriggeredDelegate, EActionSlot, SlotID, int32, EventID);

UENUM(BlueprintType)
enum class EDirection : uint8
{
    None        UMETA(Hidden),
    Forward     UMETA(DisplayName = "Forward"),
    Backward    UMETA(DisplayName = "Backward"),            
    Right       UMETA(DisplayName = "Right"),            
    Left        UMETA(DisplayName = "Left"), 
};

USTRUCT(BlueprintType)
struct FDirectionSettings
{
    GENERATED_BODY()

public:
    FDirectionSettings()
        : ForwardMin(0)
        , ForwardMax(0)
        , BackwardMin(0)
        , BackwardMax(0)
        , DeadZone(0)
    {
    };

    FDirectionSettings(float InForwardMin, float InForwardMax, float InBackwardMin, float InBackwardMax, float InDeadZone)
        : ForwardMin(InForwardMin)
        , ForwardMax(InForwardMax)
        , BackwardMin(InBackwardMin)
        , BackwardMax(InBackwardMax)
        , DeadZone(InDeadZone)
    {
    };

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ForwardMin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ForwardMax;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BackwardMin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BackwardMax;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DeadZone;
};

UENUM(BlueprintType, meta = (Bitflags))
enum class EBaseCharacterState : uint8
{
    None    = 0      UMETA(DisplayName = "None"),
    Q       = 1 << 0 UMETA(DisplayName = "Q"),                  // 00000001
    E       = 1 << 1 UMETA(DisplayName = "E"),                  // 00000010
    R       = 1 << 2 UMETA(DisplayName = "R"),                  // 00000100
    LMB     = 1 << 3 UMETA(DisplayName = "Left Mouse Button"),  // 00001000
    RMB     = 1 << 4 UMETA(DisplayName = "Right Mouse Button")  // 00010000
};

UCLASS()
class FURYOFLEGENDS_API UPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimInstance();

    // Public methods
    void EnableTurnLeft();
    void EnableTurnRight();
    void OnTurnInPlaceEnded();
    void SetRootYawOffset(float DeltaSeconds);
    void SetTurningTime();
    void TurnInPlace(float DeltaSeconds);

    void UpdateDeltaRotation();
    void SetTargetYawAndAnimLength(float AbsRootYawOffset);

    void SetTurnLeft(bool InTurnLeft) { bCanTurnLeft = InTurnLeft; }
    void SetTurnRight(bool InTurnRight) { bCanTurnRight = InTurnRight; }

    UFUNCTION(BlueprintPure, Category = "Animation", meta = (BlueprintThreadSafe))
    bool IsCharacterStateActive(EBaseCharacterState State) const;

    // Delegates
    FOnCheckComboAttackNotifyBeginDelegate OnCheckComboAttackNotifyBegin;
    FOnStopBasicAttackNotifyBeginDelegate OnStopBasicAttackNotifyBegin;
    FOnEnableMoveNotifyBeginDelegate OnEnableMoveNotifyBegin;
    FOnEnableSwitchActionNotifyBeginDelegate OnEnableSwitchActionNotifyBegin;
    FOnEnableGravityNotifyBeginDelegate OnEnableGravityNotifyBegin;
    FOnDisableGravityNotifyBeginDelegate OnDisableGravityNotifyBegin;
    FOnActionNotifyTriggeredDelegate OnActionNotifyTriggered;

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void NativeThreadSafeUpdateAnimation(float _DeltaSeconds) override;
    virtual void NativePostEvaluateAnimation() override;

public:
    // AnimNotify functions
    UFUNCTION()
    void AnimNotify_SlotEvent(EActionSlot SlotID, int32 EventID);

    UFUNCTION()
    void AnimNotify_CheckHit_Q();

    UFUNCTION()
    void AnimNotify_CheckHit_E();

    UFUNCTION()
    void AnimNotify_CheckHit_R();

    UFUNCTION()
    void AnimNotify_CheckHit_LMB();

    UFUNCTION()
    void AnimNotify_CheckHit_RMB();

    UFUNCTION()
    void AnimNotify_CanNextCombo();

    UFUNCTION()
    void AnimNotify_CanNextAction();

    UFUNCTION()
    void AnimNotify_CanMove();

    UFUNCTION()
    void AnimNotify_WeakAttackEnd();

    UFUNCTION()
    void AnimNotify_DisableGravity();

    UFUNCTION()
    void AnimNotify_EnableGravity();

protected:
    UFUNCTION(BlueprintPure, Category = "Character|ThreadSafe", meta = (BlueprintThreadSafe))
    class UCharacterMovementComponent* GetMovementComponent() const;

    UFUNCTION(BlueprintPure, Category = "Character|ThreadSafe", meta = (BlueprintThreadSafe))
    EDirection CalculateLocomotionDirection(const float InAngle) const;

protected:
    // Character References
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character", Meta = (AllowPrivateAccess))
    TObjectPtr<class AAOSCharacterBase> OwnerCharacter;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character", Meta = (AllowPrivateAccess))
    TObjectPtr<class UCharacterMovementComponent> MovementComponent;

    // Character Movement State
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Anim", meta = (AllowPrivateAccess))
    FVector MoveInputWithMaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Anim", meta = (AllowPrivateAccess))
    FVector MoveInput;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Anim", Meta = (AllowPrivateAccess))
    float GroundSpeed;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Anim", Meta = (AllowPrivateAccess))
    float YawOffset;

    // Character Aim Offset
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Anim", Meta = (AllowPrivateAccess))
    FRotator BaseAimRotation;

    // Character Turn In Place
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|Anim", Meta = (AllowPrivateAccess))
    float RootYawOffset;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Anim", Meta = (AllowPrivateAccess))
    bool bCanTurnLeft;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Anim", Meta = (AllowPrivateAccess))
    bool bCanTurnRight;

    // Character States
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Anim", Meta = (AllowPrivateAccess))
    uint8 bIsFalling : 1;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Anim", Meta = (AllowPrivateAccess))
    uint8 bIsDead : 1;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Anim", Meta = (AllowPrivateAccess))
    bool bIsAbilityRActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|ThreadSafe", Meta = (AllowPrivateAccess))
    EBaseCharacterState CharacterState = EBaseCharacterState::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|ThreadSafe", Meta = (AllowPrivateAccess))
    EDirection Direction;

    /* ------------------------------------------------------------------------------------------------------------ */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|ThreadSafe", Meta = (AllowPrivateAccess))
    FVector CurrentWorldLocation;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|ThreadSafe", Meta = (AllowPrivateAccess))
    FRotator CurrentWorldRotation;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|ThreadSafe", Meta = (AllowPrivateAccess))
    FVector Acceleration;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|ThreadSafe", Meta = (AllowPrivateAccess))
    FVector Acceleration2D;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|ThreadSafe", Meta = (AllowPrivateAccess))
    bool bHasAcceleration; 

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|ThreadSafe", Meta = (AllowPrivateAccess))
    FVector Velocity;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|ThreadSafe", Meta = (AllowPrivateAccess))
    FVector Velocity2D;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|ThreadSafe", Meta = (AllowPrivateAccess))
    float LocomotionAngle;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|ThreadSafe", Meta = (AllowPrivateAccess))
    float DisplacementSinceLastUpdate;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|ThreadSafe", Meta = (AllowPrivateAccess))
    float DisplacementSpeed;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|ThreadSafe", Meta = (AllowPrivateAccess))
    float SpeedAccelDotProduct;

    // Rotation and Timing Variables
    FRotator MovingRotation = FRotator::ZeroRotator;
    FRotator LastMovingRotation = FRotator::ZeroRotator;
    FRotator DeltaRotation = FRotator::ZeroRotator;
    FRotator LastDeltaRotation = FRotator::ZeroRotator;

    float LastInterpRotation = 0.0f;
    float TurningTimer = 0.0f;
    float RotationTimer = 0.0f;
    float TargetYaw = 0.0f;
    float AnimLength = 0.0f;

    bool bControlRotationStopped = false;
    bool bCanTurnInPlace = false;

    FTimerHandle TimerHandle;
};
