// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterCharacter.h"

#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"


// Sets default values
AShooterCharacter::AShooterCharacter() :
BaseTurnRate(45.f),
BaseLookUpRate(45.f)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create a camera boom(pulls in towards the character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f; // The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; //Rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 50.f);
	
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
	
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
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

void AShooterCharacter::FireWeapon()
{
	if(FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if(BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());
		if(MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}
		//Using GetBeamEndLocation Refactor func
		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);
		if (bBeamEnd)
		{
			if(ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEnd);
			}
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
			if(Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
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
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
	
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{

	// Get current size of the viewport
		FVector2D ViewportSize;
		if(GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->GetViewportSize(ViewportSize);
		}
		// Get screen space location of crosshairs
		FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
		CrosshairLocation.Y -= 50.f;

		FVector CrosshairWorldPosition;
		FVector CrosshairWorldDirection;
		// Get World position and direction of crosshairs
		bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
			CrosshairLocation,
			CrosshairWorldPosition,
			CrosshairWorldDirection);
		
		if(bScreenToWorld) // was deprojection successful?
		{
			FHitResult ScreenTraceHit;
			const FVector Start{CrosshairWorldPosition};
			const FVector End{CrosshairWorldPosition + CrosshairWorldDirection * 50000.f};
			// Set beam end point to line trace end point
			OutBeamLocation = End;
			// Trace outward from crosshairs world location
			GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);
			if(ScreenTraceHit.bBlockingHit) // was there a trace hit?
			{
				// Beam end point is now trace hit location
				OutBeamLocation = ScreenTraceHit.Location;
			}
			// Perform a second trace, this time for the gun barrel
			FHitResult WeaponTraceHit;
			const FVector WeaponTraceStart {MuzzleSocketLocation};
			const FVector WeaponTraceEnd {OutBeamLocation};
			GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
			if (WeaponTraceHit.bBlockingHit) // Object between barrel and BeamEndPoint?
			{
				OutBeamLocation = WeaponTraceHit.Location;
			}
			return true;
		}
	return false;
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireWeapon);
}
