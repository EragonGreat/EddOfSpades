// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "IGHUD.h"

void AIGHUD::Tick(float DeltaSeconds)
{

	if(bItemCountChanged)
	{
		bItemCountChanged = false;

		ProgressItemCountChanged();
	}

}

AIGHUD::AIGHUD()
	: NextItemID(0)
	, bItemCountChanged(false)
{

}

int32 AIGHUD::InsertNewProgressItem(FString Description)
{
	NextItemID++;

	FProgressItem NewProgressItem;
	NewProgressItem.Description = Description;
	NewProgressItem.Progress = 0.f;

	ProgressItems.Add(NextItemID, NewProgressItem);

	bItemCountChanged = true;

	return NextItemID;

}

void AIGHUD::UpdateProgressItem(int32 ProgressID, float NewProgress)
{

	FProgressItem* Item = ProgressItems.Find(ProgressID);

	if(Item)
	{
		Item->Progress = NewProgress;
	}

}

void AIGHUD::RemoveProgressItem(int32 ProgressID)
{

	ProgressItems.Remove(ProgressID);
	
	bItemCountChanged = true;


}

void AIGHUD::ProgressItemCountChanged_Implementation()
{

}
