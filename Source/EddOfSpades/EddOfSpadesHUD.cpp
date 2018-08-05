// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "EddOfSpadesHUD.h"

void AEddOfSpadesHUD::Tick(float DeltaSeconds)
{

	if(bItemCountChanged)
	{
		bItemCountChanged = false;

		ProgressItemCountChanged();
	}

}

AEddOfSpadesHUD::AEddOfSpadesHUD()
	: NextItemID(0)
	, bItemCountChanged(false)
{

}

int32 AEddOfSpadesHUD::InsertNewProgressItem(FString Description)
{
	NextItemID++;

	FProgressItem NewProgressItem;
	NewProgressItem.Description = Description;
	NewProgressItem.Progress = 0.f;

	ProgressItems.Add(NextItemID, NewProgressItem);

	bItemCountChanged = true;

	return NextItemID;

}

void AEddOfSpadesHUD::UpdateProgressItem(int32 ProgressID, float NewProgress)
{

	FProgressItem* Item = ProgressItems.Find(ProgressID);

	if(Item)
	{
		Item->Progress = NewProgress;
	}

}

void AEddOfSpadesHUD::RemoveProgressItem(int32 ProgressID)
{

	ProgressItems.Remove(ProgressID);
	
	bItemCountChanged = true;


}

void AEddOfSpadesHUD::ProgressItemCountChanged_Implementation()
{

}
