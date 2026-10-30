// Copyright Epic Games, Inc. All Rights Reserved.

#include "IAGameMode.h"
#include "IAPlayerState.h"

AIAGameMode::AIAGameMode()
{
	// Use the PlayerState that owns the AbilitySystemComponent
	PlayerStateClass = AIAPlayerState::StaticClass();
}
