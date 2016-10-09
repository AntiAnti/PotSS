// Fill out your copyright notice in the Description page of Project Settings.

#include "PotSS.h"
/******************* __SteamVR_Support__ *******************/
#include "MotionControllerComponent.h"
#include "SteamVRChaperoneComponent.h"
/***********************************************************/
#include "Components/ArrowComponent.h"
#include "PlayerFirstPersonController.h"
#include "NavigationLocatior.h"
#include "PlayerJumpingPawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "AI/Navigation/NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "Rifle.h"
#include "SkeletonIK.h"
#include "Engine.h"
#include "PlayerPawn.h"

// UE4 не дает передавать двухмерные массивы ////////////////////////////////////////////

//  оррел€ци€ наборов двух векторов
// a[3][Num], b[3][Num] - векторные массивы данных, r1, r2, r3 - коэффициенты коррел€ции
void CorrelateArraysExt(float a[][CORR_POINTS_NUM], float b[][CORR_POINTS_NUM], int32 Num, float &r1, float &r2, float &r3);

// a - velocity set[3][Num], index 0..3 - array indexes to compare, r - return value
void CorrelateFloatsExt(float* a, float* b, int32 Num, float &r);

/////////////////////////////////////////////////////////////////////////////////////////

// Sets default values
APlayerPawn::APlayerPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	SetupComponents();

	NavigationTeleportDistance = 600.0f;
	MovementSpeed = 600.0f;
	bUseDirectTeleportControl = false;
}

void APlayerPawn::SetupComponents()
{
	PawnRotationArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("PawnRotationArrow"));
	PawnRotationArrow->SetupAttachment(GetRootComponent());
	PawnRotationArrow->SetRelativeLocation(FVector::ZeroVector);
	PawnRotationArrow->SetRelativeRotation(FRotator::ZeroRotator);

	/******************* __SteamVR_Support__ *******************/

	CameraRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CameraRoot"));
	CameraRoot->SetupAttachment(GetRootComponent());

	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	VRCamera->SetupAttachment(CameraRoot);
	VRCamera->RelativeLocation = FVector::ZeroVector;
	VRCamera->bUsePawnControlRotation = false;

	ControllersRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ControllersRoot"));
	ControllersRoot->SetupAttachment(GetRootComponent());

	MControllerLeft = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MControllerLeft"));
	MControllerLeft->SetupAttachment(ControllersRoot);
	MControllerLeft->Hand = EControllerHand::Left;

	MControllerRight = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MControllerRight"));
	MControllerRight->SetupAttachment(ControllersRoot);
	MControllerRight->Hand = EControllerHand::Right;

	SteamVRChaperone = CreateDefaultSubobject<USteamVRChaperoneComponent>(TEXT("SteamVRChaperone"));

	/***********************************************************/

	FTransform trR = GetBasicHandTransform(EHandSide::Right);
	FTransform trL = GetBasicHandTransform(EHandSide::Left);

	RightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightHandMesh"));
	RightHandMesh->SetupAttachment(MControllerRight);
	RightHandMesh->SetRelativeLocationAndRotation(trR.GetTranslation(), trR.GetRotation().Rotator()); //(FVector(-13.38823f, 4.193795f, 6.885473f), FRotator(300.673248f, 351.372528f, 7.577085f));
	RightHandMesh->ComponentTags.Add(HAND_RIGHT);
	RightHandMesh->SetCollisionObjectType(EEC_ControllerChannel);
	RightHandMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RightHandMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	RightHandMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Block);
	RightHandMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	RightHandMesh->bGenerateOverlapEvents = true;
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> skmRightHand(TEXT("SkeletalMesh'/Game/Pirates/Skeletals/HandRight_SKM.HandRight_SKM'"));
	if (skmRightHand.Object != NULL) RightHandMesh->SetSkeletalMesh(skmRightHand.Object);

	LeftHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftHandMesh"));
	LeftHandMesh->SetupAttachment(MControllerLeft);
	LeftHandMesh->SetRelativeLocationAndRotation(trL.GetTranslation(), trL.GetRotation().Rotator()); //(FVector(-13.38823f, -4.193795f, 6.885473f), FRotator(300.709534f, 9.43001f, 352.015839f));
	LeftHandMesh->ComponentTags.Add(HAND_LEFT);
	LeftHandMesh->SetCollisionObjectType(EEC_ControllerChannel);
	LeftHandMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	LeftHandMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	LeftHandMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Block);
	LeftHandMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	LeftHandMesh->bGenerateOverlapEvents = true;
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> skmLeftHand(TEXT("SkeletalMesh'/Game/Pirates/Skeletals/HandLeft_SKM.HandLeft_SKM'"));
	if (skmLeftHand.Object != NULL) LeftHandMesh->SetSkeletalMesh(skmLeftHand.Object);

	NavigationPointerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NavigationPointerMesh"));
	NavigationPointerMesh->SetupAttachment(GetRootComponent());
	NavigationPointerMesh->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> smNavPointer(TEXT("StaticMesh'/Game/Pirates/Meshes/navigation_line.navigation_line'"));
	if (smNavPointer.Object != NULL) NavigationPointerMesh->SetStaticMesh(smNavPointer.Object);
	NavigationPointerMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	NavigationPointerMesh->SetVisibility(false);

	SkeletonIK = CreateDefaultSubobject<USkeletonIK>(TEXT("SkeletonIK"));
}

