// Fill out your copyright notice in the Description page of Project Settings.

#include "PotSS.h"
#include "Engine.h"
#include "PlayerFirstPersonController.h"
#include "MotionControllerComponent.h"
#include "PlayerPawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "Rifle.h"

ARifle::ARifle()
{
	PrimaryActorTick.bCanEverTick = true;

	SetupComponents();
	SetupDefaults();

	SetActorTickEnabled(false);
}

void ARifle::SetupComponents()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> smRifle(TEXT("StaticMesh'/Game/Pirates/Meshes/rifle.rifle'"));
	if (smRifle.Object != NULL) GetStaticMeshComponent()->SetStaticMesh(smRifle.Object);

	static ConstructorHelpers::FObjectFinder<USoundWave> swFire(TEXT("SoundWave'/Game/Pirates/Audio/Alliance-AssaultRifle_05-Single_Shot-04_UE4.Alliance-AssaultRifle_05-Single_Shot-04_UE4'"));
	if (swFire.Object != NULL) FireSound = swFire.Object;	

	GetStaticMeshComponent()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	GetStaticMeshComponent()->SetCollisionResponseToChannel(EEC_ProjectileChannel, ECollisionResponse::ECR_Ignore);
	GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
	GetStaticMeshComponent()->SetCanEverAffectNavigation(false);

	MainHandCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("MainHandCollider"));
	MainHandCollider->SetupAttachment(GetStaticMeshComponent(), SOCKET_COLLIDER_MAIN);
	MainHandCollider->SetRelativeLocation(FVector::ZeroVector);
	MainHandCollider->SetRelativeRotation(FRotator::ZeroRotator);
	MainHandCollider->SetBoxExtent(FVector(8.0f, 4.0f, 4.0f));
	MainHandCollider->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	MainHandCollider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MainHandCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	MainHandCollider->SetCollisionResponseToChannel(EEC_ControllerChannel, ECollisionResponse::ECR_Overlap);
	MainHandCollider->SetCollisionResponseToChannel(EEC_ProjectileChannel, ECollisionResponse::ECR_Ignore);
	MainHandCollider->OnComponentBeginOverlap.AddDynamic(this, &ARifle::OnMainCollider_BeginOverlap);
	MainHandCollider->bGenerateOverlapEvents = true;
	MainHandCollider->bHiddenInGame = true;
	MainHandCollider->SetCanEverAffectNavigation(false);

	SecondHandCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("SecondHandCollider"));
	SecondHandCollider->SetupAttachment(GetStaticMeshComponent(), SOCKET_COLLIDER_SECOND);
	SecondHandCollider->SetRelativeLocation(FVector::ZeroVector);
	SecondHandCollider->SetRelativeRotation(FRotator::ZeroRotator);
	SecondHandCollider->SetBoxExtent(FVector(8.0f, 4.0f, 6.0f));
	SecondHandCollider->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	SecondHandCollider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SecondHandCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SecondHandCollider->SetCollisionResponseToChannel(EEC_ControllerChannel, ECollisionResponse::ECR_Overlap);
	SecondHandCollider->SetCollisionResponseToChannel(EEC_ProjectileChannel, ECollisionResponse::ECR_Ignore);
	SecondHandCollider->OnComponentBeginOverlap.AddDynamic(this, &ARifle::OnSecondCollider_BeginOverlap);
	SecondHandCollider->OnComponentEndOverlap.AddDynamic(this, &ARifle::OnSecondCollider_EndOverlap);
	SecondHandCollider->bGenerateOverlapEvents = true;
	SecondHandCollider->bHiddenInGame = true;
	SecondHandCollider->SetCanEverAffectNavigation(false);
}

void ARifle::SetupDefaults()
{
	ShootingInterval_MainBarrel = 0.1f;
	ShootingInterval_SecondBarrel = 0.35f;
	ShotHeating_MainBarrel = 0.02f;
	ShotHeating_SecondBarrel = 0.06f;
	CooldownTime = 4.0f;
	IsUsable = true;

	UseSecondBarrel = false;
	Temperature = 0.0f;
	CurrentPlayer = nullptr;
	MainHandSlotState = EHandSide::None;
	SecondHandSlotState = EHandSide::None;
	bCorrectRifleTransform = false;
	bCorrectSecondHandTransform = false;
	bReturnSecondHandTransform = false;
	PuppetRifle = nullptr;
}

