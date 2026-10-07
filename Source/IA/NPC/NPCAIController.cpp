// Fill out your copyright notice in the Description page of Project Settings.

#include "NPCAIController.h"
#include "Components/StateTreeAIComponent.h"
#include "StateTree.h"
#include "IA.h"

ANPCAIController::ANPCAIController()
{
	// crea el componente de StateTree
	StateTreeAI = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));
	check(StateTreeAI);

	// arrancamos la lógica manualmente en OnPossess, cuando ya existe el Pawn
	bStartAILogicOnPossess = false;
	StateTreeAI->SetStartLogicAutomatically(false);

	bAttachToPawn = true;

	// asset por defecto: Content/IA_NPC/ST_NPC
	DefaultStateTree = TSoftObjectPtr<UStateTree>(FSoftObjectPath(TEXT("/Game/IA_NPC/ST_NPC.ST_NPC")));
}

void ANPCAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// referenciamos el StateTree en el componente antes de arrancarlo
	if (UStateTree* Tree = DefaultStateTree.LoadSynchronous())
	{
		StateTreeAI->SetStateTree(Tree);
	}
	else
	{
		UE_LOG(LogIA, Warning, TEXT("NPCAIController: no se encontró el StateTree %s"), *DefaultStateTree.ToString());
	}

	StateTreeAI->StartLogic();
}
