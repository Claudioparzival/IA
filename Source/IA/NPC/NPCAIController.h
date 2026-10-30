// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "NPCAIController.generated.h"

class UStateTreeAIComponent;
class UStateTree;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;

/**
 *  AI Controller del NPC.
 *  Contiene el StateTreeAIComponent (schema "StateTree AI Component") y arranca
 *  la lógica cuando posee al Pawn.
 *  La percepción (vista) envía eventos al StateTree sólo cuando el jugador
 *  entra o sale del cono de visión.
 */
UCLASS()
class ANPCAIController : public AAIController
{
	GENERATED_BODY()

	/** Componente que ejecuta el StateTree */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStateTreeAIComponent> StateTreeAI;

	/** Componente de percepción (sentido de vista) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAIPerceptionComponent> AIPerception;

	/** Configuración del sentido de vista */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

protected:

	/** StateTree que ejecutará el componente (por defecto /Game/IA_NPC/ST_NPC) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	TSoftObjectPtr<UStateTree> DefaultStateTree;

	/** true mientras el jugador está dentro del cono de visión */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|Perception")
	bool bCanSeePlayer = false;

	/** Última posición donde se vio al jugador */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|Perception")
	FVector LastKnownPlayerLocation = FVector::ZeroVector;

	/** Segundos sin ver al jugador antes de enviar el evento PlayerLost (evita parpadeos de la vista) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception", meta = (Units = "s", ClampMin = 0))
	float LoseSightDelay = 2.0f;

	/** Estado crudo de la percepción en este momento */
	bool bPlayerSensed = false;

	/** Timer para confirmar que el jugador se perdió */
	FTimerHandle LoseSightTimer;

public:

	ANPCAIController();

	UStateTreeAIComponent* GetStateTreeAI() const { return StateTreeAI; }

	bool CanSeePlayer() const { return bCanSeePlayer; }

	const FVector& GetLastKnownPlayerLocation() const { return LastKnownPlayerLocation; }

protected:

	virtual void OnPossess(APawn* InPawn) override;

	/** Llamado por la percepción cuando un estímulo cambia (entra/sale de la vista) */
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	/** Confirma la pérdida del jugador si sigue sin verlo */
	void OnLoseSightTimerExpired();
};
