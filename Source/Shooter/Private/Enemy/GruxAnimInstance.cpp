// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/GruxAnimInstance.h"

#include "Enemy/Enemy.h"

void UGruxAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if(Enemy == nullptr)
	{
		Enemy = Cast<AEnemy>(TryGetPawnOwner());
	}

	if(Enemy)
	{
		FVector Velocity {Enemy->GetVelocity()};
		Velocity.Z = 0.f;
		Speed = Velocity.Size();
	}
}