// Called when the game starts or when spawned
void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	
	SetHandState(EHandSide::Right, EHandState::Idle);
	SetHandState(EHandSide::Left, EHandState::Idle);

	stTraceParams = FCollisionQueryParams(FName(TAG_NAVIGATION_LINE), true, this);
	stTraceParams.bTraceComplex = true;
	stTraceParams.bTraceAsyncScene = true;
	stTraceParams.bReturnPhysicalMaterial = false;
	stTraceParams.bTraceComplex = false;
	//stTraceParams.OwnerTag = TEXT("IGNORE");

	ActiveNavSystem = UNavigationSystem::GetNavigationSystem(GetWorld());
	mti_NavigationLine = NavigationPointerMesh->CreateDynamicMaterialInstance(0);
	IsNavigationPossible = false;
	IsAvatarRunningNow = false;
	SlideInterpTime = 0.0f;

	SetActorTickEnabled(false);
}

// Called every frame
void APlayerPawn::Tick( float DeltaTime )
{
	Super::Tick(DeltaTime);

	if (bNavigationFinderActivated) {
		/////////////////////////////////////////////////////////////////////////////////////
		FHitResult stHit(ForceInit);
		FVector vStart = NavigationPointerMesh->GetComponentLocation();
		FVector vEnd = vStart + NavigationPointerMesh->GetComponentRotation().Vector() * 50000.0f;
		vStart += NavigationPointerMesh->GetComponentRotation().Vector() * 4.0f;

		FCollisionResponseParams d;
		d.CollisionResponse.SetAllChannels(ECollisionResponse::ECR_Ignore);
		d.CollisionResponse.SetResponse(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
		d.CollisionResponse.SetResponse(EEC_FloorChannel, ECollisionResponse::ECR_Block);
		d.CollisionResponse.SetResponse(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
		d.CollisionResponse.SetResponse(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
		GetWorld()->LineTraceSingleByChannel(stHit, vStart, vEnd, ECollisionChannel::ECC_Visibility, stTraceParams, d);

		//DrawDebugLine(GetWorld(), vStart, vEnd, FColor::White, false, -1, 0, 1.0f);
		//DrawDebugLine(GetWorld(), vStart, vStart * 2 - vEnd, FColor::Red, false, -1, 0, 1.0f);

		if (stHit.bBlockingHit && stHit.IsValidBlockingHit()) {
			if (stHit.GetComponent()->GetCollisionObjectType() == EEC_FloorChannel) {
				FVector HitPoint = stHit.Location;
				FNavLocation NavPoint;
				if (!IsValid(ActiveNavSystem)) ActiveNavSystem = UNavigationSystem::GetNavigationSystem(GetWorld());

				if (stHit.Distance > NavigationTeleportDistance) {
					IsNavigationPossible = false;
				}
				else if (!ActiveNavSystem->ProjectPointToNavigation(HitPoint, NavPoint)) {
					IsNavigationPossible = false;
				}
				else {
					IsNavigationPossible = true;
				}

				FVector NewActorLocation, NewPlayerLocation, NavLocatiorOffset;
				bool CustomNavArea = stHit.GetComponent()->ComponentHasTag(TAG_CUSTOM_NAV_AREA);
				if (CustomNavArea) {
					NewActorLocation = stHit.GetActor()->GetActorLocation();
					NewPlayerLocation = NewActorLocation + VRCamera->GetRelativeTransform().GetTranslation();
					NewPlayerLocation.Z = NewActorLocation.Z;
					NavLocatiorOffset = FVector(0.0f, 0.0f, 3.0f);
				}
				else {
					NewPlayerLocation = stHit.Location;
					NewActorLocation = NewPlayerLocation - VRCamera->GetRelativeTransform().GetTranslation();
					NewActorLocation.Z = stHit.Location.Z;
					NavLocatiorOffset = FVector(0.0f, 0.0f, 5.0f);
				}

				if (IsValid(NavigationLocatorMesh)) {
					NavigationLocatorMesh->SetActorLocation(NewPlayerLocation + NavLocatiorOffset); //stHit.Location + FVector(0.0f, 0.0f, 4.0f));
					if (!NavigationLocatorMesh->IsVisible()) NavigationLocatorMesh->SetVisibility(true);
					if (IsNavigationPossible) NavigationLocatorMesh->SetNavigationAreaType(CustomNavArea);
				}

				NavigationDestinationHeadPoint = NewPlayerLocation; //stHit.Location;
				//NavigationDestinationActorPoint = NavigationDestinationHeadPoint - VRCamera->GetRelativeTransform().GetTranslation();
				//NavigationDestinationActorPoint.Z = NavigationDestinationHeadPoint.Z;
				NavigationDestinationActorPoint = NewActorLocation;
				NavigationDestinationHeadPoint.Z += 85.0f;
			}
			else {
				IsNavigationPossible = false;
				if (IsValid(NavigationLocatorMesh) && !NavigationLocatorMesh->IsVisible()) {
					NavigationLocatorMesh->SetVisibility(false);
				}
			}
			NavigationPointerMesh->SetRelativeScale3D(FVector(stHit.Distance * 0.01f, 1.0f, 1.0f));
		}

		if (IsValid(mti_NavigationLine)) {
			if (IsNavigationPossible)
				mti_NavigationLine->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0.2755f, 0.945f, 0.403f, 1.0f));
			else
				mti_NavigationLine->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0.945f, 0.2755f, 0.403f, 1.0f));
		}
		if (IsValid(NavigationLocatorMesh)) {
			NavigationLocatorMesh->SetState(IsNavigationPossible);
		}
	}

	if (bSlideActorToNewLocation) {
		bNavigationFinderActivated = false;
		if (SlideInterpTime == 0.0f) {
			ActorSlideStartLocation = GetActorLocation();
		}
		float const SlideSpeed = 8.0f;
		FVector Dist = (NavigationDestinationActorPoint - ActorSlideStartLocation);

		SlideInterpTime += DeltaTime;
		//Dist.Normalize();

		FVector v = FMath::VInterpTo(ActorSlideStartLocation, NavigationDestinationActorPoint, SlideInterpTime, SlideSpeed);
			//ActorSlideStartLocation + Dist * SlideInterpTime * SlideSpeed;
		

		if ((v - NavigationDestinationActorPoint).Size() < 5.0f || SlideInterpTime >= (1.0f / SlideSpeed)) {
			v = NavigationDestinationActorPoint;
			bSlideActorToNewLocation = false;
			SlideInterpTime = 0.0f;
		}
		else if (((v - NavigationDestinationActorPoint).Size() < 100.0f) && IsValid(NavigationAvatar) && !NavigationAvatar->bHidden) {
			NavigationAvatar->DeactivatePawn();
		}
		SetActorLocation(v);
	}

	if (bDirectAvatarMovingAround) {
		if (IsValid(NavigationAvatar)) {
			FVector loc = FindDirectAvatarStartRunningPoint();
			FVector dist = loc - NavigationAvatar->GetActorLocation();
			dist.Z = 0.0f;

			if (dist.Size() > 30.0f) {
				NavigationAvatar->StartRunning(loc);
			}
		}
		else {
			bDirectAvatarMovingAround = false;
		}
	}

	if (!(bNavigationFinderActivated || bSlideActorToNewLocation || bDirectAvatarMovingAround)) SetActorTickEnabled(false);
}

