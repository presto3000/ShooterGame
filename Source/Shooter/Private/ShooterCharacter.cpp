// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterCharacter.h"

#include "BulletHitInterface.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Enemy/Enemy.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Items/Ammo.h"
#include "Items/Item.h"
#include "Items/Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Shooter/Shooter.h"
#include "Sound/SoundCue.h"
#include "Windows/WindowsApplication.h"



// Sets default values
AShooterCharacter::AShooterCharacter() :
// Base rates for turning/looking up
BaseTurnRate(45.f),
BaseLookUpRate(45.f),
// Turn rates for aiming/not aiming
HipTurnRate(90.f),
HipLookUpRate(90.f),
AimingTurnRate(20.f),
AimingLookUpRate(20.f),
// Mouse look sensitivity scale factors
MouseHipTurnRate(1.f),
MouseHipLookUpRate(1.f),
MouseAimingTurnRate(0.6f),
MouseAimingLookUpRate(0.6f),
// True when aiming the weapon
bAiming(false),
// Camera field of view values
CameraDefaultFOV(0.f), // set in BeginPlay
CameraZoomedFOV(25.f),
CameraCurrentFOV(0.f),
ZoomInterpSpeed(20.f),
// Crosshair spread factors
CrosshairSpreadMultiplier(0.f),
CrosshairVelocityFactor(0.f),
CrosshairInAirFactor(0.f),
CrosshairAimFactor(0.f),
CrosshairShootingFactor(0.f),
// Automatic fire variables
bFireButtonPressed(false),
bShouldFire(true),
//AutomaticFireRate(0.1f),
// Bullet fire timer variables
ShootTimeDuration(0.05f),
bFiringBullet(false),
// Item Trace variables
bShouldTraceForItems(false),
OverlappedItemCount(0),
// Camera Interp variables
CameraInterpDistance(250.f),
CameraInterpElevation(65.f),
// Starting ammo amounts
Starting9mmAmmo(85),
StartingARAmmo(120),
// Combat variables
CombatState(ECombatState::ECS_Unoccupied),
bCrouching(false),
BaseMovementSpeed(650.f),
CrouchMovementSpeed(300.f),
StandingCapsuleHalfHeight(88.f),
CrouchingCapsuleHalfHeight(44.f),
BaseGroundFriction(2.f),
CrouchingGroundFriction(100.f),
bAimingButtonPressed(false),
// Pickup 
bShouldPlayPickupSound(true),
bShouldPlayEquipSound(true),
PickupSoundResetTime(0.2f),
EquipSoundResetTime(0.2f),
// Icon animation property
HighlightedSlot(-1)


{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create a camera boom(pulls in towards the character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 185.f; // The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; //Rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);
	
	//Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach camera to the end
	FollowCamera->bUsePawnControlRotation = false; //Camera does not rotate relative to arm

	// Don't rotate when the controller rotates. Let the controller only affect the camera.
	bUseControllerRotationPitch = false;  
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); //... at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	//Create Hand Scene Component 
	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComp"));

	// Create Interp Components
	WeaponInterpComp = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponInterpolationComponent"));
	WeaponInterpComp->SetupAttachment(GetFollowCamera());

	InterpComp1 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 1"));
	InterpComp1->SetupAttachment(GetFollowCamera());
	
	InterpComp2 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 2"));
	InterpComp2->SetupAttachment(GetFollowCamera());

	InterpComp3 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 3"));
	InterpComp3->SetupAttachment(GetFollowCamera());
	
	InterpComp4 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 4"));
	InterpComp4->SetupAttachment(GetFollowCamera());
	
	InterpComp5 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 5"));
	InterpComp5->SetupAttachment(GetFollowCamera());
	
	InterpComp6 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 6"));
	InterpComp6->SetupAttachment(GetFollowCamera());
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if(FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
		
	}
	// Spawn a default weapon and equip it
	EquipWeapon(SpawnDefaultWeapon());
	Inventory.Add(EquippedWeapon);
	EquippedWeapon->SetSlotIndex(0);
	EquippedWeapon->DisableCustomDepth();
	EquippedWeapon->DisableGlowMaterial();
	EquippedWeapon->SetCharacter(this);
	

	// Initialize AmmoMap
	InitializeAmmoMap();
	
	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;

	// Create FInterpLocation structs for each interp location. Add to array
	InitializeInterpLocations();
	
	
	UE_LOG(LogTemp, Warning, TEXT("BeginPlay() called!"));
	
	int myInt{ (int)42.54 };
	UE_LOG(LogTemp, Warning, TEXT("int my Int: %d"), myInt);

	float myFloat {3.14159f};
	UE_LOG(LogTemp, Warning, TEXT("int my Float: %f"), myFloat);

	double myDouble {0.00014159f};
	UE_LOG(LogTemp, Warning, TEXT("int my Double: %lf"), myDouble);
	
	char myChar {'J'};
	UE_LOG(LogTemp, Warning, TEXT("int my Double: %c"), myChar);
	
	wchar_t wideChar{L'J'};
	UE_LOG(LogTemp, Warning, TEXT("wchar_t wideChar: %lc"), wideChar);

	bool myBool{true};
	UE_LOG(LogTemp, Warning, TEXT("wchar_t wideChar: %d"), myBool);

	UE_LOG(LogTemp, Warning, TEXT("int: %d, float: %f, bool: %d"), myInt, myFloat, myBool);

	FString myString{TEXT("My String!!!")};
	UE_LOG(LogTemp, Warning, TEXT("FString myString: %s"), *myString);

	UE_LOG(LogTemp, Warning, TEXT("Name of instance: %s"), *GetName());
}

void AShooterCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation {Controller->GetControlRotation()};
		const FRotator YawRotation {0, Rotation.Yaw, 0};
		
		const FVector Direction {FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X)};
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation {Controller->GetControlRotation()};
		const FRotator YawRotation {0, Rotation.Yaw, 0};
		
		const FVector Direction {FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y)};
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds()); // deg/sec * sec/frame
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds()); // deg/sec * sec/frame
}

void AShooterCharacter::Turn(float Value)
{
	float TurnScaleFactor{};
	if(bAiming)
	{
		TurnScaleFactor = MouseAimingTurnRate;
	}
	else
	{
		TurnScaleFactor = MouseHipTurnRate;
	}
	AddControllerYawInput(Value * TurnScaleFactor);
}

void AShooterCharacter::LookUp(float Value)
{
	float LookUpScaleFactor{};
	if(bAiming)
	{
		LookUpScaleFactor = MouseAimingLookUpRate;
	}
	else
	{
		LookUpScaleFactor = MouseHipLookUpRate;
	}
	AddControllerPitchInput(Value * LookUpScaleFactor);
}

void AShooterCharacter::FireWeapon()
{
	if(EquippedWeapon == nullptr) return;
	{
		if(CombatState != ECombatState::ECS_Unoccupied) return;
		{
			if(WeaponHasAmmo())
			{
				PlayFireSound();
				SendBullet();
				PlayGunFireMontage();
				// Subtract 1 from the Weapon's Ammo
				EquippedWeapon->DecrementAmmo();
				StartFireTimer();

				if(EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol)
				{
					// Start moving slide timer
					EquippedWeapon->StartSlideTimer();
				}
			}
		

			//// Get current size of the viewport
			//FVector2D ViewportSize;
			//if(GEngine && GEngine->GameViewport)
			//{
			//	GEngine->GameViewport->GetViewportSize(ViewportSize);
			//}
			// Get screen space location of crosshairs
			//FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
			//CrosshairLocation.Y -= 50.f;
			//		
			//FVector CrosshairWorldPosition;
			//FVector CrosshairWorldDirection;
			// Get World position and direction of crosshairs
			//bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
			//	CrosshairLocation,
			//	CrosshairWorldPosition,
			//	CrosshairWorldDirection);
				
			//if(bScreenToWorld) // was deprojection successful?
			//{
			//	FHitResult ScreenTraceHit;
			//	const FVector Start{CrosshairWorldPosition};
			//	const FVector End{CrosshairWorldPosition + CrosshairWorldDirection * 50000.f};
			//	// Set beam end point to line trace end point
			//	FVector BeamEndPoint{ End };
			//	// Trace outward from crosshairs world location
			//	GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);
			//	if(ScreenTraceHit.bBlockingHit) // was there a trace hit?
			//	{
			//		// Beam end point is now trace hit location
			//		BeamEndPoint = ScreenTraceHit.Location;
			//	}
			//	// Perform a second trace, this time for the gun barrel
			//	FHitResult WeaponTraceHit;
			//	const FVector WeaponTraceStart {SocketTransform.GetLocation()};
			//	const FVector WeaponTraceEnd {BeamEndPoint};
			//	GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
			//	if (WeaponTraceHit.bBlockingHit) // Object betwenn barrel and BeamEndPoint?
			//	{
			//		BeamEndPoint = WeaponTraceHit.Location;
			//	}
			//	// Spawn impact particles after updating BeamEndPoint
			//	if(ImpactParticles)
			//	{
			//		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEndPoint);
			//	}
			//	if(BeamParticles)
			//	{
			//		UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
			//		if(Beam)
			//		{
			//			Beam->SetVectorParameter(FName("Target"), BeamEndPoint);
			//		}
			//	}
			//}
			/*
			FHitResult FireHit;
				
			const FVector Start{SocketTransform.GetLocation()};
			const FQuat Rotation{SocketTransform.GetRotation()};
			const FVector RotationAxis{Rotation.GetAxisX()};
			const FVector End{Start + RotationAxis * 50000.f};
		
			FVector BeamEndPoint{ End };
				
			GetWorld()->LineTraceSingleByChannel(FireHit, Start, End, ECollisionChannel::ECC_Visibility);
			if(FireHit.bBlockingHit)
			{
			//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.f);
			//DrawDebugPoint(GetWorld(), FireHit.Location, 5.f, FColor::Red, false, 2.f);
		
			BeamEndPoint = FireHit.Location;
					
			if(ImpactParticles)
			{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.Location);
			}	
			}
			if(BeamParticles)
			{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
			if(Beam)
			{
			Beam->SetVectorParameter(FName("Target"), BeamEndPoint);
			}
			}
			*/
		}
	}
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult)
{
	// Check for crosshair trace hit
	FHitResult CrosshairHitResult;
	//local:
	FVector OutBeamLocation;
	
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation);
	if(bCrosshairHit)
	{
		// Tentative beam location = still need to trace from gun
		OutBeamLocation = CrosshairHitResult.Location;
	}
	else // no crosshair trace hit
	{
		// OutBeamLocation is the End location for the line trace
	}
	
	// Perform trace from gun barrel
	// Perform a second trace, this time for the gun barrel
	//FHitResult WeaponTraceHit;
	
	const FVector WeaponTraceStart {MuzzleSocketLocation};
	const FVector StartToEnd{OutBeamLocation - MuzzleSocketLocation};
	const FVector WeaponTraceEnd {MuzzleSocketLocation + StartToEnd * 1.25f};
	GetWorld()->LineTraceSingleByChannel(OutHitResult, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
	if (!OutHitResult.bBlockingHit) // Object between barrel and BeamEndPoint?
	{
		//OutBeamLocation = WeaponTraceHit.Location;
		OutHitResult.Location = OutBeamLocation;
		return false;
	}
	return true;
}

