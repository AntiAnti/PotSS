// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "PlayerFirstPersonController.generated.h"

/*******************************************************************************************************************/

class APlayerPawn;
class APlayerCharacter;
class ASlidingPawn;

/*******************************************************************************************************************/

#define _REFRESH_PAWN(a) if (!IsValid(a)) a = Cast<APlayerPawn>(APlayerController::GetPawn());

/*******************************************************************************************************************/
/****************************************************** ENUMS ******************************************************/
/*******************************************************************************************************************/

UENUM(BlueprintType, Blueprintable)
enum class EHandSide : uint8
{
	Right				UMETA(DisplayName = "Right"),
	Left				UMETA(DisplayName = "Left"),
	None				UMETA(DisplayName = "None")
};

UENUM(BlueprintType, Blueprintable)
enum class ELevelBlock : uint8
{
	Coridor_Basic		UMETA(DisplayName = "Coridor Basic"),
	Coridor_Doors		UMETA(DisplayName = "Coridor Doors"),
	Coridor_X			UMETA(DisplayName = "Coridor X"),
	Coridor_T			UMETA(DisplayName = "Coridor T"),
	Coridor_L			UMETA(DisplayName = "Coridor L"),
	Coridor_DeadEnd		UMETA(DisplayName = "Coridor Dead End"),
	Elevator			UMETA(DisplayName = "Elevator"),
	Passage_Basic		UMETA(DisplayName = "Passage Basic"),
	Passage2Coridor		UMETA(DisplayName = "Passage To Coridor"),
	FlatWall_Window		UMETA(DisplayName = "Flat Wall with Window"),
	FlatWall_NoWindow	UMETA(DisplayName = "Flat Wall without Window")
};

UENUM(BlueprintType, Blueprintable)
enum class EHandState : uint8
{
	Idle				UMETA(DisplayName = "Idle"),
	Point				UMETA(DisplayName = "Point"),
	RifleMain			UMETA(DisplayName = "Rifle Main"),
	RifleSupport		UMETA(DisplayName = "Rifle Support"),
	Fist				UMETA(DisplayName = "Fist")
};

/*******************************************************************************************************************/
/*******************************************************************************************************************/
/*******************************************************************************************************************/

/**
 * 
 */
UCLASS()
class POTSS_API APlayerFirstPersonController : public APlayerController
{
	GENERATED_BODY()
	
public:
	APlayerFirstPersonController();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "View Switch")
	void SetThirdPersonView();

	UFUNCTION(BlueprintCallable, Category = "View Switch")
	void SetFirstPersonView();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "View Switch")
	bool IsThirdPersonView() { return bIsThirdPersonView; };

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "View Switch")
	APawn* GetActivePawn();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "View Switch")
	FVector GetActivePawnHead();

	UFUNCTION(BlueprintCallable, Category = "View Switch")
	void SetUsingDirectTeleportControl(bool State);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "View Switch")
	bool IsUsingDirectTeleportControl() { return bUseDirectTeleportControl; };

protected:
	UPROPERTY(BlueprintReadWrite, Category = "PlayerFirstPersonController")
	APlayerPawn* OwnerPawn;

	UPROPERTY(BlueprintReadWrite, Category = "PlayerFirstPersonController")
	APlayerCharacter* OwnerCharacter;

	UPROPERTY(BlueprintReadWrite, Category = "PlayerFirstPersonController")
	bool bIsThirdPersonView;

	UFUNCTION(BlueprintNativeEvent, Category = "Player")
	APlayerCharacter* SpawnThirdPersonCharacter(FTransform ActorTransform);
	virtual APlayerCharacter* SpawnThirdPersonCharacter_Implementation(FTransform ActorTransform) { return nullptr; };

private:
	UPROPERTY()
	FVector CurrentCamPoint;

	UPROPERTY()
	FVector DestinationCamPoint;

	UPROPERTY()
	FRotator CurrentCamRotation;

	UPROPERTY()
	FRotator DestinationCamRotation;

	UPROPERTY()
	ASlidingPawn* SlidingPawn;

	UPROPERTY()
	float CamMovingProgress;

	UPROPERTY()
	bool bSlideCamera;

	UPROPERTY()
	bool bUseDirectTeleportControl;

	UPROPERTY()
	bool bWTFSkipSliding;

	UFUNCTION()
	void InitSlidingPawn(FTransform tr);
};
