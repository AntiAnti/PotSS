// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "PlayerJumpingPawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRunningComplete, FVector, NewLocation);

class APlayerPawn;
class ARifle;

UCLASS(Blueprintable)
class POTSS_API APlayerJumpingPawn : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerJumpingPawn();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintAssignable, BlueprintCallable)
	FRunningComplete OnRunningComplete;
	
	UPROPERTY()
	bool bUseDirectTeleportControl;

	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void ActivatePawn(APlayerPawn* Player, FVector DestinationPoint, FVector CurrentCameraPoint);

	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void DeactivatePawn();

	UFUNCTION(BlueprintCallable, Category = "Global")
	void SetRifle(ARifle* NewRifle, EHandSide Hand = EHandSide::Right);

	UFUNCTION(BlueprintCallable, Category = "Global")
	FRotator GetRifleRotation();

	UFUNCTION(BlueprintCallable, Category = "Global")
	FTransform GetRifleSocketTransform();

	UFUNCTION(BlueprintCallable, Category = "Global")
	ARifle* GetRiflePointer() {	return CurrentRifle; };

	UFUNCTION(BlueprintCallable, Category = "Global")
	bool GetMovingState() { return IsMoving; /*GetCharacterMovement()->Velocity.Size() > 0.0f);*/ };

	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void OnAnyMovingComplete();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Navigation")
	void StartRunning(FVector Destination);
	virtual void StartRunning_Implementation(FVector Destination);

	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void MoveByMasterPawn(FVector Destination);

	UFUNCTION()
	void MoveForward(float Value);

	UFUNCTION()
	void TurnAtRate(float Rate);

protected:
	UPROPERTY(BlueprintReadWrite, Category = "Navigation")
	APlayerPawn* MasterPawn;

	UPROPERTY(BlueprintReadWrite, Category = "Navigation")
	bool IsMoving;

private:
	UPROPERTY()
	ARifle* CurrentRifle;

	UPROPERTY()
	EHandSide RifleMainHand;
};