// Called to bind functionality to input
void APlayerPawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	check(InputComponent);

	InputComponent->BindAxis("MoveForwardLeft", this, &APlayerPawn::MoveForward);
}

void APlayerPawn::MoveForward(float Value)
{
	/*
	if (bUseDirectTeleportControl && IsValid(NavigationAvatar) && !NavigationAvatar->bHidden && !NavigationAvatar->GetMovingState() && (Value != 0.0f)) {
		FVector loc1 = NavigationAvatar->GetActorLocation();
		NavigationAvatar->MoveForward(Value);
		FVector loc2 = NavigationAvatar->GetActorLocation();

		//this->AddActorWorldOffset(loc2 - loc1);



		bDirectAvatarMovingAround = false;
	}
	*/

	if (bUseDirectTeleportControl && (Controller != NULL) && Value > 0.0f && bRightTrackpadPressed)
	{
		const FRotator Rotation = VRCamera->GetComponentRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddActorWorldOffset(GetWorld()->DeltaTimeSeconds * MovementSpeed * Value * YawRotation.Vector());
		//AddMovementInput(Direction, Value);

		if (!NavigationAvatar->bHidden) {
			FVector finloc = FindDirectAvatarStartRunningPoint(true);
			FVector diff = NavigationAvatar->GetActorLocation() - finloc;
			diff.Z = 0.0f;
			if (diff.Size() > 130.0f) 
				NavigationAvatar->GetCharacterMovement()->MaxWalkSpeed = MovementSpeed + 170.0f;
			else
				NavigationAvatar->GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;

			NavigationAvatar->MoveByMasterPawn(finloc);
		}
		bDirectAvatarMovingAround = false;
	}
}