void AShooterCharacter::AimingButtonPressed()
{
	bAimingButtonPressed = true;
	if(CombatState != ECombatState::ECS_Reloading && CombatState != ECombatState::ECS_Equipping)
	{
		Aim();
	}
}

void AShooterCharacter::AimingButtonReleased()
{
	bAimingButtonPressed = false;
	StopAiming();
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	//Set current camera field of view 
	if(bAiming)
	{
		// Interpolate to zoomed FOV
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		// Interpolate to default FOV
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::SetLookRates()
{
	if(bAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange{0.f, 600.f};
	FVector2D VelocityMultiplierRange {0.f, 1.f};
	FVector Velocity {GetVelocity()};
	Velocity.Z = 0.f;
	// Calculate crosshair velocity factor
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	// Calculate crosshair in air factor
	if(GetCharacterMovement()->IsFalling()) // is in air?
	{
		// Spread the crosshairs slowly while in air
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else // Character is on the ground
	{
		//Shrink the crosshairs rapidly while on the ground
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
	}
	// Calcualte crosshair aim factor
	if(bAiming) // Are we aiming?
	{
		// Shrink crosshairs a small amount very quickly
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.4f, DeltaTime, 30.f);
	}
	else // not aiming
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}
	// True 0.05f second after firing
	if(bFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 60.f);
	}
	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;

	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this,&AShooterCharacter::FinishCrosshairBulletFire, ShootTimeDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

void AShooterCharacter::FireButtonPressed()
{
	//if(WeaponHasAmmo())
	//{
		bFireButtonPressed = true;
		FireWeapon();
		//StartFireTimer();
	//}
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	if(EquippedWeapon == nullptr) return;
	CombatState = ECombatState::ECS_FireTimerInProgress;
	
	//if(bShouldFire)
	//{
	//FireWeapon();
	//bShouldFire = false;
	//Start the timer
	GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, EquippedWeapon->GetAutoFireRate());
	
	//}
}

void AShooterCharacter::AutoFireReset()
{
	CombatState = ECombatState::ECS_Unoccupied;
	if(EquippedWeapon == nullptr) return;
	if(WeaponHasAmmo())
	{
		if(bFireButtonPressed && EquippedWeapon->GetAutomatic())
		{
			FireWeapon();
			
		}
	}
	else
	{
		ReloadWeapon();
	}
	//if(WeaponHasAmmo())
	//{
	//	bShouldFire = true;
	//	if(bFireButtonPressed)
	//	{
	//		StartFireTimer();
	//	}
	//}
	
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	// Get Viewport Size
	FVector2D ViewportSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	// Get screen space location of crosshairs
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	//CrosshairLocation.Y -= 50.f;

	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	// Get World position and direction of crosshairs
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if(bScreenToWorld)
	{
		// Trace from Crosshair world location outward
		const FVector Start {CrosshairWorldPosition};
		const FVector End {Start + CrosshairWorldDirection * 50000.f};
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);
		if(OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}
	return false;
}

void AShooterCharacter::TraceForItems()
{
	if(bShouldTraceForItems)
	{
		FHitResult ItemTraceResult;
		FVector HitLocation;
		TraceUnderCrosshairs(ItemTraceResult, HitLocation);
		if(ItemTraceResult.bBlockingHit)
		{
			TraceHitItem = Cast<AItem>(ItemTraceResult.Actor);
			const auto TraceHitWeapon = Cast<AWeapon> (TraceHitItem);
			if(TraceHitWeapon)
			{
				if(HighlightedSlot == -1)
				{
					// Not currently highlighting a lost; highlight one 
					HighlightInventorySlot();
				}
			}
			else
			{
				// Is a slot being highlight?
				if(HighlightedSlot != -1)
				{
					// UnHighlight the slot
					UnHighlightInventorySlot();
				}
			}
			
			if(TraceHitItem && TraceHitItem->GetItemState() == EItemState::EIS_EqiupInterping)
			{
				TraceHitItem = nullptr;
			}
			if (TraceHitItem && TraceHitItem->GetPickupWidget())
			{
				// Show Item's Pickup Widget
				TraceHitItem->GetPickupWidget()->SetVisibility(true);
				TraceHitItem->EnableCustomDepth();

				if(Inventory.Num() >= Inventory_CAPACITY)
				{
					//Inventory is full
					TraceHitItem->SetCharacterInventoryFull(true);
					
				}
				else
				{
					// Inventory has room
					TraceHitItem->SetCharacterInventoryFull(false);
				}
			}
			// We hit an AItem last frame
			if(TraceHitItemLastFrame)
			{
				if(TraceHitItem != TraceHitItemLastFrame)
				{
					// We are hitting a different AItem this frame from last frame
					// Or AItem is null.
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
					TraceHitItemLastFrame->DisableCustomDepth();
				}
			}
			//Store a reference to HitITem for next frame
			TraceHitItemLastFrame = TraceHitItem;
			
		}
	}
	else if (TraceHitItemLastFrame)
	{
		// No longer Overlapping any items,
		// Item last frame should not show widget
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
		TraceHitItemLastFrame->DisableCustomDepth();
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	// Check the TSubclassOf variable
	if(DefaultWeaponClass)
	{
		// Spawn the Weapon
		//AWeapon* DefaultWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	}
	return nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip, bool bSwapping)
{
	if(WeaponToEquip)
	{
		// Get the Hand Socket
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if(HandSocket)
		{
			// Attach the Weapon to the hand socket RightHandSocket
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}

		if(EquippedWeapon == nullptr)
		{
			// -1 = no EquippedWeapon yet. No need to reverse the icon animation
			EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());
		}
		else if(!bSwapping)
		{
			EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(), WeaponToEquip->GetSlotIndex());
		}

		
		// Set EquippedWeapon to the newly spawned Weapon
		EquippedWeapon = WeaponToEquip;
		// Set Item State
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}
}

void AShooterCharacter::DropWeapon()
{
	if(EquippedWeapon)
	{
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules); //detach our mesh

		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();
	}
}

