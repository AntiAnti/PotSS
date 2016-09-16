// Fill out your copyright notice in the Description page of Project Settings.

#include "PotSS.h"
#include "PlayerPawn.h"
#include "PlayerCharacter.h"
#include "Rifle.h"
#include "SlidingPawn.h"
#include "PlayerFirstPersonController.h"

APlayerFirstPersonController::APlayerFirstPersonController()
{
	PrimaryActorTick.bCanEverTick = true;
	OwnerCharacter = nullptr;

	//SetActorTickEnabled(false);
	bBlockInput = false;
	bSlideCamera = false;
	bUseDirectTeleportControl = false;
}

void APlayerFirstPersonController::BeginPlay()
{
	Super::BeginPlay();

	bIsThirdPersonView = false;

	_REFRESH_PAWN(OwnerPawn)
	bWTFSkipSliding = true;
}

void APlayerFirstPersonController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bSlideCamera) {
		const float Speed = 2.0f;// 0.2f;

		if (!IsValid(SlidingPawn)) {
			bSlideCamera = false;
		}
		else {
			/*
			if (FMath::Abs(OwnerCharacter->GetControlRotation().Yaw) > 0.1f) {
				float y = OwnerCharacter->GetControlRotation().Yaw;
				float diff = y > 0.0f ? -1.5f * DeltaTime : 1.5f * DeltaTime;

				if (y + diff < 0.5f) {
					diff = y;
					if (bIsThirdPersonView) {
						CurrentCamPoint = OwnerCharacter->GetFollowCamera()->GetComponentLocation();
						DestinationCamPoint = OwnerPawn->VRCamera->GetComponentLocation();
					}
					else {
						DestinationCamPoint = OwnerCharacter->GetFollowCamera()->GetComponentLocation();
						CurrentCamPoint = OwnerPawn->VRCamera->GetComponentLocation();
					}
					CamMovingProgress = 0.0f;
				}
				OwnerCharacter->GetController()->SetControlRotation(FRotator(0.0f, y + diff, 0.0f));
			}
			else {
			*/
				CamMovingProgress += DeltaTime / Speed;
				FVector delta = DestinationCamPoint - CurrentCamPoint;
				FVector dest = CurrentCamPoint + delta * CamMovingProgress;
				//FRotator rot = FMath::RInterpTo(CurrentCamRotation, DestinationCamRotation, CamMovingProgress * Speed, 1.0f / Speed);

				if (CamMovingProgress >= 1.0f) {
					CamMovingProgress = 0.0f;
					dest = DestinationCamPoint;
					//rot = DestinationCamRotation;
					bSlideCamera = false;
				}
				//SlidingPawn->SetActorRotation(rot);
				SlidingPawn->CameraRoot->SetWorldLocation(dest - SlidingPawn->MainCamera->GetRelativeTransform().GetTranslation());
				//SlidingPawn->SetActorLocation(dest);
			//}
		}

		if (!bSlideCamera) {
			if (bIsThirdPersonView) {
				this->UnPossess();
				this->Possess(OwnerCharacter);
			}
			else {
				this->UnPossess();
				this->Possess(OwnerPawn);
			}
			EnableInput(this);
		}
	}
}

