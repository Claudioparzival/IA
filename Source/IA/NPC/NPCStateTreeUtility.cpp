// Fill out your copyright notice in the Description page of Project Settings.

#include "NPCStateTreeUtility.h"
#include "NPCCharacter.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

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