void AShooterCharacter::SelectButtonPressed()
{
	if(CombatState != ECombatState::ECS_Unoccupied) return;
	if(TraceHitItem)
	{
		//auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
		//SwapWeapon(TraceHitWeapon);
		TraceHitItem->StartItemCurve(this, true);
		if(TraceHitItem->GetPickupSound())
		{
			UGameplayStatics::PlaySound2D(this, TraceHitItem->GetPickupSound());
		}
		TraceHitItem = nullptr;
	}

}

void AShooterCharacter::SelectButtonReleased()
{
	
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	if(Inventory.Num() - 1 >= EquippedWeapon->GetSlotIndex())
	{
		Inventory[EquippedWeapon->GetSlotIndex()] = WeaponToSwap;
		WeaponToSwap->SetSlotIndex(EquippedWeapon->GetSlotIndex());
		
	}

	DropWeapon();
	EquipWeapon(WeaponToSwap, true);
	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

bool AShooterCharacter::WeaponHasAmmo()
{
	if(EquippedWeapon == nullptr) return false;
	return EquippedWeapon->GetAmmo() > 0;
}

void AShooterCharacter::PlayFireSound()
{
	if(EquippedWeapon->GetFireSound())
	{
		//Play fire sound
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->GetFireSound());
	}
}

