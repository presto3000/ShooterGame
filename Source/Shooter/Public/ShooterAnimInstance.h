// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EOffsetState : uint8
{
	// E Offset State
	EOS_Aiming UMETA(DisplayName = "Aiming"),
	EOS_Hip UMETA(DisplayName = "Hip"),
	EOS_Reloading UMETA(DisplayName = "Reloading"),
	EOS_InAir UMETA(DisplayName = "InAir"),
	
	EOS_MAX UMETA(DisplayName = "DefaultMAX")
	
};
/**
 * 
 */
UCLASS()
class SHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UShooterAnimInstance();
	
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);
	
	virtual void NativeInitializeAnimation() override;

protected:
	/** Handle turning in place variables */
	void TurnInPlace();

	/** Handle calculations for leaning while running */
	void Lean(float DeltaTime);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class AShooterCharacter* ShooterCharacter;

	/** The speed of the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;
	/** Whether or not the character is in the air */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;
	/** Whether or not the character is moving */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;
	/** Offset yaw used for strafing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MovementOffsetYaw;
	/** Offset yaw the frame before we stopped moving */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = Movement, meta = (AllowPrivateAccess = "true"))
	float LastMovementOffsetYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	/** Only updated when standing still and not in Air. Yaw of the Character this frame */
	float TIPCharacterYaw;

	/** Only updated when standing still and not in Air. Yaw of the Character the previous frame */
	float TIPCharacterYawLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn in Place", meta = (AllowPrivateAccess = "true"))
	float RootYawOffset;

	/** Rotation curve value this frame */
	float RotationCurve;

	/** Rotation curve value last frame */
	float RotationCurveLastFrame;

	/** The pitch of the aim rotation, used for Aim Offset */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn in Place", meta = (AllowPrivateAccess = "true"))
	float Pitch;

	/** True when reloading, used to prevent Aim Offset while reloading */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn in Place", meta = (AllowPrivateAccess = "true"))
	bool bReloading;

	/** Offset state; used to determine which Aim Offset to use */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn in Place", meta = (AllowPrivateAccess = "true"))
	EOffsetState OffsetState;

	
	/** Yaw of the Character this frame */
	FRotator CharacterRotation;

	/** Yaw of the Character the previous frame */
	FRotator CharacterRotationLastFrame;

	/** Yaw delta used for leaning in the running blendspace */
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite, Category = "Lean", meta = (AllowPrivateAccess = "true"))
	float YawDelta;

	/** True when crouching */
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite, Category = "Crouching", meta = (AllowPrivateAccess = "true"))
	bool bCrouching;

	/** True when equipping */
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite, Category = "Crouching", meta = (AllowPrivateAccess = "true"))
	bool bEquipping;

	/** Change the recoil weight based on turning in place and aiming */
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float RecoilWeight;
	
	/** True when turning in place */
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bTurningInPlace;
	
};