void APlayerFirstPersonController::SetThirdPersonView()
{
	if (!bIsThirdPersonView) {
		FTransform tr; FVector loc; FRotator rot;

		// Character actor location setup
		tr = OwnerPawn->VRCamera->GetComponentTransform();
		loc = OwnerPawn->VRCamera->GetRelativeTransform().GetTranslation() + OwnerPawn->GetActorLocation(); //->GetComponentLocation();
		loc.Z = OwnerPawn->GetActorLocation().Z + 68.0f;
		tr.SetTranslation(loc);
		rot = FRotator::ZeroRotator; //FRotator(0.0f, tr.GetRotation().Rotator().Yaw, 0.0f);
		tr.SetRotation(FRotator::ZeroRotator.Quaternion()); // rot.Quaternion());

		// Character actor spawn (if necessary)
		if (!IsValid(OwnerCharacter)) {
			OwnerCharacter = SpawnThirdPersonCharacter(tr);
		}

		// Set character actor transform
		OwnerCharacter->SetActorLocationAndRotation(loc, FRotator::ZeroRotator);
		if (IsValid(OwnerCharacter->GetController())) {
			OwnerCharacter->GetController()->SetControlRotation(FRotator::ZeroRotator);
		}

		// Sliding params setup
		CurrentCamPoint = OwnerPawn->VRCamera->GetComponentLocation();
		CurrentCamRotation = OwnerPawn->GetActorRotation();
		DestinationCamPoint = OwnerCharacter->GetFollowCamera()->GetComponentLocation();
		DestinationCamRotation = FRotator(0.0f, OwnerCharacter->GetFollowCamera()->GetComponentRotation().Yaw, 0.0f);
		CamMovingProgress = 0.0f;

		// Sliding actor setup
		InitSlidingPawn(OwnerPawn->VRCamera->GetComponentTransform());
		SlidingPawn->SetActorLocation(OwnerPawn->GetActorLocation());
		SlidingPawn->SetActorRotation(FRotator::ZeroRotator);
		SlidingPawn->CameraRoot->SetRelativeLocation(FVector::ZeroVector);
		OwnerPawn->SetActive(false);
		OwnerCharacter->SetActive(true, IsValid(OwnerPawn->CurrentRifle));

		//SetControlRotation(FRotator(0.0f, -rot.Yaw, 0.0f));
		this->UnPossess();
		if (bWTFSkipSliding) {
			this->Possess(OwnerCharacter);
			bWTFSkipSliding = false;
		}
		else {
			this->Possess(SlidingPawn);
			DisableInput(this);
			bSlideCamera = true;
		}
		//OwnerCharacter->SetActorRotation(rot);
		//SetControlRotation(FRotator(0.0f, rot.Yaw, 0.0f)); // перенесет камеру на нужное место относительно павна, но заодно развернет :(
		//OwnerCharacter->AddControllerYawInput(rot.Yaw);

		bIsThirdPersonView = true;
	}
}

void APlayerFirstPersonController::SetFirstPersonView()
{
	if (bIsThirdPersonView) {
		FVector a, b, c;
		a = OwnerCharacter->GetMesh()->GetComponentLocation();
		b = OwnerPawn->VRCamera->GetRelativeTransform().GetTranslation();
		c = a - b; c.Z = OwnerCharacter->GetActorLocation().Z - 68.0f;
		OwnerPawn->SetActorLocation(c);

		OwnerPawn->ActivateNavigationFinder(false);
		OwnerCharacter->SetActive(false);
		OwnerPawn->SetActive(true);

		CurrentCamPoint = OwnerCharacter->GetFollowCamera()->GetComponentLocation(); 
		CurrentCamRotation = OwnerCharacter->GetControlRotation(); // GetFollowCamera()->GetComponentRotation();
		DestinationCamPoint = OwnerPawn->VRCamera->GetComponentLocation();
		DestinationCamRotation = FRotator::ZeroRotator; // OwnerPawn->VRCamera->GetComponentRotation();
		CamMovingProgress = 0.0f;

		InitSlidingPawn(OwnerCharacter->GetFollowCamera()->GetComponentTransform());
		SlidingPawn->SetActorLocation(OwnerPawn->GetActorLocation());
		SlidingPawn->SetActorRotation(FRotator::ZeroRotator);

		this->UnPossess();
		this->Possess(SlidingPawn);
		DisableInput(this);
		bSlideCamera = true;
		/*
		this->UnPossess();
		this->Possess(OwnerPawn);
		*/

		bIsThirdPersonView = false;
	}
}

APawn* APlayerFirstPersonController::GetActivePawn()
{
	if (bIsThirdPersonView && IsValid(OwnerCharacter)) {
		return OwnerCharacter;
	}
	else if (IsValid(OwnerPawn)) {
		return OwnerPawn->GetActivePawn();
	}
	else {
		return nullptr;
	}
}

FVector APlayerFirstPersonController::GetActivePawnHead()
{
	if (bIsThirdPersonView && IsValid(OwnerCharacter))
		return OwnerCharacter->GetMesh()->GetSocketLocation(TEXT("head"));
	else if (IsValid(OwnerPawn)) {
		return OwnerPawn->GetActivePawnHead();
	}
	else {
		return FVector::ZeroVector;
	}
}

void APlayerFirstPersonController::SetUsingDirectTeleportControl(bool State)
{
	bUseDirectTeleportControl = State;
	if (IsValid(OwnerPawn)) OwnerPawn->bUseDirectTeleportControl = State;
}

void APlayerFirstPersonController::InitSlidingPawn(FTransform tr)
{
	if (!IsValid(SlidingPawn)) {
		FActorSpawnParameters spp;
		spp.bNoFail = true;
		SlidingPawn = GetWorld()->SpawnActor<ASlidingPawn>(ASlidingPawn::StaticClass(), tr, spp);
	}
	SlidingPawn->SetActorTransform(tr);
}