void AShooterCharacter::SendBullet()
{
	//const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	// Send bullet
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	if(BarrelSocket)
	{
		//const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());
		if(EquippedWeapon->GetMuzzleFlash())
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EquippedWeapon->GetMuzzleFlash(), SocketTransform);
		}
		//Using GetBeamEndLocation Refactor func
		//FVector BeamEnd;
		FHitResult BeamHitResult;
		
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamHitResult);
		if (bBeamEnd)
		{
			// Does hit Actor implement BulletHitInterface?
			if(BeamHitResult.Actor.IsValid())
			{
				IBulletHitInterface* BulletHitInterface = Cast<IBulletHitInterface>(BeamHitResult.Actor.Get());
				if(BulletHitInterface)
				{
					BulletHitInterface->BulletHit_Implementation(BeamHitResult);
				}
				AEnemy* HitEnemy = Cast<AEnemy>(BeamHitResult.Actor.Get());
				if(HitEnemy)
				{
					if(BeamHitResult.BoneName.ToString() == HitEnemy->GetHeadBone())
					{
						// Head shot
									UGameplayStatics::ApplyDamage(
						BeamHitResult.Actor.Get(),
						EquippedWeapon->GetHeadShotDamage(),
						GetController(),
						this,
						UDamageType::StaticClass());

					//UE_LOG(LogTemp, Warning, TEXT("Hit Component: %s"), *BeamHitResult.BoneName.ToString());		
					}
					else
					{
						// Body shot
						UGameplayStatics::ApplyDamage(
			BeamHitResult.Actor.Get(),
				EquippedWeapon->GetDamage(),
				GetController(),
					this,
								UDamageType::StaticClass());

						//UE_LOG(LogTemp, Warning, TEXT("Hit Component: %s"), *BeamHitResult.BoneName.ToString());
					}

				}
			}
			else
			{
				// Spawn default particles
				if(ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamHitResult.Location);
				}
			}

			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
			if(Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamHitResult.Location);
			}
		}
	}
}

void AShooterCharacter::PlayGunFireMontage()
{
	// Play Hip Fire Montage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
	// Start bullet fire timer for crosshairs
	StartCrosshairBulletFire();

	
}

void AShooterCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon()
{
	if(CombatState != ECombatState::ECS_Unoccupied) return;


	
	if(EquippedWeapon == nullptr) return;
	// Do we have ammo of the correct type?
	if(CarryingAmmo() && !EquippedWeapon->ClipIsFull()) 
	{
		if(bAiming)
		{
			StopAiming();
		}
		
		CombatState = ECombatState::ECS_Reloading;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if(ReloadMontage && AnimInstance)
		{
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());
		}
	}
}