void ARifle::BeginPlay()
{
	Super::BeginPlay();

	mti_Body = GetStaticMeshComponent()->CreateDynamicMaterialInstance(0);
	mti_Indicator = GetStaticMeshComponent()->CreateDynamicMaterialInstance(2);
	MainHandCollider->bHiddenInGame = true;
	SecondHandCollider->bHiddenInGame = true;
	NeedFullCooldown = false;

	SetActorTickEnabled(false);
}

void ARifle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FTransform tr;
	FTransform dest;

	// align rifle to second hand controller
	if (SecondHandSlotState != EHandSide::None) {
		float roll = GetStaticMeshComponent()->GetRelativeTransform().GetRotation().Rotator().Roll;

		FRotator lt = UKismetMathLibrary::FindLookAtRotation(GetStaticMeshComponent()->GetSocketLocation(SOCKET_COLLIDER_MAIN), TargetSecondHandController->GetComponentLocation());
		GetStaticMeshComponent()->SetWorldRotation(lt);

		FTransform tr2 = GetStaticMeshComponent()->GetRelativeTransform();
		FRotator rt2 = tr2.GetRotation().Rotator();
		rt2.Roll = (roll / 2.0f);
		tr2.SetRotation(rt2.Quaternion());
		GetStaticMeshComponent()->SetRelativeTransform(tr2);

		/*
		if (IsValid(MainHandMesh)) {
			FRotator p0, p1;
			p0 = MainHandMesh->GetComponentRotation();
			if (MainHandSlotState == EHandSide::Right)
				p1 = GetStaticMeshComponent()->GetSocketRotation(SOCKET_MAIN_HAND_R);
			else
				p1 = GetStaticMeshComponent()->GetSocketRotation(SOCKET_MAIN_HAND_L);

			p0.Pitch = p1.Pitch;
			MainHandMesh->SetWorldRotation(p0);
		}
		*/
		if ((SecondHandCollider->GetComponentLocation() - TargetSecondHandController->GetComponentLocation()).Size() > 16.0f) {
			DetachFromSecondHand(SecondHandMesh);
		}
	}

	// move rifle to main hand
	if (bCorrectRifleTransform) {
		tr = GetStaticMeshComponent()->GetRelativeTransform();
		dest = FTransform(FRotator(-15.0f, 0.0f, 0.0f).Quaternion(), FVector::ZeroVector, FVector(1.0f, 1.0f, 1.0f));

		tr = UKismetMathLibrary::TInterpTo(tr, dest, DeltaTime, 6.0f);
		if (tr.GetTranslation().Size() < 2.0f && tr.GetRotation().Vector().Size() < 4.0f) {
			tr = dest;
			bCorrectRifleTransform = false;
		}
		GetStaticMeshComponent()->SetRelativeTransform(tr);
	}

	// move second hand to rifle
	if (bCorrectSecondHandTransform) {
		/*
		tr = SecondHandMesh->GetRelativeTransform();
		dest = FTransform(FRotator::ZeroRotator.Quaternion(), FVector::ZeroVector, FVector(1.0f, 1.0f, 1.0f));

		tr = UKismetMathLibrary::TInterpTo(tr, dest, DeltaTime, 6.0f);
		if (tr.GetTranslation().Size() < 2.0f && tr.GetRotation().Vector().Size() < 4.0f) {
			tr = dest;
			bCorrectSecondHandTransform = false;
		}
		SecondHandMesh->SetRelativeTransform(tr);
		*/
	}

	// move second hand to controller location
	if (bReturnSecondHandTransform) {
		/*
		tr = SecondHandMesh->GetRelativeTransform();
		dest = APlayerPawn::GetBasicHandTransform(SecondHandSide);

		tr = UKismetMathLibrary::TInterpTo(tr, dest, DeltaTime, 6.0f);
		if (tr.GetTranslation().Size() < 2.0f && tr.GetRotation().Vector().Size() < 4.0f) {
			tr = dest;
			bReturnSecondHandTransform = false;
		}
		SecondHandMesh->SetRelativeTransform(tr);
		*/
		SecondHandMesh->SetRelativeTransform(APlayerPawn::GetBasicHandTransform(SecondHandSide));
		bReturnSecondHandTransform = false;
	}

	if (!(bCorrectRifleTransform || bCorrectSecondHandTransform || bReturnSecondHandTransform) && SecondHandSlotState == EHandSide::None) {
		SetActorTickEnabled(false);
	}
}

