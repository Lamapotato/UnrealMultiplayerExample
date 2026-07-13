// Copyright Epic Games, Inc. All Rights Reserved.

#include "UMECharacter.h"
#include "Animation/AnimMontage.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "UME.h"

AUMECharacter::AUMECharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AUMECharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUMECharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AUMECharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AUMECharacter::Look);
		
		// Kicking
		EnhancedInputComponent->BindAction(
			KickAction,
			ETriggerEvent::Started,
			this,
			&AUMECharacter::Kick
		);
	}
	else
	{
		UE_LOG(LogUME, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AUMECharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AUMECharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AUMECharacter::Kick()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	UE_LOG(
		LogUME,
		Log,
		TEXT("[LOCAL] Kick requested by %s"),
		*GetNameSafe(this)
	);

	if (HasAuthority())
	{
		HandleKickOnServer();
		return;
	}

	ServerKick();
}

void AUMECharacter::ServerKick_Implementation()
{
	HandleKickOnServer();
}

void AUMECharacter::MulticastPlayKickMontage_Implementation()
{
	PlayKickMontage();
}

void AUMECharacter::PlayKickMontage()
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (!IsValid(KickMontage))
	{
		UE_LOG(
			LogUME,
			Warning,
			TEXT("[KICK] KickMontage is not assigned for %s"),
			*GetNameSafe(this)
		);

		return;
	}

	const float MontageDuration = PlayAnimMontage(
		KickMontage,
		KickMontagePlayRate
	);

	if (MontageDuration <= 0.0f)
	{
		UE_LOG(
			LogUME,
			Warning,
			TEXT("[KICK] Failed to play KickMontage for %s"),
			*GetNameSafe(this)
		);
	}
}

void AUMECharacter::HandleKickOnServer()
{
	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();

	if (!IsValid(World))
	{
		return;
	}

	const double CurrentServerTime = World->GetTimeSeconds();

	if (CurrentServerTime < NextKickAllowedServerTime)
	{
		const double RemainingCooldown =
			NextKickAllowedServerTime - CurrentServerTime;

		UE_LOG(
			LogUME,
			Log,
			TEXT("[SERVER] Kick rejected for %s. Cooldown remaining: %.2f"),
			*GetNameSafe(this),
			RemainingCooldown
		);

		return;
	}

	NextKickAllowedServerTime =
	CurrentServerTime + KickCooldownSeconds;

	UE_LOG(
		LogUME,
		Display,
		TEXT("[SERVER] Kick accepted for %s"),
		*GetNameSafe(this)
	);

	MulticastPlayKickMontage();
	PerformKickTrace();
}

void AUMECharacter::PerformKickTrace()
{
	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();

	if (!IsValid(World))
	{
		return;
	}

	FVector KickDirection = GetActorForwardVector();
	KickDirection.Z = 0.0f;

	if (!KickDirection.Normalize())
	{
		return;
	}

	const FVector TraceStart =
		GetActorLocation() +
		KickDirection * KickStartOffset;

	const FVector TraceEnd =
		TraceStart +
		KickDirection * KickDistance;

	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(KickTrace),
		false,
		this
	);

	QueryParams.AddIgnoredActor(this);

	const FCollisionShape KickShape =
		FCollisionShape::MakeSphere(KickRadius);

	FHitResult HitResult;

	const bool bHasHit = World->SweepSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		FQuat::Identity,
		KickTraceChannel,
		KickShape,
		QueryParams
	);

	if (bDrawKickDebug)
	{
		const FColor DebugColor =
			bHasHit ? FColor::Green : FColor::Red;

		DrawDebugLine(
			World,
			TraceStart,
			TraceEnd,
			DebugColor,
			false,
			KickDebugDuration,
			0,
			2.0f
		);

		DrawDebugSphere(
			World,
			TraceStart,
			KickRadius,
			16,
			DebugColor,
			false,
			KickDebugDuration
		);

		DrawDebugSphere(
			World,
			TraceEnd,
			KickRadius,
			16,
			DebugColor,
			false,
			KickDebugDuration
		);

		if (bHasHit)
		{
			DrawDebugPoint(
				World,
				HitResult.ImpactPoint,
				15.0f,
				FColor::Yellow,
				false,
				KickDebugDuration
			);
		}
	}

	if (!bHasHit)
	{
		UE_LOG(
			LogUME,
			Log,
			TEXT("[SERVER] Kick missed for %s"),
			*GetNameSafe(this)
		);

		return;
	}

	UE_LOG(
		LogUME,
		Display,
		TEXT(
			"[SERVER] Kick hit Actor: %s, Component: %s, Location: %s"
		),
		*GetNameSafe(HitResult.GetActor()),
		*GetNameSafe(HitResult.GetComponent()),
		*HitResult.ImpactPoint.ToCompactString()
	);
	
	ApplyKickToHit(HitResult, KickDirection);
}

void AUMECharacter::ApplyKickToHit(
	const FHitResult& HitResult,
	const FVector& KickDirection
)
{
	if (!HasAuthority())
	{
		return;
	}

	AActor* HitActor = HitResult.GetActor();
	UPrimitiveComponent* HitComponent = HitResult.GetComponent();

	if (!IsValid(HitActor) || !IsValid(HitComponent))
	{
		return;
	}

	if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
	{
		FVector LaunchDirection =
			HitCharacter->GetActorLocation() -
			GetActorLocation();

		LaunchDirection.Z = 0.0f;

		if (!LaunchDirection.Normalize())
		{
			LaunchDirection = KickDirection;
		}

		const FVector LaunchVelocity =
			LaunchDirection * CharacterKickStrength +
			FVector::UpVector * CharacterKickUpwardStrength;

		HitCharacter->LaunchCharacter(
			LaunchVelocity,
			true,
			true
		);

		UE_LOG(
			LogUME,
			Display,
			TEXT(
				"[SERVER] Character %s was kicked. Launch velocity: %s"
			),
			*GetNameSafe(HitCharacter),
			*LaunchVelocity.ToCompactString()
		);

		return;
	}

	if (!HitComponent->IsSimulatingPhysics(HitResult.BoneName))
	{
		UE_LOG(
			LogUME,
			Log,
			TEXT(
				"[SERVER] Actor %s was hit, but component %s does not simulate physics"
			),
			*GetNameSafe(HitActor),
			*GetNameSafe(HitComponent)
		);

		return;
	}

	FVector PhysicsDirection = KickDirection;
	PhysicsDirection.Z = 0.15f;

	if (!PhysicsDirection.Normalize())
	{
		return;
	}

	const FVector Impulse =
		PhysicsDirection * PhysicsKickImpulse;

	HitComponent->AddImpulseAtLocation(
		Impulse,
		HitResult.ImpactPoint,
		HitResult.BoneName
	);

	UE_LOG(
		LogUME,
		Display,
		TEXT(
			"[SERVER] Physics component %s was kicked. Impulse: %s"
		),
		*GetNameSafe(HitComponent),
		*Impulse.ToCompactString()
	);
}

void AUMECharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AUMECharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AUMECharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AUMECharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}
