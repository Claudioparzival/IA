// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NPCCharacter.generated.h"

/**
 *  NPC controlado por ANPCAIController + StateTree.
 *  Patrulla entre puntos, persigue al jugador al verlo y lo saluda al acercarse.
 */
UCLASS()
class ANPCCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	/** Puntos de patrulla (opcional). Si está vacío se patrulla entre dos puntos generados alrededor del spawn. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "NPC|Patrol")
	TArray<TObjectPtr<AActor>> PatrolPoints;

	/** Distancia a cada lado del spawn para la patrulla automática */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Patrol", meta = (Units = "cm"))
	float AutoPatrolDistance = 600.0f;

	/** Velocidad al patrullar */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Movement", meta = (Units = "cm/s"))
	float PatrolSpeed = 200.0f;

	/** Velocidad al perseguir */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Movement", meta = (Units = "cm/s"))
	float ChaseSpeed = 450.0f;

	/** Índice del punto de patrulla actual (persiste entre entradas al estado) */
	int32 CurrentPatrolIndex = 0;

public:

	ANPCCharacter();

	virtual void BeginPlay() override;

	/** Devuelve la lista de puntos de patrulla a usar */
	void GetPatrolLocations(TArray<FVector>& OutLocations) const;

	/** Cambia la velocidad de movimiento */
	void SetMoveSpeed(float Speed);

	/** Muestra un texto de debug sobre la cabeza del NPC (visible en el juego) */
	void ShowStateText(const FString& Text, const FColor& Color, float Duration = 0.0f) const;

protected:

	/** Ubicación de spawn usada para la patrulla automática */
	FVector SpawnLocation;
};