void AShooterCharacter::FinishReloading()
{
	// Update the Combat State
	CombatState = ECombatState::ECS_Unoccupied;

	if(bAimingButtonPressed)
	{
		Aim();
	}

	if(EquippedWeapon == nullptr) return;
	const auto AmmoType {EquippedWeapon->GetAmmoType()};
	
	// Update the AmmoMap
	if(AmmoMap.Contains(EquippedWeapon->GetAmmoType()))
	{
		// Amount of ammo the Character is carrying of the EquppiedWeapon type
		int32 CarriedAmmo = AmmoMap[AmmoType];
		// Space left in the magazine of EquippedWeapon
		const int32 MagEmptySpace = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo();

		if(MagEmptySpace > CarriedAmmo)
		{
			// Reload the magazine with all the ammo we are carrying
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
		else
		{
			// fill the magazine
			EquippedWeapon->ReloadAmmo(MagEmptySpace);
			CarriedAmmo -= MagEmptySpace;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
	}
	
}

void AShooterCharacter::FinishEquipping()
{
	CombatState = ECombatState::ECS_Unoccupied;
	if(bAimingButtonPressed)
	{
		Aim();
	}
}

bool AShooterCharacter::CarryingAmmo()
{
	if(EquippedWeapon == nullptr) return false;

	auto AmmoType = EquippedWeapon->GetAmmoType();

	if(AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}

	return false;
}

void AShooterCharacter::GrabClip()
{
	if(EquippedWeapon == nullptr) return;

	if(HandSceneComponent == nullptr) return;
	
	// Index for the clip bone on the Equipped Weapon
	int32 ClipBoneIndex{ EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName())};
	// Store the transform of the clip
	ClipTransform =  EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);

	FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::KeepRelative, true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentTransformRules, FName(TEXT("Hand_L")));
	HandSceneComponent->SetWorldTransform(ClipTransform);

	EquippedWeapon->SetMovingClip(true);
}

void AShooterCharacter::ReleaseClip()
{
	EquippedWeapon->SetMovingClip(false);
}

void AShooterCharacter::CrouchButtonPressed()
{
	if (!GetCharacterMovement()->IsFalling())
	{
		bCrouching = !bCrouching;
	}
	if (bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
		GetCharacterMovement()->GroundFriction = CrouchingGroundFriction;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
		GetCharacterMovement()->GroundFriction = BaseGroundFriction;
	}
	
}

void AShooterCharacter::InterpCapsuleHalfHeight(float DeltaTime)
{
	float TargetCapsuleHalfHeight{};
	if(bCrouching)
	{
		TargetCapsuleHalfHeight = CrouchingCapsuleHalfHeight;
		
	}
	else
	{
		TargetCapsuleHalfHeight = StandingCapsuleHalfHeight;
	}
	const float InterpHalfHeight{FMath::FInterpTo(
		GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetCapsuleHalfHeight, DeltaTime, 20.f)};

	// Negative value if crouching, Positive value if standing
	const float DeltaCapsuleHalfHeight{InterpHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight()};
	const FVector MeshOffset{0.f, 0.f, -DeltaCapsuleHalfHeight};
	GetMesh()->AddLocalOffset(MeshOffset);
	
	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHeight);

	
}

void AShooterCharacter::Jump()
{
	if(bCrouching)
	{
		bCrouching = false;
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
	else
	{
		ACharacter::Jump();
	}
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Handle interpolation for zoom when aiming
	CameraInterpZoom(DeltaTime);
	// Change look sensitivity based on aiming
	SetLookRates();
	// Calculate crosshair spread multiplier
	CalculateCrosshairSpread(DeltaTime);
	// Check OverlappedItemCount, then trace for items
	TraceForItems();
	// Interpolate the capsule half height based on crouching/standing
	InterpCapsuleHalfHeight(DeltaTime);

}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);
	
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);
	
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);

	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AShooterCharacter::SelectButtonReleased);

	PlayerInputComponent->BindAction("ReloadButton", IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterCharacter::CrouchButtonPressed);

	PlayerInputComponent->BindAction("FKey", IE_Pressed, this, &AShooterCharacter::FKeyPressed);
	
	PlayerInputComponent->BindAction("1Key", IE_Pressed, this, &AShooterCharacter::OneKeyPressed);
	PlayerInputComponent->BindAction("2Key", IE_Pressed, this, &AShooterCharacter::TwoKeyPressed);
	PlayerInputComponent->BindAction("3Key", IE_Pressed, this, &AShooterCharacter::ThreeKeyPressed);
	PlayerInputComponent->BindAction("4Key", IE_Pressed, this, &AShooterCharacter::FourKeyPressed);
	PlayerInputComponent->BindAction("5Key", IE_Pressed, this, &AShooterCharacter::FiveKeyPressed);
}

