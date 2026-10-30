// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "IAPlayerState.generated.h"

class UAbilitySystemComponent;

/**
 *  PlayerState that owns the player's AbilitySystemComponent.
 *  The ASC lives here so the gameplay tags persist across respawns.
 */
UCLASS()
class AIAPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

	/** Ability System Component that holds the player's state tags */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Abilities", meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystemComponent;

public:

	/** Constructor */
	AIAPlayerState();

	/** IAbilitySystemInterface */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
};
