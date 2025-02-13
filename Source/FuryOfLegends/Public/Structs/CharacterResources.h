// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CharacterResources.generated.h"


USTRUCT(BlueprintType)
struct FCharacterAnimationAttribute
{
	GENERATED_BODY()

public:
	FCharacterAnimationAttribute()
		: Key(NAME_None)
		, Value(nullptr)
	{
	}

	FCharacterAnimationAttribute(FName InKey, UAnimMontage* InValue)
		: Key(InKey)
		, Value(InValue)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* Value;
};

USTRUCT(BlueprintType)
struct FCharacterParticleEffectAttribute
{
	GENERATED_BODY()

public:
	FCharacterParticleEffectAttribute()
		: Key(NAME_None)
		, Value(nullptr)
	{
	}

	FCharacterParticleEffectAttribute(FName InKey, UParticleSystem* InValue)
		: Key(InKey)
		, Value(InValue)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* Value;
};

USTRUCT(BlueprintType)
struct FCharacterStaticMeshAttribute
{
	GENERATED_BODY()

public:
	FCharacterStaticMeshAttribute()
		: Key(NAME_None)
		, Value(nullptr)
	{
	}

	FCharacterStaticMeshAttribute(FName InKey, UStaticMesh* InValue)
		: Key(InKey)
		, Value(InValue)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* Value;
};

USTRUCT(BlueprintType)
struct FCharacterMaterialAttribute
{
	GENERATED_BODY()

public:
	FCharacterMaterialAttribute()
		: Key(NAME_None)
		, Value(nullptr)
	{
	}

	FCharacterMaterialAttribute(FName InKey, UMaterialInstance* InValue)
		: Key(InKey)
		, Value(InValue)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstance* Value;
};

USTRUCT(BlueprintType)
struct FCharacterTexutreAttribute
{
	GENERATED_BODY()

public:
	FCharacterTexutreAttribute()
		: Key(NAME_None)
		, Value(nullptr)
	{
	}

	FCharacterTexutreAttribute(FName InKey, UTexture* InValue)
		: Key(InKey)
		, Value(InValue)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture* Value;
};

USTRUCT(BlueprintType)
struct FCharacterClassAttribute
{
	GENERATED_BODY()

public:
	FCharacterClassAttribute()
		: Key(NAME_None)
		, Value(nullptr)
	{
	}

	FCharacterClassAttribute(FName InKey, TObjectPtr<UClass> InValue)
		: Key(InKey)
		, Value(nullptr)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UClass> Value;
};

USTRUCT(BlueprintType)
struct FCharacterGamePlayDataRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FCharacterGamePlayDataRow()
		: GameplayMontages(TArray<FCharacterAnimationAttribute>())
		, GameplayParticles(TArray<FCharacterParticleEffectAttribute>())
		, GameplayMeshes(TArray<FCharacterStaticMeshAttribute>())
		, GameplayMaterials(TArray<FCharacterMaterialAttribute>())
		, GameplayClasses(TArray<FCharacterClassAttribute>())
		, GameplayTextures(TArray<FCharacterTexutreAttribute>())
	{
	}

	TMap<FName, UAnimMontage*> GetGamePlayMontagesMap() const
	{
		return GetGamePlayAttributesMap<FCharacterAnimationAttribute, UAnimMontage*>(GameplayMontages);
	}

	TMap<FName, UParticleSystem*> GetGamePlayParticlesMap() const
	{
		return GetGamePlayAttributesMap<FCharacterParticleEffectAttribute, UParticleSystem*>(GameplayParticles);
	}

	TMap<FName, UStaticMesh*> GetGamePlayMeshesMap() const
	{
		return GetGamePlayAttributesMap<FCharacterStaticMeshAttribute, UStaticMesh*>(GameplayMeshes);
	}

	TMap<FName, UMaterialInstance*> GetGamePlayMaterialsMap() const
	{
		return GetGamePlayAttributesMap<FCharacterMaterialAttribute, UMaterialInstance*>(GameplayMaterials);
	}

	TMap<FName, UClass*> GetGamePlayClassesMap() const
	{
		return GetGamePlayAttributesMap<FCharacterClassAttribute, UClass*>(GameplayClasses);
	}

	TMap<FName, UTexture*> GetGamePlayTexturesMap() const
	{
		return GetGamePlayAttributesMap<FCharacterTexutreAttribute, UTexture*>(GameplayTextures);
	}


	template <typename AttributeType, typename ValueType>
	TMap<FName, ValueType> GetGamePlayAttributesMap(const TArray<AttributeType>& AttributesArray) const
	{
		TMap<FName, ValueType> AttributesMap;
		for (const AttributeType& Attribute : AttributesArray)
		{
			if (!Attribute.Key.IsNone())
			{
				AttributesMap.Add(Attribute.Key, Attribute.Value);
			}
		}
		return AttributesMap;
	}


public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	TArray<FCharacterAnimationAttribute> GameplayMontages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	TArray<FCharacterParticleEffectAttribute> GameplayParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	TArray<FCharacterStaticMeshAttribute> GameplayMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	TArray<FCharacterMaterialAttribute> GameplayMaterials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	TArray<FCharacterClassAttribute> GameplayClasses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	TArray<FCharacterTexutreAttribute> GameplayTextures;
};




USTRUCT(BlueprintType)
struct FGameplayParticleAttribute
{
	GENERATED_BODY()

public:
	FGameplayParticleAttribute()
		: Key(NAME_None)
		, Value(nullptr)
	{
	}

	FGameplayParticleAttribute(FName InKey, float InValue)
		: Key(InKey)
		, Value(nullptr)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* Value;
};

USTRUCT(BlueprintType)
struct FSharedGameplay : public FTableRowBase
{
	GENERATED_BODY()

public:
	FSharedGameplay()
		: SharedGameplayParticles(TArray<FGameplayParticleAttribute>())
	{

	}

	UParticleSystem* GetSharedGamePlayParticle(const FName& InKey) const
	{
		for (const FGameplayParticleAttribute& Attribute : SharedGameplayParticles)
		{
			if (Attribute.Key.IsEqual(InKey))
			{
				return Attribute.Value;
			}
		}

		return nullptr;
	}

	TMap<FName, UParticleSystem*> GetSharedGamePlayParticlesMap() const
	{
		TMap<FName, UParticleSystem*> AttributesMap;
		for (const FGameplayParticleAttribute& Attribute : SharedGameplayParticles)
		{
			if (Attribute.Key.IsNone() == false)
			{
				AttributesMap.Add(Attribute.Key, Attribute.Value);
			}
		}
		return AttributesMap;
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	TArray<FGameplayParticleAttribute> SharedGameplayParticles;
};


/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UCharacterResources : public UObject
{
	GENERATED_BODY()
	
};