void APlayerPawn::UpdateHandState_Implementation(EHandSide Hand, EHandState State)
{
	/*
	if (Hand == EHandSide::Right) {
		RightHandState = State;
	}
	else if (Hand == EHandSide::Right) {
		LeftHandState = State;
	}
	*/
}

void APlayerPawn::SetHandState(EHandSide Hand, EHandState State)
{
	if (Hand == EHandSide::Right) {
		RightHandState = State;
	}
	else if (Hand == EHandSide::Left) {
		LeftHandState = State;
	}
	UpdateHandState(Hand, State);
}

FTransform APlayerPawn::GetBasicHandTransform(EHandSide Hand)
{
	FTransform ret;
	if (Hand == EHandSide::Right) {
		ret.SetTranslation(FVector(-13.38823f, 4.193795f, 6.885473f));
		ret.SetRotation(FRotator(300.673248f, 351.372528f, 7.577085f).Quaternion());
		ret.SetScale3D(FVector(1.0f, 1.0f, 1.0f));
	}
	else {
		ret.SetTranslation(FVector(-13.38823f, -4.193795f, 6.885473f));
		ret.SetRotation(FRotator(300.709534f, 9.43001f, 352.015839f).Quaternion());
		ret.SetScale3D(FVector(1.0f, 1.0f, 1.0f));
	}

	return ret;
}

