// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/StaticMeshActor.h"
#include "PlayerFirstPersonController.h"
#include "Rifle.generated.h"

#define SOCKET_FIRE1				TEXT("Fire1")
#define SOCKET_FIRE2				TEXT("Fire2")
#define SOCKET_MAIN_HAND_R			TEXT("MainHand_Right")
#define SOCKET_MAIN_HAND_L			TEXT("MainHand_Left")
#define SOCKET_SECOND_HAND_R		TEXT("SecondaryHand_Right")
#define SOCKET_SECOND_HAND_L		TEXT("SecondaryHand_Left")
#define SOCKET_COLLIDER_MAIN		TEXT("MainCollider")
#define SOCKET_COLLIDER_SECOND		TEXT("SecondaryCollider")
#define SOCKET_RIFLE_MAIN			TEXT("RifleMain")
#define SOCKET_RIFLE_SECOND			TEXT("RifleSecondary")

class APlayerPawn;
class UMotionControllerComponent;

/**
 * 
 */
UCLASS(Blueprintable)
class POTSS_API ARifle : public AStaticMeshActor
{
	GENERATED_BODY()
	
private:
	void SetupComponents();
	void SetupDefaults();

public:
	ARifle();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rifle Components")
	UBoxComponent* MainHandCollider;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rifle Components")
	UBoxComponent* SecondHandCollider;
	
protected:
	UPROPERTY(BlueprintReadWrite, Category = "Rifle Service")
	bool UseSecondBarrel;

	UPROPERTY(BlueprintReadWrite, Category = "Rifle Service")
	float Temperature;

	UPROPERTY(BlueprintReadWrite, Category = "Rifle Service")
	FTimerHandle hTimerShooting;

	UPROPERTY(BlueprintReadWrite, Category = "Rifle Service")
	FTimerHandle hTimerCooldown;

	UPROPERTY(BlueprintReadWrite, Category = "Rifle Service")
	APlayerPawn* CurrentPlayer;

	UPROPERTY(BlueprintReadWrite, Category = "Rifle Service")
	EHandSide MainHandSlotState;

	UPROPERTY(BlueprintReadWrite, Category = "Rifle Service")
	EHandSide SecondHandSlotState;

private:
	UPROPERTY()
	bool bCorrectRifleTransform;
	
	UPROPERTY()
	bool bCorrectSecondHandTransform;

	UPROPERTY()
	bool bReturnSecondHandTransform;

	UPROPERTY()
	UMotionControllerComponent* TargetSecondHandController;

	UPROPERTY()
	USkeletalMeshComponent* MainHandMesh;

	UPROPERTY()
	USkeletalMeshComponent* SecondHandMesh;

	UPROPERTY()
	EHandSide SecondHandSide;

	UPROPERTY()
	UMaterialInstanceDynamic* mti_Body;

	UPROPERTY()
	UMaterialInstanceDynamic* mti_Indicator;

	UPROPERTY()
	ARifle* PuppetRifle;

	UPROPERTY()
	USoundBase* FireSound;

	UPROPERTY()
	bool NeedFullCooldown;

public:
	/* Rifle params */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rifle Params")
	float ShootingInterval_MainBarrel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rifle Params")
	float ShootingInterval_SecondBarrel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rifle Params")
	float ShotHeating_MainBarrel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rifle Params")
	float ShotHeating_SecondBarrel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rifle Params")
	float CooldownTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rifle Params")
	bool IsUsable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rifle Projectile")
	TSubclassOf<class AActor> ProjectileClass_MainBarrel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rifle Projectile")
	TSubclassOf<class AActor> ProjectileClass_SecondBarrel;

public:
	UFUNCTION(BlueprintCallable, Category = "Rifle")
	void SetBarrelState(bool IsSecondaryBarrelActive);

	UFUNCTION(BlueprintCallable, Category = "Rifle")
	void SetShootingState(bool IsTriggerPressed);

	UFUNCTION(BlueprintCallable, Category = "Rifle")
	void DropRifle();

	UFUNCTION(BlueprintCallable, Category = "Rifle")
	void StartPosessRifle(ARifle* RifleToPosess);

	UFUNCTION(BlueprintCallable, Category = "Rifle")
	void StopPosessRifle();

	UFUNCTION(BlueprintCallable, Category = "Rifle")
	void AttachToTeleportingPawn(USkeletalMeshComponent* PawnMesh, EHandSide Hand);

	UFUNCTION(BlueprintCallable, Category = "Rifle")
	void Fire();

	UFUNCTION(BlueprintCallable, Category = "Rifle Service")
	EHandSide GetMainHandSlotState() { return MainHandSlotState; };

	UFUNCTION(BlueprintCallable, Category = "Rifle Service")
	EHandSide GetSecondHandSlotState() { return SecondHandSlotState; };

protected:
	UFUNCTION(BlueprintCallable, Category = "Rifle")
	void PickupRiffle(USkeletalMeshComponent* HandMesh);

	UFUNCTION(BlueprintCallable, Category = "Rifle")
	void AttachToSecondHand(USkeletalMeshComponent* HandMesh);

	UFUNCTION(BlueprintCallable, Category = "Rifle")
	void DetachFromSecondHand(USkeletalMeshComponent* HandMesh);

	UFUNCTION()
	void OnMainCollider_BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSecondCollider_BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSecondCollider_EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void Timer_Shooting();

	UFUNCTION()
	void Timer_Cooldown();
};
