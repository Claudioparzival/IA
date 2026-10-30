// Fill out your copyright notice in the Description page of Project Settings.

#include "NPCStateTreeUtility.h"
#include "NPCCharacter.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "DrawDebugHelpers.h"

namespace NPCStateTree
{
	/** Pawn del jugador 0 */
	static APawn* GetPlayerPawn(const AActor* WorldContext)
	{
		return WorldContext ? UGameplayStatics::GetPlayerPawn(WorldContext, 0) : nullptr;
	}
}

////////////////////////////////////////////////////////////////////
// NPC: Player In Range

bool FNPCPlayerInRangeCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	bool bInRange = false;

	if (const APawn* Player = NPCStateTree::GetPlayerPawn(InstanceData.Actor))
	{
		bInRange = FVector::Dist(Player->GetActorLocation(), InstanceData.Actor->GetActorLocation()) <= InstanceData.Radius;
	}

	return InstanceData.bInvert ? !bInRange : bInRange;
}

#if WITH_EDITOR
FText FNPCPlayerInRangeCondition::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	const FInstanceDataType* InstanceData = InstanceDataView.GetPtr<FInstanceDataType>();
	const float Radius = InstanceData ? InstanceData->Radius : 0.0f;
	const bool bInvert = InstanceData && InstanceData->bInvert;

	return FText::FromString(FString::Printf(TEXT("<b>Player %s</b> %.0f cm"), bInvert ? TEXT("farther than") : TEXT("within"), Radius));
}
#endif

////////////////////////////////////////////////////////////////////
// NPC: Patrol

EStateTreeRunStatus FNPCPatrolTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ANPCCharacter* NPC = Cast<ANPCCharacter>(InstanceData.Actor);
	if (!NPC || !InstanceData.AIController)
	{
		return EStateTreeRunStatus::Failed;
	}

	NPC->SetMoveSpeed(NPC->PatrolSpeed);

	// empezamos caminando hacia el punto actual
	InstanceData.bMoving = false;
	InstanceData.WaitRemaining = 0.0f;

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FNPCPatrolTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ANPCCharacter* NPC = Cast<ANPCCharacter>(InstanceData.Actor);
	AAIController* Controller = InstanceData.AIController;
	if (!NPC || !Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	TArray<FVector> Points;
	NPC->GetPatrolLocations(Points);
	NPC->CurrentPatrolIndex %= Points.Num();

	if (InstanceData.bMoving)
	{
		// ¿llegó al punto?
		if (Controller->GetMoveStatus() == EPathFollowingStatus::Idle)
		{
			InstanceData.bMoving = false;
			InstanceData.WaitRemaining = InstanceData.WaitTime;
			NPC->CurrentPatrolIndex = (NPC->CurrentPatrolIndex + 1) % Points.Num();
		}

		NPC->ShowStateText(FString::Printf(TEXT("PATRULLA -> punto %d"), NPC->CurrentPatrolIndex), FColor::Green);
	}
	else if (InstanceData.WaitRemaining > 0.0f)
	{
		// esperando en el punto
		InstanceData.WaitRemaining -= DeltaTime;
		NPC->ShowStateText(FString::Printf(TEXT("PATRULLA (esperando %.1fs)"), FMath::Max(InstanceData.WaitRemaining, 0.0f)), FColor::Green);
	}
	else
	{
		// ir al siguiente punto
		const EPathFollowingRequestResult::Type Result = Controller->MoveToLocation(Points[NPC->CurrentPatrolIndex], InstanceData.AcceptanceRadius);

		if (Result == EPathFollowingRequestResult::Failed)
		{
			// sin camino (¿falta NavMesh?): saltamos el punto y esperamos
			NPC->CurrentPatrolIndex = (NPC->CurrentPatrolIndex + 1) % Points.Num();
			InstanceData.WaitRemaining = InstanceData.WaitTime;
			NPC->ShowStateText(TEXT("PATRULLA: sin camino (falta NavMesh?)"), FColor::Red, 1.0f);
		}
		else
		{
			InstanceData.bMoving = Result == EPathFollowingRequestResult::RequestSuccessful;
			if (!InstanceData.bMoving)
			{
				// AlreadyAtGoal
				InstanceData.WaitRemaining = InstanceData.WaitTime;
				NPC->CurrentPatrolIndex = (NPC->CurrentPatrolIndex + 1) % Points.Num();
			}
		}
	}

	// la patrulla nunca termina sola: se sale por transición
	return EStateTreeRunStatus::Running;
}

void FNPCPatrolTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.AIController)
	{
		InstanceData.AIController->StopMovement();
	}
}

