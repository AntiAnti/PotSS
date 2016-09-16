// Fill out your copyright notice in the Description page of Project Settings.

#include "PotSS.h"
#include "PlayerPawn.h"
#include "AIController.h"
#include "Rifle.h"
#include "PlayerJumpingPawn.h"


// Sets default values
APlayerJumpingPawn::APlayerJumpingPawn()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -90.0f), FRotator(0.0f, -90.0f, 0.0f));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> skmBody(TEXT("SkeletalMesh'/Game/Mannequin/Character/Mesh/Mannequin_SKM.Mannequin_SKM'"));
	if (skmBody.Object != NULL) GetMesh()->SetSkeletalMesh(skmBody.Object);
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	/*
	static ConstructorHelpers::FClassFinder<UAnimInstance> abpBody(TEXT("/Game/Mannequin/Animations/ThirdPerson_AnimBP'"));
	if (abpBody.Class != NULL) {
		GetMesh()->SetAnimInstanceClass(abpBody.Class);
	}
	*/

	/*
	UAnimBlueprintGeneratedClass* armAnimBPGC = Cast<UAnimInstance>(StaticLoadObject(UAnimInstance::StaticClass(), NULL, TEXT("SkeletalMesh'/Game/Mannequin/Character/Mesh/SK_Mannequin.SK_Mannequin'")));
	if (armAnimBPGC)
	{
		GetMesh()->AnimClass = armAnimBPGC;
	}
	*/
	bUseDirectTeleportControl = false;
}

// Called when the game starts or when spawned
void APlayerJumpingPawn::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APlayerJumpingPawn::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

// Called to bind functionality to input
void APlayerJumpingPawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	check(InputComponent);

	//InputComponent->BindAxis("MoveForwardLeft", this, &APlayerJumpingPawn::MoveForward);
	//InputComponent->BindAxis("TurnRateRight", this, &APlayerJumpingPawn::TurnAtRate);
}

void APlayerJumpingPawn::MoveForward(float Value)
{
	if (bUseDirectTeleportControl && !GetMovingState()) {
		if ((Controller != NULL) && (Value != 0.0f))
		{
			// find out which way is forward
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get forward vector
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			AddMovementInput(Direction, Value);
		}
	}
}

void APlayerJumpingPawn::TurnAtRate(float Rate)
{
	const float BaseTurnRate = 45.0f;
	if (bUseDirectTeleportControl) {
		AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
}

void APlayerJumpingPawn::ActivatePawn(APlayerPawn* Player, FVector DestinationPoint, FVector CurrentCameraPoint)
{
	GetMesh()->GlobalAnimRateScale = 1.0f;
	this->SetActorHiddenInGame(false);

	if (IsValid(Player)) {
		MasterPawn = Player;
		bUseDirectTeleportControl = Player->bUseDirectTeleportControl;
		FVector loc = CurrentCameraPoint; // MasterPawn->VRCamera->GetComponentLocation();
		FRotator rot = MasterPawn->VRCamera->GetComponentRotation();
		FVector loc0 = MasterPawn->VRCamera->GetRelativeTransform().GetTranslation();
		FVector inc = (DestinationPoint - loc);
		inc.Normalize(); inc.Z = 0.0f; inc *= 50.0f;
		float k = (loc0.Z + 10.0f) / (88.0f * 2.0f); // GetCapsuleComponent()->GetUnscaledCapsuleSize();

		loc.Z = MasterPawn->GetActorLocation().Z + (88.0f * 2.0f * k) / 2.0f;
		rot.Pitch = 0.0f;
		rot.Roll = 0.0f;

		SetActorLocation(loc + inc);
		SetActorRotation(rot);
		SetActorScale3D(FVector(k, k, k));

		if (IsValid(CurrentRifle)) {
			CurrentRifle->SetActorHiddenInGame(!IsValid(MasterPawn->CurrentRifle));
		}

		if (!bUseDirectTeleportControl) {
			StartRunning(DestinationPoint);
		}
	}
}

void APlayerJumpingPawn::DeactivatePawn()
{
	GetMesh()->GlobalAnimRateScale = 0.0f;
	this->SetActorHiddenInGame(true);

	if (IsValid(CurrentRifle)) {
		CurrentRifle->SetActorHiddenInGame(true);
	}
}

void APlayerJumpingPawn::StartRunning_Implementation(FVector Destination)
{
	AAIController* ctrl = Cast<AAIController>(GetController());

	IsMoving = true;
	/*
	if (bUseDirectTeleportControl) {
		IsMoving = true;
	}*/

	if (IsValid(ctrl)) {
		ctrl->MoveToLocation(Destination, 10.0f, false, true, true);
	}
}

void APlayerJumpingPawn::MoveByMasterPawn(FVector Destination)
{
	AAIController* ctrl = Cast<AAIController>(GetController());

	if (IsValid(ctrl)) {
		ctrl->MoveToLocation(Destination, 5.0f, false, true, true);
	}
}

void APlayerJumpingPawn::OnAnyMovingComplete()
{
	IsMoving = false;
	AAIController* ctrl = Cast<AAIController>(GetController());
	FVector diff = GetActorLocation() - MasterPawn->VRCamera->GetComponentLocation();
	diff.Z = 0.0f;
	diff.Normalize();
	//ctrl->SetControlRotation(diff.Rotation());
	AddControllerYawInput(diff.Rotation().Yaw - GetControlRotation().Yaw);
}

void APlayerJumpingPawn::SetRifle(ARifle* NewRifle, EHandSide Hand)
{
	CurrentRifle = NewRifle;
	CurrentRifle->AttachToTeleportingPawn(GetMesh(), Hand);
	CurrentRifle->SetActorHiddenInGame(true);
	RifleMainHand = Hand;
}

FRotator APlayerJumpingPawn::GetRifleRotation()
{
	if (IsValid(CurrentRifle))
		return CurrentRifle->GetActorRotation();
	else
		return FRotator::ZeroRotator;
}

FTransform APlayerJumpingPawn::GetRifleSocketTransform()
{
	FTransform tr;
	if (IsValid(CurrentRifle))
	{
		if (RifleMainHand == EHandSide::Right)
			tr = CurrentRifle->GetStaticMeshComponent()->GetSocketTransform(SOCKET_SECOND_HAND_L);
		else
			tr = CurrentRifle->GetStaticMeshComponent()->GetSocketTransform(SOCKET_SECOND_HAND_R);
	}
	return tr;
}