void AShooterCharacter::Aim()
{
	bAiming = true;
	GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
}

void AShooterCharacter::StopAiming()
{
	bAiming = false;
	if(!bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
}


void AShooterCharacter::ResetPickupSoundTimer()
{
	bShouldPlayPickupSound = true;
}

void AShooterCharacter::ResetEquipSoundTimer()
{
	bShouldPlayEquipSound = true;
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	if(OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}
#if 0 // No longer needed; AItem has GetInterpLocation
FVector AShooterCharacter::GetCameraInterpLocation()
{
	const FVector CameraWorldLocation{FollowCamera->GetComponentLocation()};
	const FVector CameraForward{FollowCamera->GetForwardVector()};
	// Desired = CameraWorldLocation + Forward * A + Up * B
	return CameraWorldLocation + CameraForward * CameraInterpDistance + FVector(0.f, 0.f, CameraInterpElevation);
}
#endif
void AShooterCharacter::GetPickupItem(AItem* Item)
{
	Item->PlayEquipSound();
	
	auto Weapon = Cast<AWeapon>(Item);
	if(Weapon)
	{
		if(Inventory.Num() < Inventory_CAPACITY)
		{
			Weapon->SetSlotIndex(Inventory.Num());
			Inventory.Add(Weapon);
			Weapon->SetItemState(EItemState::EIS_PickedUp);
		}
		else // Inventory is full! Swap with EquippedWeapon
		{
			SwapWeapon(Weapon);

		}

	}

	auto Ammo = Cast<AAmmo> (Item);
	if(Ammo)
	{
		PickupAmmo(Ammo);
	}
	
	
}

void AShooterCharacter::PickupAmmo(AAmmo* Ammo)
{
	// check to see if AmmoMap contains Ammo's AmmoType
	if(AmmoMap.Find(Ammo->GetAmmoType()))
	{
		// Get amount of ammo in our AmmoMap for Ammo's type
		int32 AmmoCount{AmmoMap[Ammo->GetAmmoType()]};
		AmmoCount += Ammo->GetItemCount();
		// Set the amount of ammo in the Map for this type
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}
	if(EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType())
	{
		// Check to see if the gun is empty
		if(EquippedWeapon->GetAmmo() == 0)
		{
			ReloadWeapon();
		}
	}
	
	Ammo->Destroy();
}

void AShooterCharacter::InitializeInterpLocations()
{
	FInterpLocation WeaponLocation{WeaponInterpComp, 0};
	InterpLocations.Add(WeaponLocation);

	FInterpLocation InterpLoc1{InterpComp1, 0};
	InterpLocations.Add(InterpLoc1);
	
	FInterpLocation InterpLoc2{InterpComp2, 0};
	InterpLocations.Add(InterpLoc2);
	
	FInterpLocation InterpLoc3{InterpComp3, 0};
	InterpLocations.Add(InterpLoc3);
	
	FInterpLocation InterpLoc4{InterpComp4, 0};
	InterpLocations.Add(InterpLoc4);
	
	FInterpLocation InterpLoc5{InterpComp5, 0};
	InterpLocations.Add(InterpLoc5);
	
	FInterpLocation InterpLoc6{InterpComp6, 0};
	InterpLocations.Add(InterpLoc6);
	
}

void AShooterCharacter::FKeyPressed()
{
	if(EquippedWeapon->GetSlotIndex() == 0) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 0);
}

void AShooterCharacter::OneKeyPressed()
{
	if(EquippedWeapon->GetSlotIndex() == 1) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 1);
}

void AShooterCharacter::TwoKeyPressed()
{
		if(EquippedWeapon->GetSlotIndex() == 2) return;
    	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 2);
}

void AShooterCharacter::ThreeKeyPressed()
{
	if(EquippedWeapon->GetSlotIndex() == 3) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 3);
}

void AShooterCharacter::FourKeyPressed()
{
	if(EquippedWeapon->GetSlotIndex() == 4) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 4);
}

