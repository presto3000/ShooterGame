// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Ammo.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AAmmo::AAmmo()
{
	// Construct the AmmoMesh component and set it as the root
	AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoMesh"));
	SetRootComponent(AmmoMesh);

	GetCollisionBox()->SetupAttachment(GetRootComponent());
	GetPickupWidget()->SetupAttachment(GetRootComponent());
	GetAreaSphere()->SetupAttachment(GetRootComponent());
}

void AAmmo::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
}

void AAmmo::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAmmo::SetItemProperties(EItemState State)
{
	Super::SetItemProperties(State);
		switch (State)
	{
		//----------------EIS_Pickup----------------//
		case EItemState::EIS_Pickup:
			// Set mesh properties
			AmmoMesh->SetSimulatePhysics(false);
			AmmoMesh->SetEnableGravity(false);
			AmmoMesh->SetVisibility(true);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			
			break;
		//----------------EIS_Equipped----------------//
		case EItemState::EIS_Equipped:
			// Set mesh properties
			AmmoMesh->SetSimulatePhysics(false);
			AmmoMesh->SetEnableGravity(false);
			AmmoMesh->SetVisibility(true);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			
			break;
		//----------------EIS_Falling----------------//
		case EItemState::EIS_Falling:
			// Set mesh properties
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			AmmoMesh->SetSimulatePhysics(true);
			AmmoMesh->SetEnableGravity(true);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
			break;

		//----------------EIS_EqiupInterping----------------//
		case EItemState::EIS_EqiupInterping:
			// Set mesh properties
			AmmoMesh->SetSimulatePhysics(false);
			AmmoMesh->SetEnableGravity(false);
			AmmoMesh->SetVisibility(true);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			break;
	}
}
