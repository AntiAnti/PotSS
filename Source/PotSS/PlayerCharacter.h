// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

#define SOCKET_SPINE_RIFLE TEXT("SpineRifleSocket")

class ARifle;

UCLASS()
class POTSS_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	ARifle* AttachedRifle;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	float BaseLookUpRate;

	UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
	bool bTriggerPressedLeft;

	UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
	bool bTriggerPressedRight;

	UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
	bool bThumbstickPressedLeft;

	UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
	bool bThumbstickPressedRight;

	UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
	bool bGripPressedLeft;

	UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
	bool bGripPressedRight;

public:
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetActive(bool IsActive, bool ShowRifle = true);

protected:
	UFUNCTION()
	void MoveForward(float Value);

	UFUNCTION()
	void MoveForwardL(float Value) { if (bThumbstickPressedLeft) MoveForward(Value); };

	UFUNCTION()
	void MoveForwardR(float Value) { if (bThumbstickPressedLeft) MoveForward(Value); };

	UFUNCTION()
	void MoveRight(float Value);

	UFUNCTION()
	void TurnAtRate(float Rate);

	UFUNCTION()
	void TurnAtRateL(float Rate) { if (bThumbstickPressedRight) TurnAtRate(Rate); };

	UFUNCTION()
	void TurnAtRateR(float Rate) { if (bThumbstickPressedRight) TurnAtRate(Rate); };

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
