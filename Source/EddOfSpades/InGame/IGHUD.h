// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "IGHUD.generated.h"

USTRUCT(BlueprintType)
struct EDDOFSPADES_API FProgressItem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FString Description;

	UPROPERTY(BlueprintReadOnly)
	float Progress;

};

UCLASS(Blueprintable)
class AIGHUD : public AHUD
{
	GENERATED_BODY()

private:
	int32 NextItemID;
	bool bItemCountChanged;

public:
	UPROPERTY(BlueprintReadOnly)
	TMap<int32, FProgressItem> ProgressItems;
	

	virtual void Tick(float DeltaSeconds) override;

public:
	AIGHUD();

	int32 InsertNewProgressItem(FString Description);
	void UpdateProgressItem(int32 ProgressID, float NewProgress);
	void RemoveProgressItem(int32 ProgressID);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, meta=(DisplayName = "Progress Item Count Changed"))
	void ProgressItemCountChanged();

};