void ARifle::OnMainCollider_BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsUsable || MainHandSlotState != EHandSide::None) return;

	if (OtherActor->IsA(APawn::StaticClass()) && OtherComp->IsA(USkeletalMeshComponent::StaticClass())) {
		USkeletalMeshComponent* ctrl = Cast<USkeletalMeshComponent>(OtherComp);
		PickupRiffle(ctrl);
	}
}

void ARifle::OnSecondCollider_BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsUsable || !IsValid(CurrentPlayer) || MainHandSlotState == EHandSide::None || SecondHandSlotState != EHandSide::None) return;

	if (OtherActor->IsA(APawn::StaticClass()) && OtherActor->GetUniqueID() == CurrentPlayer->GetUniqueID() && OtherComp->IsA(USkeletalMeshComponent::StaticClass())) {
		USkeletalMeshComponent* ctrl = Cast<USkeletalMeshComponent>(OtherComp);
		AttachToSecondHand(ctrl);
	}
}

void ARifle::OnSecondCollider_EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	/*
	if (!IsUsable || MainHandSlotState == EHandSide::None || !IsValid(CurrentPlayer)) return;

	if (OtherActor->IsA(APawn::StaticClass()) && OtherActor->GetUniqueID() == CurrentPlayer->GetUniqueID() && OtherComp->IsA(USkeletalMeshComponent::StaticClass())) {
		USkeletalMeshComponent* ctrl = Cast<USkeletalMeshComponent>(OtherComp);
		DetachFromSecondHand(ctrl);
	}
	*/
}

void ARifle::PickupRiffle(USkeletalMeshComponent* HandMesh)
{
	if (!IsUsable) return;
	CurrentPlayer = Cast<APlayerPawn>(HandMesh->GetOwner());

	if (IsValid(CurrentPlayer)) {
		EHandSide hand;
		if (HandMesh->ComponentHasTag(HAND_RIGHT)) {
			hand = EHandSide::Right;
			if (CurrentPlayer->RightHandState != EHandState::Idle) return;
		}
		else if (HandMesh->ComponentHasTag(HAND_LEFT)) {
			hand = EHandSide::Left;
			if (CurrentPlayer->LeftHandState != EHandState::Idle) return;
		}
		else {
			CurrentPlayer = nullptr;
			return;
		}

		CurrentPlayer->SetHandState(hand, EHandState::RifleMain);
		CurrentPlayer->CurrentRifle = this;

		MainHandMesh = HandMesh;
		MainHandSlotState = hand;
		this->AttachToComponent(HandMesh, FAttachmentTransformRules::KeepWorldTransform, SOCKET_RIFLE_MAIN);
		bCorrectRifleTransform = true;
		SetActorTickEnabled(true);
	}
}

void ARifle::AttachToSecondHand(USkeletalMeshComponent* HandMesh)
{
	if (!IsUsable) return;
	if (IsValid(CurrentPlayer)) {
		EHandSide hand;
		FName socket;
		if (HandMesh->ComponentHasTag(HAND_RIGHT))
			hand = EHandSide::Right;
		else if (HandMesh->ComponentHasTag(HAND_LEFT))
			hand = EHandSide::Left;
		else {
			return;
		}
		// trying to take with the same hand - just to make sure everything is right
		if (MainHandSlotState == hand) return;
		if (hand == EHandSide::Right)
			socket = SOCKET_SECOND_HAND_R;
		else
			socket = SOCKET_SECOND_HAND_L;

		SecondHandSlotState = hand;
		SecondHandSide = hand;
		SecondHandMesh = HandMesh;
		TargetSecondHandController = (hand == EHandSide::Right) ? CurrentPlayer->MControllerRight : CurrentPlayer->MControllerLeft;
		HandMesh->AttachToComponent(GetStaticMeshComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale, socket);// ::KeepWorldTransform, socket);
		//HandMesh->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
		CurrentPlayer->SetHandState(hand, EHandState::RifleSupport);

		bCorrectSecondHandTransform = false;
		//bCorrectSecondHandTransform = true;
		SetActorTickEnabled(true);
	}
}