// -----------------------------------------------------
// trackpad click & trackpad cancel release
// -----------------------------------------------------
void APlayerPawn::ActivateNavigationFinder(bool State, EHandSide Side)
{
	if (State) {
		if (Side == EHandSide::Right)
			bRightTrackpadPressed = State;
		else
			bLeftTrackpadPressed = State;
	}

	// use classical teleport
	if (!bUseDirectTeleportControl) {
		if (State) {
			if (IsAvatarRunningNow) return;

			if (IsValid(NavigationLocatorMesh)) NavigationLocatorMesh->SetVisibility(true);
			if (IsValid(CurrentRifle)) {
				NavigationPointerMesh->AttachToComponent(CurrentRifle->GetStaticMeshComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SOCKET_NAV_POINTER);
			}
			else if (Side == EHandSide::Right) {
				NavigationPointerMesh->AttachToComponent(RightHandMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SOCKET_NAV_POINTER);
			}
			else if (Side == EHandSide::Left) {
				NavigationPointerMesh->AttachToComponent(LeftHandMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SOCKET_NAV_POINTER);
			}
			NavigationPointerMesh->SetVisibility(true);

			bNavigationFinderActivated = true;
			SetActorTickEnabled(true);

			if (IsValid(CurrentRifle) && IsValid(NavigationAvatar)) {
				CurrentRifle->StartPosessRifle(NavigationAvatar->GetRiflePointer());
			}
		}
		else {
			if (IsValid(NavigationLocatorMesh)) NavigationLocatorMesh->SetVisibility(false);
			NavigationPointerMesh->SetVisibility(false);
			if (IsValid(CurrentRifle) && !IsAvatarRunningNow) CurrentRifle->StopPosessRifle();

			//SetActorTickEnabled(false);
			bNavigationFinderActivated = false;
		}
	}
	// use IK & trackpads controlled third person movement from single point
	else if (bUseDirectTeleportControl && Side == EHandSide::Right) {
		if (State) {
			if (IsAvatarRunningNow) return;

			if (IsValid(NavigationAvatar)) {
				FVector StartPoint = FindDirectAvatarStartRunningPoint();

				if (StartPoint != FVector::ZeroVector) {
					NavigationAvatar->ActivatePawn(this, StartPoint, StartPoint);
					IsAvatarRunningNow = true;
					bDirectAvatarMovingAround = true;
					SetActorTickEnabled(true);
				}
			}
		}
	}
}

// -----------------------------------------------------
// trackpad ok release
// -----------------------------------------------------
void APlayerPawn::NavigateByFinder(EHandSide Side)
{
	if (Side == EHandSide::Right)
		bRightTrackpadPressed = false;
	else
		bLeftTrackpadPressed = false;

	// use classical teleport
	if (!bUseDirectTeleportControl) {
		if (IsAvatarRunningNow) return;
		bool ShowAvatar = false;

		if (IsNavigationPossible) {
			if (IsValid(NavigationAvatar) && (NavigationDestinationActorPoint - GetActorLocation()).Size() > 100.0f) {
				FVector StartPoint = FindAvatarStartRunningPoint();

				if (StartPoint != FVector::ZeroVector) {
					NavigationAvatar->ActivatePawn(this, NavigationDestinationHeadPoint, StartPoint);
					IsAvatarRunningNow = true;
					ShowAvatar = true;

					if (IsValid(DebugIKAvatar)) DebugIKAvatar->ActivatePawn(this, NavigationDestinationHeadPoint + FVector(0.0f, 150.0f, 0.0f), DebugIKAvatar->GetActorLocation());
				}
			}

			if (!ShowAvatar) {
				//SetActorLocation(NavigationDestinationActorPoint);
				bSlideActorToNewLocation = true;
				SetActorTickEnabled(true);
			}
		}

		ActivateNavigationFinder(false);
	}
	// use IK & trackpads controlled third person movement from single point
	else if (bUseDirectTeleportControl && Side == EHandSide::Right) {
		if (IsValid(CurrentRifle)) {
			CurrentRifle->StopPosessRifle();
		}

		IsAvatarRunningNow = false;
		if (IsValid(NavigationLocatorMesh)) NavigationLocatorMesh->SetVisibility(false);
		if (IsValid(CurrentRifle) && !IsAvatarRunningNow) CurrentRifle->StopPosessRifle();
		//bSlideActorToNewLocation = true;
		bDirectAvatarMovingAround = false;
		NavigationAvatar->DeactivatePawn();
		SetActorTickEnabled(false);
	}
}

