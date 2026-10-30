// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeConditionBase.h"
#include "EnvironmentQuery/EnvQueryTypes.h"

#include "NPCStateTreeUtility.generated.h"

class AAIController;
class UEnvQuery;

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

////////////////////////////////////////////////////////////////////
// TAREA: Buscar punto de ventaja con EQS

/** Resultado compartido entre la tarea y el callback asíncrono del EQS */
struct FNPCVantageQueryResult
{
	/** Se incrementa cada vez que llega un resultado válido */
	int32 Version = 0;
	FVector Location = FVector::ZeroVector;
};

USTRUCT()
struct FNPCFindVantagePointTaskInstanceData
{
	GENERATED_BODY()

	/** Pawn del NPC: es el Querier del EQS */
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor;

	/** EQS Query a ejecutar (ej. EQ_VantagePoint) */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	TObjectPtr<UEnvQuery> Query;

	/** Modo de ejecución: mejor punto, o aleatorio entre los mejores */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	TEnumAsByte<EEnvQueryRunMode::Type> RunMode = EEnvQueryRunMode::SingleResult;

	/** Cada cuánto se vuelve a ejecutar la consulta (el jugador se mueve) */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (Units = "s", ClampMin = 0.1))
	float RequeryInterval = 1.0f;

	/** SALIDA: mejor punto encontrado por el EQS. Enlázalo (Property Binding) al Destination de "NPC: Move To Point" */
	UPROPERTY(EditAnywhere, Category = "Output")
	FVector VantagePoint = FVector::ZeroVector;

	/** SALIDA: true cuando ya hay al menos un resultado válido */
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasResult = false;

	float RequeryRemaining = 0.0f;
	int32 QueryRequestId = INDEX_NONE;
	int32 LastVersion = 0;
	TSharedPtr<FNPCVantageQueryResult> Result;
};

USTRUCT(meta = (DisplayName = "NPC: Find Vantage Point (EQS)", Category = "NPC"))
struct FNPCFindVantagePointTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FNPCFindVantagePointTaskInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif

private:

	/** Lanza la consulta EQS asíncrona */
	void RunQuery(FInstanceDataType& InstanceData) const;
};

////////////////////////////////////////////////////////////////////
// TAREA: Moverse a un punto (recibido por Property Binding)

USTRUCT()
struct FNPCMoveToPointTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor;

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> AIController;

	/** ENTRADA: destino. Debe enlazarse a "VantagePoint" de la tarea EQS */
	UPROPERTY(EditAnywhere, Category = "Input")
	FVector Destination = FVector::ZeroVector;

	/** ENTRADA (opcional): si está enlazado a "bHasResult", no se mueve hasta que el EQS responda */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bDestinationValid = true;

	/** Distancia para considerar que llegó */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (Units = "cm", ClampMin = 0))
	float AcceptanceRadius = 50.0f;

	/** Mantener la mirada (y la vista de la percepción) en el jugador mientras se reposiciona */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bFacePlayer = true;

	/** Sólo se pide un camino nuevo si el destino cambió más que esto */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (Units = "cm", ClampMin = 0))
	float RepathDistance = 100.0f;

	FVector LastRequestedDestination = FVector::ZeroVector;
	bool bHasRequested = false;
};

USTRUCT(meta = (DisplayName = "NPC: Move To Point", Category = "NPC"))
struct FNPCMoveToPointTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FNPCMoveToPointTaskInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};
