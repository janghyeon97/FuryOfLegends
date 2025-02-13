// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_BuffListEntry.h"
#include "Components/Image.h"
#include "Components/Border.h"

void UUW_BuffListEntry::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	MaterialRef = Image->GetDynamicMaterial();
}

void UUW_BuffListEntry::NativeConstruct()
{
	Super::NativeConstruct();

}

void UUW_BuffListEntry::UpdateImage(UTexture* NewTexure)
{
	if (MaterialRef != nullptr && NewTexure != nullptr)
	{
		MaterialRef->SetTextureParameterValue(FName("Texture"), NewTexure);
	}
}

void UUW_BuffListEntry::UpdateBorderColor(FLinearColor NewColor)
{
    if (!Border)
    {
        UE_LOG(LogTemp, Error, TEXT("Border widget is null in %s"), *GetName());
        return;
    }

    // 현재 색상과 새로 설정할 색상이 다를 경우에만 업데이트
    if (Border->GetColorAndOpacity() != NewColor)
    {
        Border->SetColorAndOpacity(NewColor);
    }
}

void UUW_BuffListEntry::UpdateCooldownMaskOpacity(const float NewOpacity)
{
    if (MaterialRef != nullptr)
    {
        MaterialRef->SetScalarParameterValue(FName("Param"), NewOpacity);
    }
}

void UUW_BuffListEntry::UpdateCooldownPercent(const float NewPercent)
{
    if (!MaterialRef)
    {
        UE_LOG(LogTemp, Error, TEXT("[UUW_BuffListEntry] MaterialRef is null in %s"), *GetName());
        return;
    }

    MaterialRef->SetScalarParameterValue(FName("Percent"), NewPercent);
}
