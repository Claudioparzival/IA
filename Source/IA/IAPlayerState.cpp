// Fill out your copyright notice in the Description page of Project Settings.

#include "IAPlayerState.h"
#include "AbilitySystemComponent.h"

AIAPlayerState::AIAPlayerState()
{
	// Create the ASC and replicate it to the clients
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// The PlayerState updates slowly by default, the ASC needs it faster
	SetNetUpdateFrequency(100.0f);
}

UAbilitySystemComponent* AIAPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