void APlayerPawn::SetJumpTeleportAvatar(APlayerJumpingPawn* TeleportAvatar)
{
	NavigationAvatar = TeleportAvatar;

	if (IsValid(NavigationAvatar)) {
		NavigationAvatar->OnRunningComplete.AddDynamic(this, &APlayerPawn::OnAvatarRunningComplete);
	}
}

APawn* APlayerPawn::GetActivePawn()
{
	if (IsAvatarRunningNow && IsValid(NavigationAvatar)) {
		return NavigationAvatar;
	}
	else {
		return this;
	}
}

FVector APlayerPawn::GetActivePawnHead()
{
	if (IsAvatarRunningNow && IsValid(NavigationAvatar)) {
		return NavigationAvatar->GetMesh()->GetSocketLocation(TEXT("head"));
	}
	else {
		return VRCamera->GetComponentLocation();
	}
}

void APlayerPawn::OnAvatarRunningComplete(FVector NewLocation)
{
	if (bUseDirectTeleportControl) return;
	if (bSlideActorToNewLocation) return;

	if (IsValid(CurrentRifle)) {
		CurrentRifle->StopPosessRifle();
	}

	IsAvatarRunningNow = false;
	ActivateNavigationFinder(false);

	bSlideActorToNewLocation = true;
	SetActorTickEnabled(true);

	SkeletonIK->ResetFeetLocations();
}

// Return Transform of RIfle relative to head transform (bUseHeadRotation == true)
// or body (i.e pelvis and ribcage) transform (bUseHeadRotation == false).
FTransform APlayerPawn::GetRifleRelativeTransform(FTransform PelvisOffset, bool bUseHeadRotation)
{
	FTransform cam, rifle, rel, par, mesh;
	FVector pos, dec;
	FRotator rot;

	// [bUseHeadRotation == false]
	// The idea is to get pelvis transform of PlayerPawn and use it as a parent transform for rifle
	// The function need to be called after SkeletonTransformData::PelvisOffset and SkeletonTransformData::HeadOffset setup.

	cam = VRCamera->GetComponentTransform();
	cam.SetTranslation(cam.GetTranslation() + cam.GetRotation().GetForwardVector() * -HEAD_HALF_WIDTH);
	rifle = CurrentRifle->GetActorTransform();

	if (bUseHeadRotation) {
		pos = VRCamera->GetComponentLocation();						// pos = world space (0,0,0) transform for camera
		pos.Z = GetRootComponent()->GetComponentLocation().Z;
		rot = FRotator(0.0f, cam.GetRotation().Rotator().Yaw, 0.0f);
		dec = rot.Vector(); dec.Normalize();	// поправка, чтобы не влезать в тело меша
		pos -= dec * 15.0f; pos.Z -= 5.0f;		// поправка, чтобы не влезать в тело меша
		par.SetTranslation(pos); par.SetRotation(rot.Quaternion()); par.SetScale3D(FVector(1.0f, 1.0f, 1.0f));
		mesh.SetTranslation(FVector::ZeroVector); mesh.SetRotation(FRotator(0.0f, -90.0f, 0.0f).Quaternion()); mesh.SetScale3D(FVector(1.0f, 1.0f, 1.0f));

		rel = UKismetMathLibrary::ConvertTransformToRelative(par, rifle);
		rel = UKismetMathLibrary::ConvertTransformToRelative(mesh, rel);
	}
	else {
		FTransform cambase = FTransform(cam.GetRotation(), FVector(cam.GetTranslation().X, cam.GetTranslation().Y, 0.0f));
		FTransform fp_pelvis = PelvisOffset * cambase; // pelvis relative to cam in world space

		//FTransform fp_pelvis = SkeletonTransformData.PelvisOffset;
		//fp_pelvis.SetLocation(FVector(cam.GetLocation().X, cam.GetLocation().Y, 0.0f));
		//FRotator y;
		//y = fp_pelvis.GetRotation().Rotator();
		//y.Yaw += 180.0f;
		//fp_pelvis.SetRotation(y.Quaternion());

		rel = UKismetMathLibrary::ConvertTransformToRelative(cam, rifle); // rifle relative to cam
		rel.SetLocation(rel.GetLocation() + FVector(0.0f, 0.0f, 10.0f));
		//rel *= SkeletonTransformData.HeadOffset;
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::SanitizeFloat(fp_pelvis.GetTranslation().Z) + " / " + FString::SanitizeFloat(rifle.GetTranslation().Z));
	}

	return rel;
}

