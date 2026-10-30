// Fill out your copyright notice in the Description page of Project Settings.

#include "NPCAIController.h"
#include "NPCGameplayTags.h"
#include "Components/StateTreeAIComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
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

	// percepción: sentido de vista. Usa trazas de visibilidad, así que respeta obstáculos (línea de visión real)
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1500.0f;
	SightConfig->LoseSightRadius = 1800.0f;
	SightConfig->PeripheralVisionAngleDegrees = 60.0f;
	SightConfig->SetMaxAge(5.0f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	AIPerception->ConfigureSense(*SightConfig);
	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
	SetPerceptionComponent(*AIPerception);
}

void ANPCAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AIPerception->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &ANPCAIController::OnTargetPerceptionUpdated);

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

void ANPCAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// sólo nos interesa el jugador
	const APawn* SensedPawn = Cast<APawn>(Actor);
	if (!SensedPawn || !SensedPawn->IsPlayerControlled())
	{
		return;
	}

	bPlayerSensed = Stimulus.WasSuccessfullySensed();
	LastKnownPlayerLocation = Stimulus.StimulusLocation;

	if (bPlayerSensed)
	{
		GetWorldTimerManager().ClearTimer(LoseSightTimer);

		// el StateTree sólo se entera cuando el jugador entra al cono de visión
		if (!bCanSeePlayer)
		{
			bCanSeePlayer = true;
			UE_LOG(LogIA, Log, TEXT("%s: jugador visto"), *GetNameSafe(GetPawn()));
			StateTreeAI->SendStateTreeEvent(TAG_NPC_Perception_PlayerSeen);
		}
	}
	else if (bCanSeePlayer && !GetWorldTimerManager().IsTimerActive(LoseSightTimer))
	{
		// salió del cono: confirmamos la pérdida tras LoseSightDelay segundos sin volver a verlo
		GetWorldTimerManager().SetTimer(LoseSightTimer, this, &ANPCAIController::OnLoseSightTimerExpired, FMath::Max(LoseSightDelay, KINDA_SMALL_NUMBER));
	}
}

void ANPCAIController::OnLoseSightTimerExpired()
{
	if (bPlayerSensed || !bCanSeePlayer)
	{
		return;
	}

	bCanSeePlayer = false;
	UE_LOG(LogIA, Log, TEXT("%s: jugador perdido"), *GetNameSafe(GetPawn()));
	StateTreeAI->SendStateTreeEvent(TAG_NPC_Perception_PlayerLost);
}
