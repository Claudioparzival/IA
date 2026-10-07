// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTreeTask_FindCover.h"
#include "StateTreeExecutionContext.h"          // Motor de ejecución que administra bindings y memoria de la tarea
#include "AIController.h"                       // Controlador base de IA en Unreal
#include "EnvironmentQuery/EnvQueryManager.h"   // Gestor global de consultas EQS

EStateTreeRunStatus FStateTreeTask_FindCover::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// 1. Obtener la memoria de variables mutable asignada a este agente en particular.
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	// 2. Crear un resultado nuevo en estado Running mientras el EQS procesa en segundo plano.
	InstanceData.Result = MakeShared<FStateTreeFindCoverResult>();

	// 3. Obtener y almacenar la referencia al AIController desde el contexto raíz.
	InstanceData.AIController = Cast<AAIController>(Context.GetOwner());

	// 4. Control de fallos preliminar: validar que exista asset de EQS y un AIController válido.
	if (!InstanceData.CoverQuery || !InstanceData.AIController)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 5. Validar que el AIController esté poseyendo un Pawn (necesario para ubicar el centro de búsqueda).
	APawn* ControlledPawn = InstanceData.AIController->GetPawn();
	if (!ControlledPawn)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 6. Configurar la petición al EQS pasando el asset y el Pawn que actúa como centro ("Querier").
	FEnvQueryRequest QueryRequest(InstanceData.CoverQuery, ControlledPawn);

	// 7. El callback sólo captura el resultado compartido (TSharedPtr), así es seguro aunque
	//    el estado ya haya terminado cuando el EQS responda.
	TSharedPtr<FStateTreeFindCoverResult> SharedResult = InstanceData.Result;

	// 8. Disparar la consulta asíncrona registrando una función Lambda como callback de finalización.
	InstanceData.QueryRequestId = QueryRequest.Execute(
		InstanceData.RunMode,
		FQueryFinishedSignature::CreateLambda([SharedResult](TSharedPtr<FEnvQueryResult> QueryResult)
			{
				// Verificar si el EQS arrojó resultados válidos (encontró cobertura válida en NavMesh).
				if (QueryResult.IsValid() && QueryResult->IsSuccessful())
				{
					// Tomar la ubicación elegida (índice 0) y marcar la tarea como Succeeded.
					SharedResult->Location = QueryResult->GetItemAsLocation(0);
					SharedResult->Status = EStateTreeRunStatus::Succeeded;
				}
				else
				{
					// No se encontraron puntos viables con los tests del EQS.
					SharedResult->Status = EStateTreeRunStatus::Failed;
				}
			})
	);

	// Notificar al StateTree que la tarea comenzó pero requiere esperar el resultado en ticks posteriores.
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FStateTreeTask_FindCover::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	// En cada frame, StateTree consulta el estado actual.
	// Mientras el EQS esté evaluando, devolverá Running.
	// Al terminar la Lambda, devolverá Succeeded o Failed, desbloqueando la transición del estado.
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if (!InstanceData.Result.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

	if (InstanceData.Result->Status == EStateTreeRunStatus::Succeeded)
	{
		// Copiar la ubicación a la salida para que otras tareas puedan enlazarla.
		InstanceData.CoverLocation = InstanceData.Result->Location;
	}

	return InstanceData.Result->Status;
}