// If current Camera location is outside of a Navigation Mesh
// Find the closest point at the line Source-Destination to start moving
FVector APlayerPawn::FindAvatarStartRunningPoint()
{
	FVector st, fn, curr, diff;
	FVector ret = FVector::ZeroVector;
	FNavLocation resultpoint;
	const int SegmentsCount = 10;

	st = VRCamera->GetComponentLocation();
	st.Z = GetActorLocation().Z;
	fn = NavigationDestinationHeadPoint;
	diff = fn - st;

	if (ActiveNavSystem->ProjectPointToNavigation(st, resultpoint)) {
		ret = st;
	}
	else {
		if (!IsValid(ActiveNavSystem)) ActiveNavSystem = UNavigationSystem::GetNavigationSystem(GetWorld());

		//split to 10 segments
		for (int i = 1; i < SegmentsCount; i++) {
			curr = st + diff * (float)i / (float)SegmentsCount;

			if (ActiveNavSystem->ProjectPointToNavigation(curr, resultpoint)) {
				ret = curr;
				break;
			}
		}
	}

	return ret;
}

// If current Camera location is outside of a Navigation Mesh
// Find the closest point at the line Source-Destination to start moving
FVector APlayerPawn::FindDirectAvatarStartRunningPoint(bool RunningTarget)
{
	FVector st, fn;
	FVector ret = FVector::ZeroVector;
	float mul = RunningTarget ? 200.0f : 100.0f;
	FNavLocation resultpoint;

	st = VRCamera->GetComponentLocation();
	st.Z = GetActorLocation().Z;
	fn = FRotator(0.0f, VRCamera->GetComponentRotation().Yaw, 0.0f).Vector() * mul + st;

	if (!IsValid(ActiveNavSystem)) ActiveNavSystem = UNavigationSystem::GetNavigationSystem(GetWorld());
	if (ActiveNavSystem->ProjectPointToNavigation(fn, resultpoint)) {
		ret = fn;
	}

	return ret;
}

void APlayerPawn::SetActive(bool IsActive)
{
	SetActorHiddenInGame(!IsActive);
	if (IsValid(CurrentRifle)) {
		CurrentRifle->SetActorHiddenInGame(!IsActive);
	}
}

float APlayerPawn::DeltaAngle(FVector Angle1, FVector Angle2)
{
	/*
	float fRet = FMath::Abs(Angle1 - Angle2);

	if (fRet > 180.0f) {
		if (Angle1 < Angle2) Swap(Angle1, Angle2);

		Angle1 -= 360.0f;
		fRet = Angle2 - Angle1;
	}
	*/
	float fRet = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Angle1, Angle2)));
	
	return fRet;
}

inline float APlayerPawn::GetAngleToInterp(float Current, float Target)
{
	float fRet = Current;

	if (FMath::Abs(Current - Target) > 180.0f) {
		if (Current < Target) fRet += 360.0f; else fRet -= 360.0f;
	}

	return fRet;
}