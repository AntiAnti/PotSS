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
	bool bRotateRibcage;

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

	/* Correlation head - right hand */
	UPROPERTY(BlueprintReadOnly, Category = "IK")
	FVector CorrelationResultLoc1;

	/* Correlation head - left hand */
	UPROPERTY(BlueprintReadOnly, Category = "IK")
	FVector CorrelationResultLoc2;

	/* Correlation head - right hand */
	UPROPERTY(BlueprintReadOnly, Category = "IK")
	FRotator CorrelationResultRot1;

	/* Correlation head - left hand */
	UPROPERTY(BlueprintReadOnly, Category = "IK")
	FRotator CorrelationResultRot2;

	/* Data to restore third person skeleton state in anim buleprint */
	UPROPERTY(BlueprintReadOnly, Category = "IK")
	FIKSkeletonData SkeletonTransformData;

	UPROPERTY(BlueprintReadWrite, Category = "IK")
	FVector IKAvatarLocation;
	/*
	UPROPERTY(BlueprintReadWrite, Category = "IK")
	ACharacter* IKThirdPersonActor;
	*/

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
	FTransform GetRifleRelativeTransform(bool bUseHeadRotation = true/*, float nBodyRotationYaw = 0.0f*/);

	UFUNCTION(BlueprintCallable, Category = "IK")
	void CalculateBodyIKParams();

	UFUNCTION(BlueprintCallable, Category = "IK")
	void CalculateBodyIK(float DeltaTime = -1.0f);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "IK")
	FTransform GetLastFootTargetTransform(EHandSide FootSide);

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

	/*Category = "IK"*/
	UPROPERTY()
	FTimerHandle hVRInputTimer;

	UPROPERTY()
	FTimerHandle hResetFootLTimer;

	UPROPERTY()
	FTimerHandle hResetFootRTimer;

	/*Category = "IK"*/
	UPROPERTY()
	uint8 CurrentVRInputIndex;

	/*UPROPERTY(Category = "IK")*/
	FVRInputData VRInputData[CORR_SAVE_POINTS_NUM];

	/*UPROPERTY(Category = "IK")*/
	float cora[3][CORR_POINTS_NUM];
	/*UPROPERTY(Category = "IK")*/
	float corb[3][CORR_POINTS_NUM];
	/*UPROPERTY(Category = "IK")*/
	float corc[3][CORR_POINTS_NUM];

	UPROPERTY()
	float LastCameraYaw;
	/*Category = "IK"*/
	UPROPERTY()
	float TargetCharacterYaw;

	UPROPERTY()
	bool ResetFootLocationL;

	UPROPERTY()
	bool ResetFootLocationR;

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
	void VRInputTimer_Tick();

	UFUNCTION()
	void ResetFootsTimerL_Tick();

	UFUNCTION()
	void ResetFootsTimerR_Tick();

	UFUNCTION()
	void GetCorrelationKoef(bool bCalcRotation, FVector& Head2Controller1, FVector& Head2Controller2, int32 StartIndex = -1);

	UFUNCTION()
	void CorrelateArrays(uint8 CompareType, int32 Num, float &r1, float &r2, float &r3);

	UFUNCTION()
	float DeltaAngle(FVector Angle1, FVector Angle2);

	UFUNCTION()
	inline float GetAngleToInterp(float Current, float Target);
};