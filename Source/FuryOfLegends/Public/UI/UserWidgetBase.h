// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UserWidgetBase.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UUserWidgetBase : public UUserWidget
{
	GENERATED_BODY()
	
public:
    AActor* GetOwningActor() const { return OwningActor; }

    void SetOwningActor(AActor* InOwningActor) { OwningActor = InOwningActor; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UserWidgetBase")
    TObjectPtr<AActor> OwningActor;
};
