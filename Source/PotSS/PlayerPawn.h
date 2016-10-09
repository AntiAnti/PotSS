// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "PlayerFirstPersonController.h"
#include "PlayerPawn.generated.h"

/*******************************************************************************************************************/

#define HAND_LEFT			TEXT("LEFT")
#define HAND_RIGHT			TEXT("RIGHT")
#define TAG_NAVIGATION_LINE	TEXT("NAVIGATIONLINE")
#define TAG_CUSTOM_NAV_AREA TEXT("CUSTOMNAVAREA")
#define SOCKET_NAV_POINTER	TEXT("NavPointerSocket")

#define CORR_POINTS_NUM 10
#define CORR_SAVE_POINTS_NUM 100
#define HEAD_HALF_WIDTH 15.0f

/*******************************************************************************************************************/

class ARifle;
class ANavigationLocatior;
class APlayerJumpingPawn;

/*******************************************************************************************************************/

USTRUCT(BlueprintType, Blueprintable)
struct FVRInputData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector HeadLoc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LeftHandLoc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector RightHandLoc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator HeadRot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator LeftHandRot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator RightHandRot;
};

USTRUCT(BlueprintType, Blueprintable)
struct FIKSkeletonData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float fHandLeftIK;								// floats are target IK alpha

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float fHandRightIK;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float fFootLeftIK;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float fFootRightIK;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRibcageYaw;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float fBending;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float fPelvisOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float fCharacterHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform PelvisOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform RibcageOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator BendingRot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform RifleOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform HeadOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform FootLeftTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform FootRightTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform FootLeftTransform_CurrentTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform FootRightTransform_CurrentTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EHandState LeftHandState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EHandState RightHandState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector HeadOffsetToNeck;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRibcagePitch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSittingDown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector NeckOffsetToPelvis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float fHeadHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float fJumpingOffset;
};

/*******************************************************************************************************************/

UCLASS(Blueprintable)
class POTSS_API APlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	class USceneComponent* SceneRoot;										// Root Component

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	class UArrowComponent* PawnRotationArrow;

	/******************* __SteamVR_Support__ *******************/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	class USceneComponent* CameraRoot;										// Camera Root

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* VRCamera;										// VR Camera

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	class USceneComponent* ControllersRoot;									// Controllers Root

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	class UMotionControllerComponent* MControllerLeft;						// Controller 1

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	class UMotionControllerComponent* MControllerRight;						// Controller 2

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	class USteamVRChaperoneComponent* SteamVRChaperone;						// Steam VR Chaperone
	/***********************************************************/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	class USkeletalMeshComponent* RightHandMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	class USkeletalMeshComponent* LeftHandMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navigation")
	class UStaticMeshComponent* NavigationPointerMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navigation")
	class USkeletonIK* SkeletonIK;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
	float NavigationTeleportDistance;

	UPROPERTY(BlueprintReadWrite, Category = "Player")
	EHandState RightHandState;

	UPROPERTY(BlueprintReadWrite, Category = "Player")
	EHandState LeftHandState;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	ARifle* CurrentRifle;

	UPROPERTY()
	bool bUseDirectTeleportControl;

	UPROPERTY(BlueprintReadWrite, Category = "Player")
	float MovementSpeed;

	UPROPERTY(BlueprintReadWrite, Category = "IK")
	APlayerJumpingPawn* DebugIKAvatar;

public:
	APlayerPawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetHandState(EHandSide Hand, EHandState State);

	// Set Hand State in Animation Blueprint
	UFUNCTION(BlueprintNativeEvent, Category = "Player")
	void UpdateHandState(EHandSide Hand, EHandState State);
	virtual void UpdateHandState_Implementation(EHandSide Hand, EHandState State);

	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetActive(bool IsActive);

	UFUNCTION()
	static FTransform GetBasicHandTransform(EHandSide Hand);

	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void SetNavigationLocator(ANavigationLocatior* LocatorActor) { NavigationLocatorMesh = LocatorActor; };

	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void ActivateNavigationFinder(bool State, EHandSide Side = EHandSide::Right);

	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void NavigateByFinder(EHandSide Side);

	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void SetJumpTeleportAvatar(APlayerJumpingPawn* TeleportAvatar);

	UFUNCTION(BlueprintCallable, Category = "Navigation")
	APawn* GetActivePawn();

	UFUNCTION(BlueprintCallable, Category = "Navigation")
	FVector GetActivePawnHead();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SteamVR")
	FTransform GetRifleRelativeTransform(const FTransform PelvisOffset, bool bUseHeadRotation = true);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Navigation")
	bool IsAvatarRunning() { return IsAvatarRunningNow; };

private:
	UPROPERTY()
	ANavigationLocatior* NavigationLocatorMesh;

	UPROPERTY()
	bool bNavigationFinderActivated;

	UPROPERTY()
	bool bSlideActorToNewLocation;

	UPROPERTY()
	bool bDirectAvatarMovingAround;

	UPROPERTY()
	UMaterialInstanceDynamic* mti_NavigationLine;

	UPROPERTY()
	FVector NavigationDestinationHeadPoint;

	UPROPERTY()
	FVector NavigationDestinationActorPoint;

	UPROPERTY()
	FVector ActorSlideStartLocation;

	UPROPERTY()
	float SlideInterpTime;

	UPROPERTY()
	bool IsNavigationPossible;

	UPROPERTY()
	bool IsAvatarRunningNow;

	UPROPERTY()
	APlayerJumpingPawn* NavigationAvatar;

	UPROPERTY()
	UNavigationSystem* ActiveNavSystem;

	UPROPERTY()
	bool bRightTrackpadPressed;

	UPROPERTY()
	bool bLeftTrackpadPressed;

	FCollisionQueryParams stTraceParams;

	// Setup actor components
	UFUNCTION()
	void SetupComponents();

	UFUNCTION()
	void OnAvatarRunningComplete(FVector NewLocation);

	UFUNCTION()
	FVector FindAvatarStartRunningPoint();

	UFUNCTION()
	FVector FindDirectAvatarStartRunningPoint(bool RunningTarget = false);

	UFUNCTION()
	void MoveForward(float Value);

	UFUNCTION()
	float DeltaAngle(FVector Angle1, FVector Angle2);

	UFUNCTION()
	inline float GetAngleToInterp(float Current, float Target);
};