void ARifle::DetachFromSecondHand(USkeletalMeshComponent* HandMesh)
{
	if (!IsUsable) return;
	if (IsValid(CurrentPlayer)) {
		EHandSide hand;
		if (HandMesh->ComponentHasTag(HAND_RIGHT))
			hand = EHandSide::Right;
		else if (HandMesh->ComponentHasTag(HAND_LEFT))
			hand = EHandSide::Left;
		else {
			return;
		}
		// trying to take with the same hand - just to make sure everything is right
		if (SecondHandSlotState == hand) {
			SecondHandMesh = HandMesh;
			TargetSecondHandController = (hand == EHandSide::Right) ? CurrentPlayer->MControllerRight : CurrentPlayer->MControllerLeft;
			HandMesh->AttachToComponent(TargetSecondHandController, FAttachmentTransformRules::KeepWorldTransform);
			GetStaticMeshComponent()->SetRelativeRotation(FRotator::ZeroRotator);

			CurrentPlayer->SetHandState(hand, EHandState::Idle);
			SecondHandSlotState = EHandSide::None;

			bReturnSecondHandTransform = true;
			SetActorTickEnabled(true);
		}
	}
}

void ARifle::DropRifle() {
	if (IsValid(CurrentPlayer)) {
		if (SecondHandSlotState != EHandSide::None) {
			CurrentPlayer->SetHandState(SecondHandSlotState, EHandState::Idle);
			if (IsValid(SecondHandMesh)) {
				SecondHandMesh->AttachToComponent(TargetSecondHandController, FAttachmentTransformRules::KeepWorldTransform);
				SecondHandMesh->SetRelativeTransform(APlayerPawn::GetBasicHandTransform(SecondHandSlotState));
			}
			SecondHandSlotState = EHandSide::None;
		}
		if (MainHandSlotState != EHandSide::None) {
			CurrentPlayer->SetHandState(MainHandSlotState, EHandState::Idle);
			if (IsValid(MainHandMesh)) {
				MainHandMesh->SetRelativeTransform(APlayerPawn::GetBasicHandTransform(MainHandSlotState));
			}
			MainHandSlotState = EHandSide::None;
		}

		SetActorTickEnabled(false);
		GetStaticMeshComponent()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		//DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		CurrentPlayer->CurrentRifle = nullptr;
		CurrentPlayer = nullptr;
	}
}

void ARifle::SetShootingState(bool IsTriggerPressed)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, "ARifle::SetShootingState");

	if (hTimerShooting.IsValid()) {
		GetWorldTimerManager().ClearTimer(hTimerShooting);
		hTimerShooting.Invalidate();
	}
	/*
	if (hTimerCooldown.IsValid()) {
		GetWorldTimerManager().ClearTimer(hTimerShooting);
		hTimerCooldown.Invalidate();
	}
	*/
	if (IsTriggerPressed) {
		float interval = UseSecondBarrel ? ShootingInterval_SecondBarrel : ShootingInterval_MainBarrel;
		GetWorldTimerManager().SetTimer(hTimerShooting, this, &ARifle::Timer_Shooting, interval, true, 0.0f);

		if (!hTimerCooldown.IsValid()) {
			GetWorldTimerManager().SetTimer(hTimerCooldown, this, &ARifle::Timer_Cooldown, 0.1f, true);
		}
	}
	else {
		if (!hTimerCooldown.IsValid() && Temperature > 0.0f) {
			GetWorldTimerManager().SetTimer(hTimerCooldown, this, &ARifle::Timer_Cooldown, 0.1f, true);
		}
	}
}

void ARifle::SetBarrelState(bool IsSecondaryBarrelActive)
{
	if (IsValid(PuppetRifle)) {
		PuppetRifle->SetBarrelState(IsSecondaryBarrelActive);
	}

	UseSecondBarrel = IsSecondaryBarrelActive;

	if (hTimerShooting.IsValid()) {
		GetWorldTimerManager().ClearTimer(hTimerShooting);
		hTimerShooting.Invalidate();

		float interval = UseSecondBarrel ? ShootingInterval_SecondBarrel : ShootingInterval_MainBarrel;
		GetWorldTimerManager().SetTimer(hTimerShooting, this, &ARifle::Timer_Shooting, interval, true, 0.0f);
	}
}

