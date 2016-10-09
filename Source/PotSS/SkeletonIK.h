// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "PlayerPawn.h"
#include "SkeletonIK.generated.h"

#define HEIGHT_INIT   0
#define HEIGHT_MODIFY 1
#define HEIGHT_STABLE 2


UCLASS( ClassGroup=(Custom), abstract, BlueprintType, Blueprintable)
class POTSS_API USkeletonIK : public UActorComponent
{
	GENERATED_BODY()

public:	
	USkeletonIK();
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	float TimerInterval;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Skeleton IK")
	void CalcFrame(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Skeleton IK")
	void Initialize(FVector HeadToNeckOffset, FVector NeckToPelvisOffset, float InitHeight, float HeadHeight, USceneComponent* CameraComp, USceneComponent* LeftHandComp, USceneComponent* RightHandComp);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skeleton IK")
	FIKSkeletonData GetSkeleton() { return SkeletonTransformData; };

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skeleton IK")
	FTransform GetLastFootTargetTransform(EHandSide FootSide);

	UFUNCTION(BlueprintCallable, Category = "Skeleton IK")
	void SetIKAvatarLocation(FVector Location) { IKAvatarLocation = Location; };

	UFUNCTION(BlueprintCallable, Category = "Skeleton IK")
	void ResetFeetLocations() { ResetFootLocationR = true; ResetFootLocationL = true; };

private:
	UPROPERTY()
	USceneComponent* VRCamera;

	UPROPERTY()
	USceneComponent* MControllerRight;

	UPROPERTY()
	USceneComponent* MControllerLeft;

	UPROPERTY()
	class APlayerPawn* OwningPawn;

	UPROPERTY()
	float IKSpineLength;

	UPROPERTY()
	FRotator TargetCharacterPelvisRot;

	UPROPERTY()
	FIKSkeletonData SkeletonTransformData;

	UPROPERTY()
	FVector IKAvatarLocation;

	UPROPERTY()
	FTimerHandle hVRInputTimer;

	UPROPERTY()
	FTimerHandle hResetFootLTimer;

	UPROPERTY()
	FTimerHandle hResetFootRTimer;

	UPROPERTY()
	uint8 CurrentVRInputIndex;

	FVRInputData VRInputData[CORR_SAVE_POINTS_NUM];

	float cora[3][CORR_POINTS_NUM];
	float corb[3][CORR_POINTS_NUM];
	float corc[3][CORR_POINTS_NUM];

	float corloca[3][CORR_POINTS_NUM];
	float corlocb[3][CORR_POINTS_NUM];
	float corlocc[3][CORR_POINTS_NUM];

	UPROPERTY()
	FVector TargetCharacterNeckOffset;

	UPROPERTY()
	bool ResetFootLocationL;

	UPROPERTY()
	bool ResetFootLocationR;

	UPROPERTY()
	uint8 nModifyHeightState; // 0 - waiting; 1 - modify on; 2 - don't modify

	UPROPERTY()
	float ModifyHeightStartTime;

private:
	UFUNCTION()
	void VRInputTimer_Tick();

	UFUNCTION()
	void ResetFootsTimerL_Tick();

	UFUNCTION()
	void ResetFootsTimerR_Tick();

	UFUNCTION()
	void GetCorrelationKoef(bool bCalcRotation, int32 StartIndex = -1);

	UFUNCTION()
	inline float GetAngleToInterp(float Current, float Target);

	UFUNCTION()
	inline float DeltaAngle(FVector Angle1, FVector Angle2) { return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Angle1, Angle2))); };
};

// Корреляция наборов двух векторов
// a[3][Num], b[3][Num] - векторные массивы данных, r1, r2, r3 - коэффициенты корреляции
void SkIK_CorrelateArraysExt(float a[][CORR_POINTS_NUM], float b[][CORR_POINTS_NUM], int32 Num, float &r1, float &r2, float &r3);

// a - velocity set[3][Num], index 0..3 - array indexes to compare, r - return value
void SkIK_CorrelateFloatsExt(float* a, float* b, int32 Num, float &r);

// Находит примерный центр вращения по четырем точкам
FVector SkIK_CalcPointRotationCentreExt(float* locdata[CORR_POINTS_NUM]);