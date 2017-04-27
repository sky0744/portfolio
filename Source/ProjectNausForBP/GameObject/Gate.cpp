﻿// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectNausForBP.h"
#include "Gate.h"

AGate::AGate()
{
	objectMesh->SetCanEverAffectNavigation(true);
	objectMesh->SetEnableGravity(false);
	objectMesh->SetSimulatePhysics(true);
	objectMesh->BodyInstance.MassScale = 100.0f;
	objectMesh->BodyInstance.LinearDamping = 50.0f;
	objectMesh->BodyInstance.AngularDamping = 5000.0f;
	objectMesh->BodyInstance.bLockZTranslation = true;
	objectMesh->BodyInstance.bLockXRotation = true;
	objectMesh->BodyInstance.bLockYRotation = true;
	objectMesh->Mobility = EComponentMobility::Movable;
	RootComponent = objectMesh;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;
	PrimaryActorTick.bTickEvenWhenPaused = false;
	PrimaryActorTick.TickInterval = 5.0f;
}

#pragma region Event Calls
void AGate::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogClass, Log, TEXT("[Info][Gate][Spawn] Spawn Finish!"));
}

void AGate::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	currentShield = FMath::Clamp(currentShield + rechargeShield * DeltaTime, 0.0f, maxShield);
	currentArmor = FMath::Clamp(currentArmor + repairArmor * DeltaTime, 0.0f, maxArmor);
	currentHull = FMath::Clamp(currentHull + repairHull * DeltaTime, 0.0f, maxHull);
}

float AGate::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) {
	Faction dealingFaction;

	if (DamageCauser->IsA(ABeam::StaticClass()))
		dealingFaction = Cast<ABeam>(DamageCauser)->GetLaunchingFaction();
	else if (DamageCauser->IsA(AProjectiles::StaticClass()))
		dealingFaction = Cast<AProjectiles>(DamageCauser)->GetLaunchingFaction();
	else
		return 0.0f;

	FHitResult _hitResult;
	FVector _hitDirect;
	DamageEvent.GetBestHitInfo(this, DamageCauser, _hitResult, _hitDirect);
	_hitDirect = _hitResult.ImpactPoint - GetActorLocation();
	_hitDirect.Normalize();

	float _remainDamage = DamageAmount * FMath::FRandRange(0.85f, 1.15f);
	float _effectShieldDamage = 0.0f;
	float _effectArmorDamage = 0.0f;
	float _effectHullDamage = 0.0f;
	bool _isCritical = false;

	if (FVector::DotProduct(GetActorForwardVector(), _hitDirect) > 0.95f) {
		_remainDamage *= 2.0f;
		_isCritical = true;
	}
	else if (FVector::DotProduct(GetActorForwardVector(), _hitDirect) < -0.95f) {
		_remainDamage *= 3.0f;
		_isCritical = true;
	}

	//remainDamage = sDefShield.GetValue();
	if (currentShield > _remainDamage) {
		_effectShieldDamage = _remainDamage;
		currentShield -= _remainDamage;
		_remainDamage = 0.0f;
	}
	else {
		_effectShieldDamage = currentShield;
		_remainDamage -= _effectShieldDamage;
		currentShield = 0.0f;
	}

	if (currentArmor > _remainDamage) {
		_effectArmorDamage = _remainDamage;
		currentArmor -= _remainDamage;
		_remainDamage = 0.0f;
	}
	else {
		_effectArmorDamage = currentArmor;
		_remainDamage -= _effectArmorDamage;
		currentArmor = 0.0f;
	}

	if (currentHull > _remainDamage) {
		_effectHullDamage = _remainDamage;
		currentHull -= _remainDamage;
		_remainDamage = 0.0f;
	}
	else {
		_effectHullDamage = currentHull;
		currentHull = 0.0f;
		Destroy();
	}

	UE_LOG(LogClass, Log, TEXT("[Info][Drone][Damaged] %s Get %s Type of %.0f Damage From %s! Effect Damage : Shield - %.0f / Armor - %.0f / Hull - %.0f. is Critical Damage? : %s"), *this->GetName(), *DamageEvent.DamageTypeClass->GetName(), _remainDamage, *DamageCauser->GetName(), _effectShieldDamage, _effectArmorDamage, _effectHullDamage, _isCritical ? TEXT("Critical") : TEXT("Non Critical"));

	return _effectShieldDamage + _effectArmorDamage + _effectHullDamage;
}

void AGate::BeginDestroy() {
	if (structureInfo) {
		structureInfo->isDestroyed = true;
		structureInfo->remainRespawnTime = structureInfo->maxRespawnTime;
	}
	if (GetWorld() && UGameplayStatics::GetGameState(GetWorld()) && UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD()->IsA(ASpaceHUDBase::StaticClass())) {
		Cast<ASpaceState>(UGameplayStatics::GetGameState(GetWorld()))->AccumulateToShipCapacity(true);
		Cast<ASpaceHUDBase>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD())->RemoveFromObjectList(this);
	}

	UnregisterAllComponents();
	Super::BeginDestroy();
}
#pragma endregion

