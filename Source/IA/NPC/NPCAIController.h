// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "NPCAIController.generated.h"

class UStateTreeAIComponent;
class UStateTree;

/**
 *  AI Controller del NPC.
 *  Contiene el StateTreeAIComponent (schema "StateTree AI Component") y arranca
 *  la lógica cuando posee al Pawn.
 */
UCLASS()
class ANPCAIController : public AAIController
{
	GENERATED_BODY()

	/** Componente que ejecuta el StateTree */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStateTreeAIComponent> StateTreeAI;

protected:

	/** StateTree que ejecutará el componente (por defecto /Game/IA_NPC/ST_NPC) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	TSoftObjectPtr<UStateTree> DefaultStateTree;

public:

	ANPCAIController();

	UStateTreeAIComponent* GetStateTreeAI() const { return StateTreeAI; }

protected:

	virtual void OnPossess(APawn* InPawn) override;
};