#if WITH_EDITOR
FText FNPCPatrolTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString(TEXT("<b>Patrol</b> between points"));
}
#endif

////////////////////////////////////////////////////////////////////
// NPC: Chase Player

EStateTreeRunStatus FNPCChasePlayerTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.Actor || !InstanceData.AIController)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (ANPCCharacter* NPC = Cast<ANPCCharacter>(InstanceData.Actor))
	{
		NPC->SetMoveSpeed(NPC->ChaseSpeed);
	}

	// forzar el cálculo de camino en el primer Tick
	InstanceData.RepathRemaining = 0.0f;

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FNPCChasePlayerTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	APawn* Player = NPCStateTree::GetPlayerPawn(InstanceData.Actor);
	if (!Player || !InstanceData.AIController)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.RepathRemaining -= DeltaTime;
	if (InstanceData.RepathRemaining <= 0.0f)
	{
		InstanceData.RepathRemaining = InstanceData.RepathInterval;
		InstanceData.AIController->MoveToActor(Player, InstanceData.AcceptanceRadius);
	}

	if (const ANPCCharacter* NPC = Cast<ANPCCharacter>(InstanceData.Actor))
	{
		NPC->ShowStateText(TEXT("PERSIGUIENDO!"), FColor::Orange);
	}

	return EStateTreeRunStatus::Running;
}

void FNPCChasePlayerTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.AIController)
	{
		InstanceData.AIController->StopMovement();
	}

	if (ANPCCharacter* NPC = Cast<ANPCCharacter>(InstanceData.Actor))
	{
		NPC->SetMoveSpeed(NPC->PatrolSpeed);
	}
}

#if WITH_EDITOR
FText FNPCChasePlayerTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString(TEXT("<b>Chase</b> the player"));
}
#endif

////////////////////////////////////////////////////////////////////
// NPC: Greet Player

EStateTreeRunStatus FNPCGreetPlayerTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.Actor || !InstanceData.AIController)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.TimeRemaining = InstanceData.Duration;

	// detenerse para interactuar
	InstanceData.AIController->StopMovement();

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FNPCGreetPlayerTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (const ANPCCharacter* NPC = Cast<ANPCCharacter>(InstanceData.Actor))
	{
		NPC->ShowStateText(InstanceData.Message, FColor::Cyan);
	}

	// girar suavemente hacia el jugador
	if (const APawn* Player = NPCStateTree::GetPlayerPawn(InstanceData.Actor))
	{
		const FRotator Current = InstanceData.Actor->GetActorRotation();
		const FRotator Target = (Player->GetActorLocation() - InstanceData.Actor->GetActorLocation()).GetSafeNormal2D().Rotation();
		InstanceData.Actor->SetActorRotation(FMath::RInterpTo(Current, FRotator(0.0f, Target.Yaw, 0.0f), DeltaTime, 8.0f));
	}

	InstanceData.TimeRemaining -= DeltaTime;

	return InstanceData.TimeRemaining > 0.0f ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
}

#if WITH_EDITOR
FText FNPCGreetPlayerTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString(TEXT("<b>Greet</b> the player"));
}
#endif

////////////////////////////////////////////////////////////////////
// NPC: Find Vantage Point (EQS)

void FNPCFindVantagePointTask::RunQuery(FInstanceDataType& InstanceData) const
{
	FEnvQueryRequest QueryRequest(InstanceData.Query, InstanceData.Actor);

	// el callback sólo captura el resultado compartido, así es seguro aunque el estado ya haya terminado
	TSharedPtr<FNPCVantageQueryResult> SharedResult = InstanceData.Result;

	InstanceData.QueryRequestId = QueryRequest.Execute(InstanceData.RunMode,
		FQueryFinishedSignature::CreateLambda([SharedResult](TSharedPtr<FEnvQueryResult> QueryResult)
		{
			if (QueryResult.IsValid() && QueryResult->IsSuccessful() && QueryResult->Items.Num() > 0)
			{
				SharedResult->Location = QueryResult->GetItemAsLocation(0);
				SharedResult->Version++;
			}
		}));
}

