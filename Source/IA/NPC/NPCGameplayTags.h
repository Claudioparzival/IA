// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

/**
 *  Tags de los eventos que el NPCAIController envía al StateTree
 *  cuando la percepción (vista) detecta o pierde al jugador.
 */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Perception_PlayerSeen);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Perception_PlayerLost);
