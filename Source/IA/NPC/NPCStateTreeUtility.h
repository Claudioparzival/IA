// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeConditionBase.h"

#include "NPCStateTreeUtility.generated.h"

class AAIController;

/**
 *  Las propiedades con Category = "Context" se enlazan (Property Binding) automáticamente
 *  con el Context Data del schema "StateTree AI Component":
 *    - Actor        -> el Pawn controlado (NPC)
 *    - AIController -> el ANPCAIController
 */

////////////////////////////////////////////////////////////////////
// CONDICIÓN: jugador dentro de un radio

USTRUCT()
struct FNPCPlayerInRangeConditionInstanceData
{
	GENERATED_BODY()

	/** Pawn del NPC (Context Data: Actor) */
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor;

	/** Radio de detección */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (Units = "cm", ClampMin = 0))
	float Radius = 800.0f;

	/** Si es true la condición pasa cuando el jugador está FUERA del radio */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bInvert = false;
};

USTRUCT(DisplayName = "NPC: Player In Range", meta = (Category = "NPC"))
struct FNPCPlayerInRangeCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FNPCPlayerInRangeConditionInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};

////////////////////////////////////////////////////////////////////
// TAREA: Patrullar

USTRUCT()
struct FNPCPatrolTaskInstanceData
{
	GENERATED_BODY()

	/** Pawn del NPC (Context Data: Actor) */
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor;

	/** Controlador del NPC (Context Data: AIController) */
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> AIController;

	/** Segundos de espera en cada punto */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (Units = "s", ClampMin = 0))
	float WaitTime = 2.0f;

	/** Distancia para considerar que llegó al punto */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (Units = "cm", ClampMin = 0))
	float AcceptanceRadius = 50.0f;

	/** Tiempo de espera restante */
	float WaitRemaining = 0.0f;

	/** true mientras camina hacia un punto */
	bool bMoving = false;
};

USTRUCT(meta = (DisplayName = "NPC: Patrol", Category = "NPC"))
struct FNPCPatrolTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FNPCPatrolTaskInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};

////////////////////////////////////////////////////////////////////
// TAREA: Perseguir al jugador

USTRUCT()
struct FNPCChasePlayerTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor;

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> AIController;

	/** Distancia a la que se detiene del jugador */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (Units = "cm", ClampMin = 0))
	float AcceptanceRadius = 100.0f;

	/** Cada cuánto se recalcula el camino */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (Units = "s", ClampMin = 0.05))
	float RepathInterval = 0.25f;

	float RepathRemaining = 0.0f;
};

USTRUCT(meta = (DisplayName = "NPC: Chase Player", Category = "NPC"))
struct FNPCChasePlayerTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FNPCChasePlayerTaskInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};

////////////////////////////////////////////////////////////////////
// TAREA: Saludar / interactuar con el jugador

USTRUCT()
struct FNPCGreetPlayerTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor;

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> AIController;

	/** Texto que dice el NPC */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FString Message = TEXT("Hola, jugador!");

	/** Duración de la interacción; al terminar la tarea devuelve Succeeded */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (Units = "s", ClampMin = 0))
	float Duration = 3.0f;

	float TimeRemaining = 0.0f;
};

USTRUCT(meta = (DisplayName = "NPC: Greet Player", Category = "NPC"))
struct FNPCGreetPlayerTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FNPCGreetPlayerTaskInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};
