// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"                  // Definición base para structs de tareas de StateTree (FStateTreeTaskCommonBase)
#include "EnvironmentQuery/EnvQueryTypes.h"     // Tipos del EQS: EEnvQueryRunMode, resultados y peticiones
#include "StateTreeTask_FindCover.generated.h"

// Forward declarations para acelerar tiempos de compilación
class UEnvQuery;
class AAIController;

/**
 * Resultado compartido entre la tarea y el callback asíncrono del EQS.
 * Se usa un TSharedPtr para que el callback pueda escribir el resultado aunque
 * se ejecute varios frames después, sin tener que reconstruir el contexto del StateTree.
 */
struct FStateTreeFindCoverResult
{
	EStateTreeRunStatus Status = EStateTreeRunStatus::Running;
	FVector Location = FVector::ZeroVector;
};

/**
 * DATOS DE INSTANCIA (Instance Data):
 * StateTree sigue el patrón Flyweight: la struct de la tarea (FStateTreeTask_FindCover) es compartida
 * y de solo lectura. Todo estado mutable propio de cada agente se aloja en esta estructura.
 */
USTRUCT()
struct FStateTreeTask_FindCoverInstanceData
{
	GENERATED_BODY()

	// Asset del EQS (Environment Query) a evaluar; asignable en el panel de detalles del StateTree.
	UPROPERTY(EditAnywhere, Category = "Parameter")
	TObjectPtr<UEnvQuery> CoverQuery = nullptr;

	// Modo de ejecución de la consulta (SingleResult por defecto, o variantes de porcentaje para aleatoriedad).
	UPROPERTY(EditAnywhere, Category = "Parameter")
	TEnumAsByte<EEnvQueryRunMode::Type> RunMode = EEnvQueryRunMode::SingleResult;

	// Salida de la tarea: la coordenada 3D hallada por el EQS. Se puede enlazar (bind) a otras tareas (ej. MoveTo).
	UPROPERTY(EditAnywhere, Category = "Output")
	FVector CoverLocation = FVector::ZeroVector;

	// Puntero en caché al AIController dueño para evitar múltiples llamadas a Cast en tiempo de ejecución.
	UPROPERTY()
	TObjectPtr<AAIController> AIController = nullptr;

	// Identificador único de la petición EQS en curso devuelto por el EnvQueryManager.
	int32 QueryRequestId = INDEX_NONE;

	// Resultado escrito por el callback del EQS y consultado en cada Tick.
	TSharedPtr<FStateTreeFindCoverResult> Result;
};

/**
 * TAREA LIVIANA (Struct Task):
 * Hereda de FStateTreeTaskCommonBase. Al ser USTRUCT en lugar de UCLASS, no genera sobrecarga
 * de recolección de basura (Garbage Collector) y almacena sus datos contiguos en memoria.
 */
USTRUCT(meta = (DisplayName = "Find Cover (EQS)", Category = "AI|Cover"))
struct IA_API FStateTreeTask_FindCover : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	// Define el alias canónico de datos de instancia requerido internamente por StateTree.
	using FInstanceDataType = FStateTreeTask_FindCoverInstanceData;

	// Retorna la reflexión de la struct que contiene las variables del agente.
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	// Se llama al entrar al estado en el que reside esta tarea. Dispara la consulta asíncrona.
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	// Se llama cada frame mientras EnterState o Tick retornen EStateTreeRunStatus::Running.
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