void ARifle::Timer_Shooting()
{
	float tempinc = UseSecondBarrel ? ShotHeating_SecondBarrel : ShotHeating_MainBarrel;
	float interval = UseSecondBarrel ? ShootingInterval_SecondBarrel : ShootingInterval_MainBarrel;
	if (!NeedFullCooldown) Temperature += tempinc;
	//if (interval < CooldownTime) Temperature -= (interval / CooldownTime);
	//if (Temperature < 0.0f) Temperature = 0.0f;

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, "ARifle::Timer_Shooting");

	if (Temperature < 1.0f && !NeedFullCooldown) {
		if (IsValid(PuppetRifle)) {
			PuppetRifle->Fire();
		}
		else {
			Fire();
		}
	}
	else {
		Temperature = 1.0f;
		NeedFullCooldown = true;
		//GetWorldTimerManager().SetTimer(hTimerCooldown, this, &ARifle::Timer_Cooldown, CooldownTime, false);
	}
	if (IsValid(mti_Indicator)) {
		mti_Indicator->SetScalarParameterValue(TEXT("Value"), Temperature);
	}
}

void ARifle::Timer_Cooldown()
{
	const float TimerInterval = 0.1f;
	Temperature -= (TimerInterval / CooldownTime);

	if (Temperature <= 0.0f) {
		Temperature = 0.0f;
		NeedFullCooldown = false;
		if (!hTimerShooting.IsValid()) {
			GetWorldTimerManager().ClearTimer(hTimerCooldown);
			hTimerCooldown.Invalidate();
		}
	}

	if (IsValid(mti_Indicator)) {
		mti_Indicator->SetScalarParameterValue(TEXT("Value"), Temperature);
	}
}

void ARifle::Fire()
{
	if ((!UseSecondBarrel && ProjectileClass_MainBarrel != NULL) || (UseSecondBarrel && ProjectileClass_SecondBarrel != NULL)) {
		FVector vSrcPoint;
		FRotator rDirection;
		AActor* tmp;
		const float Diversity = 1.5f;

		if (UseSecondBarrel) {
			vSrcPoint = GetStaticMeshComponent()->GetSocketLocation(SOCKET_FIRE2);
			rDirection = GetStaticMeshComponent()->GetSocketRotation(SOCKET_FIRE2);
			rDirection.Yaw += FMath::FRandRange(-Diversity, Diversity);
			rDirection.Pitch += FMath::FRandRange(-Diversity, Diversity);
			tmp = GetWorld()->SpawnActor<AActor>(ProjectileClass_SecondBarrel, vSrcPoint, rDirection);
		}
		else {
			vSrcPoint = GetStaticMeshComponent()->GetSocketLocation(SOCKET_FIRE1);
			rDirection = GetStaticMeshComponent()->GetSocketRotation(SOCKET_FIRE1);
			rDirection.Yaw += FMath::FRandRange(-Diversity, Diversity);
			rDirection.Pitch += FMath::FRandRange(-Diversity, Diversity);
			tmp = GetWorld()->SpawnActor<AActor>(ProjectileClass_MainBarrel, vSrcPoint, rDirection);
		}

		if (IsValid(FireSound)) {
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, vSrcPoint, 1.0f);
		}
		if (IsUsable)
			tmp->Tags.Add("PLAYER");
		else
			tmp->Tags.Add("NPC");

		tmp->SetLifeSpan(3.0f);
	}
}

void ARifle::StartPosessRifle(ARifle* RifleToPosess)
{
	if (IsValid(PuppetRifle)) StopPosessRifle();

	PuppetRifle = RifleToPosess;
	PuppetRifle->ProjectileClass_MainBarrel = ProjectileClass_MainBarrel;
	PuppetRifle->ProjectileClass_SecondBarrel = ProjectileClass_SecondBarrel;
	if (IsValid(mti_Body)) {
		mti_Body->SetScalarParameterValue(TEXT("IsPosessed"), 1.0f);
	}
}

void ARifle::StopPosessRifle()
{
	if (IsValid(PuppetRifle)) {
		PuppetRifle = nullptr;
	}

	if (IsValid(mti_Body)) {
		mti_Body->SetScalarParameterValue(TEXT("IsPosessed"), 0.0f);
	}
}

void ARifle::AttachToTeleportingPawn(USkeletalMeshComponent* PawnMesh, EHandSide Hand)
{
	FName socket;
	if (Hand == EHandSide::Right)
		socket = SOCKET_MAIN_HAND_R;
	else
		socket = SOCKET_MAIN_HAND_R;

	this->AttachToComponent(PawnMesh, FAttachmentTransformRules::SnapToTargetIncludingScale/*, socket*/);
}