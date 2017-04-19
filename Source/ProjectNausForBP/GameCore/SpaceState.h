// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameStateBase.h"

#include "ProjectNausForBP.h"
#include "../GameCoreSub/SafeENGINE.h"
#include "../GameCoreSub/SaveLoader.h"

#include "../Interface/Commandable.h"
#include "../Interface/Structureable.h"
#include "../Interface/Collectable.h"

#include "../GameCoreSub/Enums.h"
#include "SpaceState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNAUSFORBP_API ASpaceState : public AGameState {
	GENERATED_BODY()
public:
	ASpaceState();
#pragma region Event Call
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
#pragma endregion

#pragma region Save/Load
public:
	UFUNCTION(BlueprintCallable, Category = "Call Function : State")
		bool SaveSpaceState(USaveLoader* saver);
	UFUNCTION(BlueprintCallable, Category = "Call Function : State")
		bool LoadSpaceState(USaveLoader* loader);
#pragma endregion

#pragma region Get Data Or SectorData Processing
	UFUNCTION(BlueprintCallable, Category = "Call Function : State")
		Peer PeerIdentify(Faction requestor, Faction target);
	UFUNCTION(BlueprintCallable, Category = "Call Function : State")
		void GetCurrentSectorInfo(FSectorData& getSectorInfo);
	UFUNCTION(BlueprintCallable, Category = "Call Function : State")
		bool isValidSector(FString& checkSectorName);
#pragma endregion

#pragma region ObjectTMap
protected:
	UPROPERTY()
		TArray<FSectorData> sectorInfo;
	FSectorData* currentSectorInfo;
public:
	UPROPERTY()
		TArray<float> factionRelation;
#pragma endregion

#pragma region Respawn Ship
	void AccumulateToShipCapacity(bool isDestroying);

	int shipRegenLimit;
	int currentShipCapacity;
	bool skipingFirstTick;

	int totalChanceFactor;
	int shipRegenAmount;
	int randomResultForRegen;
	int randomGateForRegenLocation;
};
