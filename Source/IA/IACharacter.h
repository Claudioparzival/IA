// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "GameplayTagContainer.h"
#include "IACharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UAIPerceptionStimuliSourceComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class AIACharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Makes the player perceivable by the NPC sight sense */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UAIPerceptionStimuliSourceComponent* StimuliSource;
	
protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	/** Sprint Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* SprintAction;

public:

	/** Constructor */
	AIACharacter();	

protected:

	/** Registers the stimuli source for AI perception */
	virtual void BeginPlay() override;

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	/** Adds the sprinting tag to the ASC, unless the character is stunned */
	UFUNCTION(BlueprintCallable, Category="Input")
	void StartSprinting();

	/** Removes the sprinting tag from the ASC */
	UFUNCTION(BlueprintCallable, Category="Input")
	void StopSprinting();

	/** Returns true if the ASC owns the given tag */
	UFUNCTION(BlueprintCallable, Category="Input")
	bool HasStateTag(FGameplayTag TagToCheck) const;

	/** Adds the State.Debuff.Stunned tag if bIsStunned is true, removes it otherwise */
	UFUNCTION(BlueprintCallable, Category="State")
	void ToggleStun(bool bIsStunned);

protected:

	/** Server: initializes the ASC actor info */
	virtual void PossessedBy(AController* NewController) override;

	/** Client: initializes the ASC actor info */
	virtual void OnRep_PlayerState() override;

	/** Sets the PlayerState as owner and this character as avatar of the ASC */
	void InitAbilityActorInfo();

private:

	//Tag Cache
	FGameplayTag SprintingTag;

	FGameplayTag StunnedTag; //new tag for the stunned debuff.

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