void AShooterCharacter::FiveKeyPressed()
{
	if(EquippedWeapon->GetSlotIndex() == 5) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 5);
}

void AShooterCharacter::ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
{
#if 0
	if((CurrentItemIndex == NewItemIndex) || (NewItemIndex >= Inventory.Num()
		|| (CombatState != ECombatState::ECS_Unoccupied))) return;
#endif
	const bool bCanExchangeItems = (CurrentItemIndex != NewItemIndex) &&
		(NewItemIndex < Inventory.Num()) &&
		(CombatState == ECombatState::ECS_Unoccupied || CombatState == ECombatState::ECS_Equipping);
	if (bCanExchangeItems)
	{
		if(bAiming)
		{
			StopAiming();
		}
		auto OldEquippedWeapon = EquippedWeapon;
		auto NewWeapon = Cast<AWeapon>(Inventory[NewItemIndex]);
		EquipWeapon(NewWeapon);

		OldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
		NewWeapon->SetItemState(EItemState::EIS_Equipped);

		CombatState = ECombatState::ECS_Equipping;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if(AnimInstance && EquipMontage)
		{
			AnimInstance->Montage_Play(EquipMontage, 1.f);
			AnimInstance->Montage_JumpToSection(FName("Equip"));
		}

		NewWeapon->PlayEquipSound(true);	
	}

}

int32 AShooterCharacter::GetEmptyInventorySlot()
{
	for(int32 i = 0; i < Inventory.Num(); i++)
	{
		if(Inventory[i] == nullptr)
		{
			return i;
		}
	}
	if (Inventory.Num() < Inventory_CAPACITY)
	{
		return Inventory.Num();
	}
	return -1; // Inventory is full!
}

void AShooterCharacter::HighlightInventorySlot()
{
	const int32 EmptySlot{GetEmptyInventorySlot()};
	HighlightIconDelegate.Broadcast(EmptySlot, true);
	HighlightedSlot = EmptySlot;
}

EPhysicalSurface AShooterCharacter::GetSurfaceType()
{
	// Line trace for the surfaces
	FHitResult HitResult;
	//Starting loc and end
	const FVector Start{GetActorLocation()};
	const FVector End {Start + FVector(0.f, 0.f, -400.f)};
	//Colission quryparams:
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;
	
	GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECollisionChannel::ECC_Visibility,
		QueryParams);
	//if(HitResult.GetActor())
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *HitResult.Actor->GetName());	
	//}
	//auto HitSurface = HitResult.PhysMaterial->SurfaceType;
	//if(HitSurface == EPS_Grass)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Hit Grass surface type!"));	
	//}

	return UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
	
	
}

void AShooterCharacter::UnHighlightInventorySlot()
{
	HighlightIconDelegate.Broadcast(HighlightedSlot, false);
	HighlightedSlot = -1;
}

int32 AShooterCharacter::GetInterpLocationIndex()
{
	int32 LowestIndex = 1;
	int32 LowestCount = INT_MAX;
	for(int32 i = 1; i < InterpLocations.Num(); i++)
	{
		if(InterpLocations[i].ItemCount < LowestCount)
		{
			LowestIndex = i;
			LowestCount = InterpLocations[i].ItemCount;
			
		}
	}
	return LowestIndex;
}

void AShooterCharacter::IncrementInterpLocItemCount(int32 Index, int32 Amount)
{
	if(Amount < -1 || Amount > 1) return;
	if(InterpLocations.Num() >= Index)
	{
		InterpLocations[Index].ItemCount += Amount;
	}
}

void AShooterCharacter::StartPickupSoundTimer()
{
	bShouldPlayPickupSound = false;
	GetWorldTimerManager().SetTimer(PickupSoundTimer, this,
		&AShooterCharacter::ResetPickupSoundTimer, PickupSoundResetTime);
	
}

void AShooterCharacter::StartEquipSoundTimer()
{
	bShouldPlayEquipSound = false;
	GetWorldTimerManager().SetTimer(EquipSoundTimer, this,
		&AShooterCharacter::ResetEquipSoundTimer, EquipSoundResetTime);
	
}

FInterpLocation AShooterCharacter::GetInterpLocation(int32 Index)
{
	if(Index <= InterpLocations.Num())
	{
		return InterpLocations[Index];
	}
	return FInterpLocation();
}