#pragma region SpaceObject Inheritance
int AGate::GetObjectID() const {
	if (structureInfo != nullptr)
		return structureInfo->structureID;
	else return -1;
}

ObjectType AGate::GetObjectType() const {
	return ObjectType::Gate;
}

Faction AGate::GetFaction() const {
	if (structureInfo != nullptr)
		return structureInfo->structureFaction;
	else return Faction::Neutral;
}

void AGate::SetFaction(const Faction setFaction) {
	return;
}

BehaviorState AGate::GetBehaviorState() const {
	return BehaviorState::Idle;
}

bool AGate::InitObject(const int objectId) {
	return false;
}

bool AGate::LoadBaseObject(const float shield, const float armor, const float hull, const float power) {
	return false;
}

float AGate::GetValue(const GetStatType statType) const {
	float _value;

	switch (statType) {
	case GetStatType::halfLength:
		_value = lengthToLongAsix * 0.5f;
		break;
	case GetStatType::maxShield:
		_value = maxShield;
		break;
	case GetStatType::rechargeShield:
		_value = rechargeShield;
		break;
	case GetStatType::currentShield:
		_value = currentShield;
		break;
	case GetStatType::defShield:
		_value = defShield;
		break;

	case GetStatType::maxArmor:
		_value = maxArmor;
		break;
	case GetStatType::repaireArmor:
		_value = repairArmor;
		break;
	case GetStatType::currentArmor:
		_value = currentArmor;
		break;
	case GetStatType::defArmor:
		_value = defArmor;
		break;

	case GetStatType::maxHull:
		_value = maxHull;
		break;
	case GetStatType::repaireHull:
		_value = repairHull;
		break;
	case GetStatType::currentHull:
		_value = currentHull;
		break;
	case GetStatType::defHull:
		_value = defHull;
		break;
	default:
		_value = 0.0f;
		break;
	}

	return _value;
}

void AGate::GetRepaired(const GetStatType statType, float repairValue) {

	repairValue = FMath::Clamp(repairValue, 0.0f, 500.0f);
	switch (statType) {
	case GetStatType::currentShield:
		currentShield = FMath::Clamp(currentShield + repairValue, 0.0f, maxShield);
		break;
	case GetStatType::currentArmor:
		currentArmor = FMath::Clamp(currentArmor + repairValue, 0.0f, maxArmor);
		break;
	case GetStatType::currentHull:
		currentHull = FMath::Clamp(currentHull + repairValue, 0.0f, maxHull);
		break;
	default:
		break;
	}
}
#pragma endregion

#pragma region Interface Implementation : IStructureable
const FString AGate::GetDestinationName() const {
	if (structureInfo != nullptr)
		return structureInfo->LinkedSector;
	else return "";
}

StructureType AGate::GetStationType() const {
	return StructureType::Gate;
}

bool AGate::RequestedDock(const Faction requestFaction) const {
	if (structureInfo != nullptr)
		return true;
	else return false;
}
bool AGate::RequestedJump(const Faction requestFaction) const {
	if(structureInfo != nullptr)
		return true;
	else return false;
}

bool AGate::SetStructureData(UPARAM(ref) FStructureInfo& structureData) {
	if (isInited)
		return false;

	USafeENGINE* _tempInstance = Cast<USafeENGINE>(GetGameInstance());
	FStationData _tempStationData;
	
	structureInfo = &structureData;
	_tempStationData = _tempInstance->GetStationData(structureInfo->structureID);
	objectName = _tempStationData.Name;

	UStaticMesh* newMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), NULL, *_tempStationData.MeshPath.ToString()));
	objectMesh->SetStaticMesh(newMesh);
	lengthToLongAsix = _tempStationData.lengthToLongAsix;

	maxShield = _tempStationData.Shield;
	currentShield = structureInfo->structureShieldRate * maxShield;
	rechargeShield = _tempStationData.RechargeShield;
	defShield = _tempStationData.DefShield;

	maxArmor = _tempStationData.Armor;
	currentArmor = structureInfo->structureArmorRate * maxArmor;
	repairArmor = _tempStationData.RepairArmor;
	defArmor = _tempStationData.DefArmor;

	maxHull = _tempStationData.Hull;
	currentHull = structureInfo->structureHullRate * maxHull;
	repairHull = _tempStationData.RepairHull;
	defHull = _tempStationData.DefHull;

	isInited = true;
	return true;
}

void AGate::GetStructureData(FStructureInfo& structureData) const {
	structureData = *structureInfo;
	return;
}
#pragma endregion

#pragma region Functions
FStructureInfo* AGate::GetStructureDataPointer() const{
	return structureInfo;
}

#pragma endregion
