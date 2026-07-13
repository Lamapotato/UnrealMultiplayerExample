// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "UMECharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UAnimMontage;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class AUMECharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** Next server time when the kick can be used */
	double NextKickAllowedServerTime = 0.0;
	
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
	
	/** Kick Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* KickAction;
	
	/** Time between kick attempts */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Kick",
		meta = (ClampMin = "0.0")
	)
	float KickCooldownSeconds = 1.0f;
	
	/** Animation played when the kick is accepted by the server */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Kick|Animation"
	)
	UAnimMontage* KickMontage = nullptr;

	/** Playback rate of the kick montage */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Kick|Animation",
		meta = (ClampMin = "0.01")
	)
	float KickMontagePlayRate = 1.0f;
	
	/** Maximum kick reach in centimeters */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Kick|Trace",
		meta = (ClampMin = "0.0")
	)
	float KickDistance = 180.0f;

	/** Radius of the kick sweep */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Kick|Trace",
		meta = (ClampMin = "0.0")
	)
	float KickRadius = 45.0f;

	/** Forward offset from the character origin */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Kick|Trace",
		meta = (ClampMin = "0.0")
	)
	float KickStartOffset = 40.0f;

	/** Collision channel used by the kick */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Kick|Trace"
	)
	TEnumAsByte<ECollisionChannel> KickTraceChannel = ECC_Visibility;

	/** Draw the kick sweep for debugging */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Kick|Debug"
	)
	bool bDrawKickDebug = true;

	/** Debug visualization lifetime */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Kick|Debug",
		meta = (ClampMin = "0.0")
	)
	float KickDebugDuration = 2.0f;
	
	/** Horizontal launch velocity applied to kicked characters */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Kick|Force",
		meta = (ClampMin = "0.0")
	)
	float CharacterKickStrength = 900.0f;

	/** Vertical launch velocity applied to kicked characters */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Kick|Force",
		meta = (ClampMin = "0.0")
	)
	float CharacterKickUpwardStrength = 200.0f;

	/** Impulse applied to physics-simulated components */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Kick|Force",
		meta = (ClampMin = "0.0")
	)
	float PhysicsKickImpulse = 100000.0f;
	
public:

	/** Constructor */
	AUMECharacter();	

protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	
	/** Called for kick input */
	void Kick();
	
	/** Sends the kick request to the server */
	UFUNCTION(Server, Reliable)
	void ServerKick();
	
	/** Plays kick animation on the server and relevant clients */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayKickMontage();

	/** Plays the kick montage on this character instance */
	void PlayKickMontage();

	/** Processes an authoritative kick request */
	void HandleKickOnServer();
	
	/** Performs an authoritative kick sweep */
	void PerformKickTrace();
	
	/** Applies the kick effect to the hit target */
	void ApplyKickToHit(
		const FHitResult& HitResult,
		const FVector& KickDirection
	);

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

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

