// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"

#include "AITypes.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if(ShooterCharacter == nullptr)
	{
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}
	if(ShooterCharacter)
	{
		//Get the lateral speed of the character from velocity
		FVector Velocity{ShooterCharacter->GetVelocity()};
		Velocity.Z = 0;
		Speed = Velocity.Size();

		//Is the character in the air?
		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();

		//Is the character accelerating?
		if(ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0)
		{
			bIsAccelerating = true;
		}
		else
		{
			bIsAccelerating = false;
		}
		
		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
		//FString RotationMessage = FString::Printf(TEXT("Base Aim Rotation: %f"), AimRotation.Yaw);
		//if(GEngine)
		//{
		//	GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, RotationMessage);
		//}
		
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
		//FString MovementRotationMessage = FString::Printf(TEXT("Movement Rotation: %f"), MovementRotation.Yaw);
		//if(GEngine)
		//{
		//	GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, MovementRotationMessage);
		//}
		
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
		//FString OffsetMessage = FString::Printf(TEXT("MovementOffsetYaw: %f"), MovementOffsetYaw);
		//if(GEngine)
		//{
		//	GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, OffsetMessage);
		//}
		if(ShooterCharacter->GetVelocity().Size() > 0.f)
		{
			LastMovementOffsetYaw = MovementOffsetYaw;
		}
	}
	
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	

	
}
