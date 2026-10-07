// Fill out your copyright notice in the Description page of Project Settings.

#include "NPCCharacter.h"
#include "NPCAIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "UObject/ConstructorHelpers.h"
#include "DrawDebugHelpers.h"

ANPCCharacter::ANPCCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	// el NPC lo posee nuestro AI Controller, tanto si está colocado en el nivel como si se spawnea
	AIControllerClass = ANPCAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// rotar hacia la dirección del movimiento
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;

	// malla y animación del maniquí del template
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMeshAsset(MeshFinder.Object);
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimFinder(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));
	if (AnimFinder.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(AnimFinder.Class);
	}

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -96.0f), FRotator(0.0f, -90.0f, 0.0f));
}

void ANPCCharacter::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();
	SetMoveSpeed(PatrolSpeed);
}

void ANPCCharacter::GetPatrolLocations(TArray<FVector>& OutLocations) const
{
	OutLocations.Reset();

	for (const AActor* Point : PatrolPoints)
	{
		if (Point)
		{
			OutLocations.Add(Point->GetActorLocation());
		}
	}

	// sin puntos asignados: patrullar entre dos puntos a los lados del spawn
	if (OutLocations.Num() < 2)
	{
		OutLocations.Reset();
		const FVector Offset = GetActorRightVector() * AutoPatrolDistance;
		OutLocations.Add(SpawnLocation + Offset);
		OutLocations.Add(SpawnLocation - Offset);
	}
}

void ANPCCharacter::SetMoveSpeed(float Speed)
{
	GetCharacterMovement()->MaxWalkSpeed = Speed;
}

void ANPCCharacter::ShowStateText(const FString& Text, const FColor& Color, float Duration) const
{
	DrawDebugString(GetWorld(), FVector(0.0f, 0.0f, 120.0f), Text, const_cast<ANPCCharacter*>(this), Color, Duration, true, 1.5f);
}
