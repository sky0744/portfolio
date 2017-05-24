﻿// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectNausForBP.h"
#include "Resource.h"

AResource::AResource()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;
	PrimaryActorTick.bTickEvenWhenPaused = false;
	PrimaryActorTick.TickInterval = 0.0f;
	resourceID = -1;
}

#pragma region Event Calls
void AResource::BeginPlay()
{
	Super::BeginPlay();

	asteroidRotator = FMath::FRandRange(_define_RandomRotateSpeedMIN, _define_RandomRotateSpeedMAX);
	UE_LOG(LogClass, Log, TEXT("[Info][Resource][Spawn] Spawn Finish!"));
}

void AResource::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	SetActorRotation(FRotator(0.0f, GetActorRotation().Yaw + asteroidRotator, 0.0f));
}

float AResource::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) {

	float _remainDamage = DamageAmount * FMath::FRandRange(0.85f, 1.15f);
	float _effectDamage = 0.0f;
	
	//remainDamage = sDefShield.GetValue();
	if (currentDurability > _remainDamage) {
		_effectDamage = _remainDamage;
		currentDurability -= _remainDamage;
		_remainDamage = 0.0f;
	}
	else {
		_effectDamage = currentDurability;
		currentDurability = 0.0f;
		Destroy();
	}
	UE_LOG(LogClass, Log, TEXT("[Info][Resource][Damaged] %s Get %s Type of %.0f Damage From %s! Effect Damage : %.0f"), *this->GetName(), *DamageEvent.DamageTypeClass->GetName(), _remainDamage, *DamageCauser->GetName(), _effectDamage);

	return _effectDamage;
}

void AResource::BeginDestroy() {
	Super::BeginDestroy();
}
#pragma endregion

#pragma region SpaceObject Inheritance
int AResource::GetObjectID() const {
	return resourceID;
}

ObjectType AResource::GetObjectType() const {
	return ObjectType::Resource;
}

Faction AResource::GetFaction() const {
	return Faction::Neutral;
}

void AResource::SetFaction(const Faction setFaction) {
	return;
}

BehaviorState AResource::GetBehaviorState() const {
	return BehaviorState::Idle;
}

bool AResource::InitObject(const int objectId) {
	if (objectId < 0)
		return false;

	USafeENGINE* _tempInstance = Cast<USafeENGINE>(GetGameInstance());
	FResourceData _tempResourceData = _tempInstance->GetResourceData(objectId);
	bool _isRich = false;
	if (FMath::FRandRange(0.0f, 1.0f) > 1.0f - _tempResourceData.RichOreChance) 
		_isRich = true;

	if (resourceID != objectId) {
		resourceID = objectId;
		if(_isRich)
			objectName = FText::Format(NSLOCTEXT("FTextFieldLiteral", "FTextField", "Rich {name}"), _tempResourceData.ResourceName);
		else objectName = _tempResourceData.ResourceName;

	
		if(_tempResourceData.FlipSprite.Num() > 1)
			objectFlipBook = _tempResourceData.FlipSprite[FMath::RandRange(0, _tempResourceData.FlipSprite.Num() - 1)];
		else if(_tempResourceData.FlipSprite.Num() == 1)
			objectFlipBook = _tempResourceData.FlipSprite[0];
		if(objectFlipBook)
			objectSprite->SetSprite(objectFlipBook->GetSpriteAtFrame(0));
		resourceType = _tempResourceData.Type;

		maxDurability = FMath::FRandRange(_tempResourceData.DurabilityRange.X, _tempResourceData.DurabilityRange.Y);
		currentDurability = maxDurability;
		currentResource = FItem(_tempResourceData.ResourceItemID, FMath::FloorToInt(FMath::FRandRange(_tempResourceData.OreAmountRange.X, _tempResourceData.OreAmountRange.Y)));
		defResource = _tempResourceData.DurabilityDef;
		if (_isRich) {
			currentDurability *= FMath::FRandRange(2.0f, 3.0f);
			currentResource.itemAmount *= FMath::FRandRange(2.0f, 3.0f);
		}
	}
	return true;
}

bool AResource::LoadBaseObject(const float shield, const float armor, const float hull, const float power) {
	return false;
}

float AResource::GetValue(const GetStatType statType) const {
	float _value;

	switch (statType) {
	case GetStatType::maxHull:
		_value = maxDurability;
		break;
	case GetStatType::currentHull:
		_value = currentDurability;
		break;
	case GetStatType::defHull:
		_value = defResource;
		break;
	default:
		_value = 0.0f;
		break;
	}
	return _value;
}
#pragma endregion

#pragma region Resource Functions
void AResource::SetResource(float durability, int amount) {
	USafeENGINE* _tempInstance = Cast<USafeENGINE>(GetGameInstance());
	FResourceData _tempResourceData = _tempInstance->GetResourceData(resourceID);

	currentDurability = FMath::Clamp(durability, 0.0f, maxDurability);
	currentResource.itemAmount = FMath::Clamp(amount, 0, FMath::FloorToInt(FMath::Max(_tempResourceData.OreAmountRange.X, _tempResourceData.OreAmountRange.Y)));
}

float AResource::GetResourceAmount() const {
	return currentResource.itemAmount;
}

FItem AResource::CollectResource(float miningPerfomance) {

	int _collectedAmount = FMath::Clamp(FMath::FloorToInt(miningPerfomance * (1.0f - defResource / 1000.0f)), 0, currentResource.itemAmount);
	currentResource.itemAmount -= _collectedAmount;

	if (currentResource.itemAmount <= 0)
		Destroy();
	return FItem(currentResource.itemID, _collectedAmount);
}
#pragma endregion