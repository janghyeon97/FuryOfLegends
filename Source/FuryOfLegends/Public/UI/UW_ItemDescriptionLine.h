#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_ItemDescriptionLine.generated.h"

UCLASS()
class FURYOFLEGENDS_API UUW_ItemDescriptionLine : public UUserWidget
{
    GENERATED_BODY()

public:
    UUW_ItemDescriptionLine(const FObjectInitializer& ObjectInitializer);
    void SetDescriptionText(const FString& Text);

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class URichTextBlock> DescriptionRichTextBlock;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Style")
    TObjectPtr<class UDataTable> RichTextStyleSet;
};
