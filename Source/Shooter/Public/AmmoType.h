// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "AmmoType.generated.h"
UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	// AmmoType
	EAT_9mm UMETA (DisplayName = "9am"),
	EAT_AR UMETA (DisplayName = "Assault Rifle"),

	EAT_MAX UMETA (DisplayName = "DefaultMAX"),
	//Default 
};