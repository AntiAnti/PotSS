// Fill out your copyright notice in the Description page of Project Settings.

#include "PotSS.h"
#include "Rifle.h"
#include "AIController.h"
#include "PlayerCharacter.h"


// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> skmPlayerBody(TEXT("SkeletalMesh'/Game/Mannequin/Character/Mesh/Mannequin_SKM.Mannequin_SKM'"));

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(30.0f, 68.0f);

	// set our turn rates for input
	BaseTurnRate = 45.0f;
	BaseLookUpRate = 45.0f;

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -68.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	if (skmPlayerBody.Object != NULL) {
		GetMesh()->SetSkeletalMesh(skmPlayerBody.Object);
	}
	this->AIControllerClass = AAIController::StaticClass();
	
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, -40.0f)); // 75.0f));
	//CameraBoom->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	CameraBoom->TargetArmLength = 110.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	bTriggerPressedRight = bTriggerPressedLeft = false;
	bThumbstickPressedRight = bThumbstickPressedLeft = false;
	bGripPressedRight = bGripPressedLeft = false;

	SetActorTickEnabled(false);
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	FActorSpawnParameters spp;
	spp.bNoFail = true;
	AttachedRifle = GetWorld()->SpawnActor<ARifle>(ARifle::StaticClass(), FTransform(), spp);
	AttachedRifle->IsUsable = false;
	AttachedRifle->SetActorLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
	AttachedRifle->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, SOCKET_SPINE_RIFLE);
	//AttachedRifle->SetActorRelativeLocation(FVector::ZeroVector);
	//AttachedRifle->SetActorRelativeRotation(FRotator::ZeroRotator);
	AttachedRifle->SetActorHiddenInGame(true);
}

// Called every frame
void APlayerCharacter::Tick( float DeltaTime )
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	// Set up gameplay key bindings
	check(InputComponent);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	InputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	InputComponent->BindAxis("MoveForwardLeft", this, &APlayerCharacter::MoveForwardL);
	//InputComponent->BindAxis("MoveForwardRight", this, &APlayerCharacter::MoveForwardR);
	//InputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	//InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &APlayerCharacter::TurnAtRate);
	//InputComponent->BindAxis("TurnRateLeft", this, &APlayerCharacter::TurnAtRateL);
	InputComponent->BindAxis("TurnRateRight", this, &APlayerCharacter::TurnAtRateR);
}

void APlayerCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::MoveForward(float Value)
{
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

void APlayerCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void APlayerCharacter::SetActive(bool IsActive, bool ShowRifle)
{
	SetActorHiddenInGame(!IsActive);
	AttachedRifle->SetActorHiddenInGame(!IsActive || !ShowRifle);
}