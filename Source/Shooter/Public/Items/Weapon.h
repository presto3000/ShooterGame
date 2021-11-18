// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "AmmoType.h"
#include "Engine/DataTable.h"
#include "WeaponType.h"
#include "Weapon.generated.h"

USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAmmoType AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WeaponAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MagazingCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USoundCue* PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* EquipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* InventoryIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* AmmoIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstance* MaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaterialIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ClipBoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ReloadMontageSection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> AnimBP;


	//----Crosshair
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsMiddle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsBottom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoFireRate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UParticleSystem* MuzzleFlash;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* FireSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BoneToHide;
};
/**
 * 
 */
UCLASS()
class SHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()
public:
	AWeapon();

	virtual void Tick (float DeltaTime) override;
protected:
	void StopFalling();

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void BeginPlay() override;
private:
	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bFalling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties", meta = (Allowprivateaccess = "true"))
	int32 Ammo;

	/** Maximum ammo that our weapon can hold */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties", meta = (Allowprivateaccess = "true"))
	int32 MagazineCapacity;

	/** The type of weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties", meta = (Allowprivateaccess = "true"))
	EWeaponType WeaponType;

	/** The type of ammo for this weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties", meta = (Allowprivateaccess = "true"))
	EAmmoType AmmoType;

	/** FName for the Reload Montage Section */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties", meta = (Allowprivateaccess = "true"))
	FName ReloadMontageSection;

	/** True when moving the clip while reloading */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponProperties", meta = (Allowprivateaccess = "true"))
	bool bMovingClip;

	/** Name for the clip bone  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties", meta = (Allowprivateaccess = "true"))
	FName ClipBoneName;

#pragma region WeaponDataTable
	//-------------
	/** Data table for weapon properties */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataTable, meta = (Allowprivateaccess = "true"))
	UDataTable* WeaponDataTable;

	int32 PreviousMaterialIndex;

	//----Crosshair : Textures for the weapon crosshairs
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (Allowprivateaccess = "true"))
	UTexture2D* CrosshairsMiddle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (Allowprivateaccess = "true"))
	UTexture2D* CrosshairsLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (Allowprivateaccess = "true"))
	UTexture2D* CrosshairsRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (Allowprivateaccess = "true"))
	UTexture2D* CrosshairsBottom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (Allowprivateaccess = "true"))
	UTexture2D* CrosshairsTop;

	/** The speed at which automatic fire happens */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (Allowprivateaccess = "true"))
	float AutoFireRate;

	/** Particle system spawned at the BarrelSocket */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (Allowprivateaccess = "true"))
	class UParticleSystem* MuzzleFlash;

	/** Sound played when the weapon is fired */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (Allowprivateaccess = "true"))
	USoundCue* FireSound;

	/** Name of the bone to hide on the weapon mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName BoneToHide;
	
#pragma endregion WeaponDataTable
public:
	/** Adds an impulse to the Weapon */
	void ThrowWeapon();

	FORCEINLINE int32 GetAmmo() const {return Ammo; }
	FORCEINLINE int32 GetMagazineCapacity() const {return MagazineCapacity; }
	/** Called from Character class when firing weapon */
	void DecrementAmmo();

	FORCEINLINE EWeaponType GetWeaponType() const {return WeaponType; }
	FORCEINLINE EAmmoType GetAmmoType() const {return AmmoType; }
	FORCEINLINE FName GetReloadMontageSection() const {return ReloadMontageSection; }
	FORCEINLINE void SetReloadMontageSection(FName Name) {ReloadMontageSection = Name; }
	FORCEINLINE FName GetClipBoneName() const {return  ClipBoneName; }
	FORCEINLINE void SetClipBoneName(FName Name) {ClipBoneName = Name; }
	FORCEINLINE float GetAutoFireRate() const {return AutoFireRate; }
	FORCEINLINE UParticleSystem* GetMuzzleFlash() const {return MuzzleFlash; }
	FORCEINLINE USoundCue* GetFireSound() const {return FireSound; }
	
	void ReloadAmmo(int32 Amount);
	//Setter for moving clip:
	FORCEINLINE void SetMovingClip(bool Move) {bMovingClip = Move; }

	bool ClipIsFull();
};