EStateTreeRunStatus FNPCFindVantagePointTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.Actor || !InstanceData.Query)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.Result = MakeShared<FNPCVantageQueryResult>();
	InstanceData.LastVersion = 0;
	InstanceData.bHasResult = false;
	InstanceData.RequeryRemaining = InstanceData.RequeryInterval;

	RunQuery(InstanceData);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FNPCFindVantagePointTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// ¿llegó un resultado nuevo? -> publicarlo en la salida (la leen las tareas enlazadas)
	if (InstanceData.Result.IsValid() && InstanceData.Result->Version != InstanceData.LastVersion)
	{
		InstanceData.LastVersion = InstanceData.Result->Version;
		InstanceData.VantagePoint = InstanceData.Result->Location;
		InstanceData.bHasResult = true;
	}

	// re-evaluar periódicamente porque el jugador se mueve
	InstanceData.RequeryRemaining -= DeltaTime;
	if (InstanceData.RequeryRemaining <= 0.0f)
	{
		InstanceData.RequeryRemaining = InstanceData.RequeryInterval;
		RunQuery(InstanceData);
	}

	if (InstanceData.bHasResult)
	{
		DrawDebugSphere(InstanceData.Actor->GetWorld(), InstanceData.VantagePoint, 30.0f, 12, FColor::Yellow);
	}

	return EStateTreeRunStatus::Running;
}

void FNPCFindVantagePointTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.QueryRequestId != INDEX_NONE && InstanceData.Actor)
	{
		if (UEnvQueryManager* QueryManager = UEnvQueryManager::GetCurrent(InstanceData.Actor))
		{
			QueryManager->AbortQuery(InstanceData.QueryRequestId);
		}
	}

	InstanceData.QueryRequestId = INDEX_NONE;
	InstanceData.Result.Reset();
}

#if WITH_EDITOR
FText FNPCFindVantagePointTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	const FInstanceDataType* InstanceData = InstanceDataView.GetPtr<FInstanceDataType>();
	return FText::FromString(FString::Printf(TEXT("<b>Find Vantage Point</b> %s"), *GetNameSafe(InstanceData ? InstanceData->Query.Get() : nullptr)));
}
#endif

////////////////////////////////////////////////////////////////////
// NPC: Move To Point

EStateTreeRunStatus FNPCMoveToPointTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.Actor || !InstanceData.AIController)
	{
		return EStateTreeRunStatus::Failed;
	}

	ANPCCharacter* NPC = Cast<ANPCCharacter>(InstanceData.Actor);
	if (NPC)
	{
		NPC->SetMoveSpeed(NPC->ChaseSpeed);
	}

	// mirar al jugador: la vista de la percepción sigue la rotación de control, así no lo pierde al reposicionarse
	if (InstanceData.bFacePlayer)
	{
		InstanceData.AIController->SetFocus(NPCStateTree::GetPlayerPawn(InstanceData.Actor), EAIFocusPriority::Gameplay);
		if (NPC)
		{
			NPC->SetFaceFocusMode(true);
		}
	}

	InstanceData.bHasRequested = false;

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FNPCMoveToPointTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ANPCCharacter* NPC = Cast<ANPCCharacter>(InstanceData.Actor);
	AAIController* Controller = InstanceData.AIController;
	if (!NPC || !Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	// Destination llega por Property Binding desde la tarea EQS; ZeroVector = todavía sin resultado
	const bool bValid = InstanceData.bDestinationValid && !InstanceData.Destination.IsNearlyZero();

	if (!bValid)
	{
		NPC->ShowStateText(TEXT("TACTICO: buscando punto (EQS)..."), FColor::Yellow);
		return EStateTreeRunStatus::Running;
	}

	const bool bDestinationChanged = !InstanceData.bHasRequested
		|| FVector::Dist(InstanceData.Destination, InstanceData.LastRequestedDestination) > InstanceData.RepathDistance;

	if (bDestinationChanged)
	{
		const EPathFollowingRequestResult::Type Result = Controller->MoveToLocation(InstanceData.Destination, InstanceData.AcceptanceRadius);
		InstanceData.bHasRequested = Result != EPathFollowingRequestResult::Failed;
		InstanceData.LastRequestedDestination = InstanceData.Destination;
	}

	if (Controller->GetMoveStatus() == EPathFollowingStatus::Moving)
	{
		NPC->ShowStateText(TEXT("TACTICO: moviendo a punto de ventaja"), FColor::Yellow);
	}
	else
	{
		NPC->ShowStateText(TEXT("TACTICO: en posicion"), FColor::Yellow);
	}

	return EStateTreeRunStatus::Running;
}

void FNPCMoveToPointTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.AIController)
	{
		InstanceData.AIController->StopMovement();
		InstanceData.AIController->ClearFocus(EAIFocusPriority::Gameplay);
	}

	if (ANPCCharacter* NPC = Cast<ANPCCharacter>(InstanceData.Actor))
	{
		NPC->SetMoveSpeed(NPC->PatrolSpeed);
		NPC->SetFaceFocusMode(false);
	}
}

#if WITH_EDITOR
FText FNPCMoveToPointTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString(TEXT("<b>Move To</b> bound point"));
}
#endif
