// Fill out your copyright notice in the Description page of Project Settings.

#include "PotSS.h"
#include "PlayerPawn.h"
#include "SteamVRFunctionLibrary.h"
#include "IHeadMountedDisplay.h"
#include "Engine.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "PlayerPawn.h"
#include "Rifle.h"
#include "SkeletonIK.h"

// Sets default values for this component's properties
USkeletonIK::USkeletonIK()
{
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = false;

	TimerInterval = 0.02f;
}


// Called when the game starts
void USkeletonIK::BeginPlay()
{
	Super::BeginPlay();

	CurrentVRInputIndex = CORR_SAVE_POINTS_NUM;
	ZeroMemory(cora, sizeof(float) * 3 * CORR_POINTS_NUM);
	ZeroMemory(corb, sizeof(float) * 3 * CORR_POINTS_NUM);
	ZeroMemory(corc, sizeof(float) * 3 * CORR_POINTS_NUM);
	ZeroMemory(corloca, sizeof(float) * 3 * CORR_POINTS_NUM);
	ZeroMemory(corlocb, sizeof(float) * 3 * CORR_POINTS_NUM);
	ZeroMemory(corlocc, sizeof(float) * 3 * CORR_POINTS_NUM);
	ResetFootLocationL = false;
	ResetFootLocationR = false;

	SkeletonTransformData.HeadOffsetToNeck = FVector(15.0f, 0.0f, -10.0f);
	SkeletonTransformData.bRibcageYaw = false;
	SkeletonTransformData.bRibcagePitch = false;
	SkeletonTransformData.fBending = 0.0f;
	SkeletonTransformData.fHandLeftIK = 0.0f;
	SkeletonTransformData.fHandRightIK = 0.0f;
	SkeletonTransformData.fFootLeftIK = 0.0f;
	SkeletonTransformData.fFootRightIK = 0.0f;
	SkeletonTransformData.fPelvisOffset = 0.0f;
	SkeletonTransformData.fCharacterHeight = 120.0f;
	SkeletonTransformData.PelvisOffset = FTransform();
	SkeletonTransformData.RibcageOffset = SkeletonTransformData.PelvisOffset;
	SkeletonTransformData.fHeadHeight = 20.f;
	TargetCharacterNeckOffset = FVector::ZeroVector;
	TargetCharacterPelvisRot = FRotator::ZeroRotator;
	IKSpineLength = 80.0f;
	IKAvatarLocation = FVector::ZeroVector;
	nModifyHeightState = 0;
	SkeletonTransformData.fJumpingOffset = 0;
}

void USkeletonIK::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}

// Main function to calculate current Skeleton state
void USkeletonIK::CalcFrame(float DeltaTime)
{
	if (!IsValid(OwningPawn)) return;

	static float LastCameraYaw = 0.0f;
	static float LastCameraPitch = 0.0f;
	static float LastCameraRoll = 0.0f;
	static FVector LastCameraLoc = FVector::ZeroVector;

	if (DeltaTime == -1.0f) DeltaTime = GetWorld()->DeltaTimeSeconds;
	FTransform cam = VRCamera->GetComponentTransform();
	FVector camloc = cam.GetTranslation();

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 1. Ribcage and pelvis /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FRotator PelvisRot = SkeletonTransformData.PelvisOffset.GetRotation().Rotator();
	FVector PelvisLoc = SkeletonTransformData.PelvisOffset.GetTranslation();
	if (SkeletonTransformData.bRibcageYaw) {
		float CurrentYaw = cam.GetRotation().Rotator().Yaw;
		float diff = CurrentYaw - LastCameraYaw;

		TargetCharacterPelvisRot.Yaw += diff;

		LastCameraYaw = CurrentYaw;

	}
	float CurrentPitch = cam.GetRotation().Rotator().Pitch;
	float CurrentRoll = cam.GetRotation().Rotator().Roll;
	if (SkeletonTransformData.bRibcagePitch && false) {
		float diffPitch = CurrentPitch - LastCameraPitch;
		float diffRoll = CurrentRoll - LastCameraRoll;
		FVector diff = camloc - LastCameraLoc;

		PelvisLoc.X -= diff.X;
		PelvisLoc.Y -= diff.Y;

		//TargetCharacterPelvisRot.Pitch += diffPitch;
		TargetCharacterPelvisRot.Pitch += diffPitch; // = FMath::RadiansToDegrees(FMath::Atan((camloc.Z - PelvisLoc.Z) / FMath::Sqrt(PelvisLoc.X*PelvisLoc.X + PelvisLoc.Y*PelvisLoc.Y)));
		TargetCharacterPelvisRot.Roll += diffRoll;
	}
	else if (SkeletonTransformData.bSittingDown && SkeletonTransformData.fJumpingOffset == 0.0f) {
		FVector diff = camloc - LastCameraLoc;

		PelvisLoc.Z += diff.Z;
		//if (PelvisLoc.Z < 5.0f) PelvisLoc.Z = 5.0f;
	}

	// ПЕРЕПРОВЕРИТЬ PelvisLoc
	if (nModifyHeightState == HEIGHT_STABLE) {
		float camZ = camloc.Z - OwningPawn->GetActorLocation().Z;
		SkeletonTransformData.fJumpingOffset = camZ - SkeletonTransformData.fCharacterHeight - SkeletonTransformData.fHeadHeight;

		if (SkeletonTransformData.fJumpingOffset > 0) {
			// в воздухе

		} else {
			// на земле
			SkeletonTransformData.fJumpingOffset = 0.0f;
			float SpineToTopHeight = SkeletonTransformData.fHeadHeight + FMath::Abs(SkeletonTransformData.NeckOffsetToPelvis.Z) + FMath::Abs(SkeletonTransformData.HeadOffsetToNeck.Z);
			PelvisLoc.Z = camZ - SpineToTopHeight;

			float koef = 1.0f - camZ / SkeletonTransformData.fCharacterHeight;
			FRotator camrot = cam.GetRotation().Rotator();
			TargetCharacterPelvisRot.Pitch = koef * camrot.Pitch;
			TargetCharacterPelvisRot.Roll = koef * camrot.Roll;

			FVector spine = TargetCharacterPelvisRot.RotateVector(SkeletonTransformData.NeckOffsetToPelvis);
			PelvisLoc.X = -spine.X;
			PelvisLoc.Y = -spine.Y;
		}
		//PelvisLoc.Z = camloc.Z - SpineToTopHeight;

	}

	LastCameraPitch = CurrentPitch;
	LastCameraRoll = CurrentRoll;
	LastCameraLoc = camloc;


	// теперь вычислить положение и ориентацию пояса
	/*
	FVector neck_offset = -cam.GetRotation().RotateVector(SkeletonTransformData.HeadOffsetToNeck);		// neck_offset и pelvis_offset - смещения относительно камеры
	FVector pelvis_offset = neck_offset - TargetCharacterNeckOffset;
	pelvis_offset.Z = PelvisLoc.Z - cam.GetTranslation().Z;
	FVector dir = neck_offset - pelvis_offset;
	//if (dir.Size() > IKSpineLength) {
	dir.Normalize();
	pelvis_offset = neck_offset - dir * IKSpineLength;
	//}
	PelvisLoc.X = pelvis_offset.X;
	PelvisLoc.Y = pelvis_offset.Y;

	DrawDebugCylinder(GetWorld(), cam.GetTranslation() + pelvis_offset - FVector(0.0f, 150.0f, 0.0f), cam.GetTranslation() + neck_offset - FVector(0.0f, 150.0f, 0.0f), 1.0f, 6, FColor::Red, false, 0.0f, 0, 1.0f);
	DrawDebugSphere(GetWorld(), cam.GetTranslation() + pelvis_offset - FVector(0.0f, 150.0f, 0.0f), 3.0f, 12, FColor::White, false, 0.0f, 0.0f, 1.0f);
	PelvisRot.Pitch = FMath::Atan(dir.Z / dir.X);
	PelvisRot.Roll = FMath::Atan(dir.Z / dir.Y);
	*/

	// теперь сдвинуть пояс, вычитая из положения камеры смещение шеи и смещение груди
	//PelvisRot = TargetCharacterPelvisRot;
	PelvisRot.Yaw = FMath::FInterpTo(GetAngleToInterp(PelvisRot.Yaw, TargetCharacterPelvisRot.Yaw), TargetCharacterPelvisRot.Yaw, DeltaTime, 5.0f);
	PelvisRot.Pitch = FMath::FInterpTo(GetAngleToInterp(PelvisRot.Pitch, TargetCharacterPelvisRot.Pitch), TargetCharacterPelvisRot.Pitch, DeltaTime, 5.0f);
	PelvisRot.Roll = FMath::FInterpTo(GetAngleToInterp(PelvisRot.Roll, TargetCharacterPelvisRot.Roll), TargetCharacterPelvisRot.Roll, DeltaTime, 5.0f);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::SanitizeFloat(PelvisRot.Yaw));

	//PelvisRot.Pitch = 0.0f;
	//PelvisRot.Roll = 0.0f;
	SkeletonTransformData.PelvisOffset.SetTranslation(PelvisLoc);
	SkeletonTransformData.PelvisOffset.SetRotation(PelvisRot.Quaternion());

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2. Head ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FTransform fp_pelvis = SkeletonTransformData.PelvisOffset;
	//fp_pelvis.SetLocation(FVector(cam.GetLocation().X, cam.GetLocation().Y, 0.0f));
	FVector fp_lepvis_loc = fp_pelvis.GetTranslation();
	fp_lepvis_loc.X += cam.GetLocation().X; fp_lepvis_loc.Y += cam.GetLocation().Y; fp_lepvis_loc.Z = 0.0f;
	fp_pelvis.SetTranslation(fp_lepvis_loc);

	FTransform tp_head = UKismetMathLibrary::ConvertTransformToRelative(fp_pelvis, cam); // head relative to pelvis
	FRotator r = tp_head.GetRotation().Rotator();
	r.Yaw *= -1.0f;
	tp_head.SetRotation(r.Quaternion());
	SkeletonTransformData.HeadOffset = tp_head;
	//SkeletonTransformData.HeadOffset.SetRotation(cam.GetRotation());

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 3. Hands //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if (IsValid(OwningPawn->CurrentRifle)) {
		// need to replace yaw to full transform
		SkeletonTransformData.RifleOffset = OwningPawn->GetRifleRelativeTransform(SkeletonTransformData.PelvisOffset, false);
		SkeletonTransformData.fHandLeftIK = 1.0f;
		SkeletonTransformData.fHandRightIK = 1.0f;

		switch (OwningPawn->RightHandState) {
			case EHandState::RifleMain:
				SkeletonTransformData.RightHandState = EHandState::RifleMain; break;
			case EHandState::RifleSupport:
				SkeletonTransformData.RightHandState = EHandState::RifleSupport; break;
			default:
				SkeletonTransformData.fHandRightIK = 0.0f; break;
		}
		switch (OwningPawn->LeftHandState) {
			case EHandState::RifleMain:
				SkeletonTransformData.LeftHandState = EHandState::RifleMain; break;
			case EHandState::RifleSupport:
				SkeletonTransformData.LeftHandState = EHandState::RifleSupport; break;
			default:
				SkeletonTransformData.fHandLeftIK = 0.0f; break;
		}
	}
	else {
		SkeletonTransformData.fHandLeftIK = 0.0f;
		SkeletonTransformData.fHandRightIK = 0.0f;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 4. Feet ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FTransform pelvis_base;
	FTransform lfoot, rfoot, lfoot_back, rfoot_front;

	// target transforms relative to pelvis floor projection
	FTransform lfoot_rel = FTransform(FRotator(90.0f, -36.869884f, 51.698578f).Quaternion(), FVector(7.42f, -19.3f, 12.0f));			// left front (normal)
	FTransform rfoot_rel = FTransform(FRotator(-90.0f, -26.565033f, 152.513519f).Quaternion(), FVector(-28.217f, 14.193f, 12.0f));		// right back (normal)
	FTransform lfoot_rel_walk = FTransform(FRotator(90.0f, -36.869884f, 51.698578f).Quaternion(), FVector(-28.42f, -19.3f, 6.0f));		// left back (walking)
	FTransform rfoot_rel_walk = FTransform(FRotator(-90.0f, -26.565033f, 152.513519f).Quaternion(), FVector(7.217f, 14.193f, 6.0f));	// right front (walking)
	//FTransform lfoot_rel_walk = FTransform(FRotator(90.0f, -36.869884f, 51.698578f).Quaternion(), FVector(7.42f, -19.3f, 6.0f) * 2.0f);
	//FTransform rfoot_rel_walk = FTransform(FRotator(-90.0f, -26.565033f, 152.513519f).Quaternion(), FVector(-28.217f, 14.193f, 6.0f) * 2.0f);

	// projection of pelvis at the floor
	FVector objectbase;
	if (IKAvatarLocation == FVector::ZeroVector) {
		objectbase = OwningPawn->GetActorLocation();
	} else {
		objectbase = IKAvatarLocation;
	}
	pelvis_base.SetTranslation(FVector(objectbase.X, objectbase.Y, OwningPawn->GetActorLocation().Z));
	pelvis_base.SetRotation(FRotator(0.0f, SkeletonTransformData.PelvisOffset.GetRotation().Rotator().Yaw, 0.0f).Quaternion());

	// calculated feet locations
	lfoot = lfoot_rel * pelvis_base;
	rfoot = rfoot_rel * pelvis_base;
	lfoot_back = lfoot_rel_walk * pelvis_base;
	rfoot_front = rfoot_rel_walk * pelvis_base;

	// compare with current location
	// left
	float dv = (SkeletonTransformData.FootLeftTransform.GetTranslation() - lfoot.GetTranslation()).Size();
	float dv2 = (SkeletonTransformData.FootLeftTransform.GetTranslation() - lfoot_back.GetTranslation()).Size();
	FVector dVec1 = SkeletonTransformData.FootLeftTransform.GetRotation().GetRightVector();
	FVector dVec2 = lfoot.GetRotation().GetRightVector();
	if (dv > 25.0f) {
		if ((dv2 - dv) > 24.0f) { // select larger distance (but small difference need to be excluded)
			lfoot = lfoot_back;
		}
		SkeletonTransformData.FootLeftTransform = lfoot;
	}
	else if (DeltaAngle(dVec1, dVec2) > 45.0f) {
		SkeletonTransformData.FootLeftTransform = lfoot;
	}
	else if (ResetFootLocationL) {
		ResetFootLocationL = false;
		SkeletonTransformData.FootLeftTransform = lfoot;
	}
	if (dv > 5.0f && !hResetFootLTimer.IsValid()) {
		OwningPawn->GetWorldTimerManager().SetTimer(hResetFootLTimer, this, &USkeletonIK::ResetFootsTimerL_Tick, 3.0f, false);
	}
	SkeletonTransformData.FootLeftTransform_CurrentTarget = lfoot;

	// right
	dv = (SkeletonTransformData.FootRightTransform.GetTranslation() - rfoot.GetTranslation()).Size();
	dv2 = (SkeletonTransformData.FootRightTransform.GetTranslation() - rfoot_front.GetTranslation()).Size();
	dVec1 = SkeletonTransformData.FootRightTransform.GetRotation().GetRightVector();
	dVec2 = rfoot.GetRotation().GetRightVector();
	if (dv > 25.0f) {
		if (dv2 - dv > 24.0f) { // select larger distance
			rfoot = rfoot_front;
		}
		SkeletonTransformData.FootRightTransform = rfoot;
	}
	else if (DeltaAngle(dVec1, dVec2) > 45.0f) {
		SkeletonTransformData.FootRightTransform = rfoot;
	}
	else if (ResetFootLocationR) {
		ResetFootLocationR = false;
		SkeletonTransformData.FootRightTransform = rfoot;
	}
	if (dv > 5.0f && !hResetFootRTimer.IsValid()) {
		OwningPawn->GetWorldTimerManager().SetTimer(hResetFootRTimer, this, &USkeletonIK::ResetFootsTimerR_Tick, 3.0f, false);
	}
	SkeletonTransformData.FootRightTransform_CurrentTarget = rfoot;

	/*
	DrawDebugCylinder(GetWorld(), pelvis_base.GetTranslation(), pelvis_base.GetTranslation() + FVector(0.0f, 0.0f, 100.0f), 1.0f, 6, FColor::Red, false, 0.0f, 0, 1.0f);
	DrawDebugCylinder(GetWorld(), pelvis_base.GetTranslation(), pelvis_base.GetTranslation() + pelvis_base.GetRotation().GetForwardVector() * 25.0f, 1.0f, 6, FColor::Red, false, 0.0f, 0, 1.0f);

	*/
	DrawDebugCylinder(GetWorld(), SkeletonTransformData.FootLeftTransform.GetTranslation(), SkeletonTransformData.FootLeftTransform.GetTranslation() + SkeletonTransformData.FootLeftTransform.GetRotation().GetRightVector() * 30.0f, 0.5f, 6, FColor::White, false, 0.0f, 0, 1.0f);
	DrawDebugCylinder(GetWorld(), SkeletonTransformData.FootRightTransform.GetTranslation(), SkeletonTransformData.FootRightTransform.GetTranslation() + SkeletonTransformData.FootRightTransform.GetRotation().GetRightVector() * 30.0f, 0.5f, 6, FColor::White, false, 0.0f, 0, 1.0f);
}

void USkeletonIK::Initialize(FVector HeadToNeckOffset, FVector NeckToPelvisOffset, float InitHeight, float HeadHeight, USceneComponent* CameraComp, USceneComponent* LeftHandComp, USceneComponent* RightHandComp)
{
	OwningPawn = Cast<APlayerPawn>(GetOwner());
	if (IsValid(OwningPawn)) {
		//DeviceHMD = GEngine->HMDDevice.Get();
		VRCamera = CameraComp;
		MControllerRight = RightHandComp;
		MControllerLeft = LeftHandComp;
		SkeletonTransformData.HeadOffsetToNeck = HeadToNeckOffset;
		SkeletonTransformData.NeckOffsetToPelvis = NeckToPelvisOffset;
		SkeletonTransformData.fCharacterHeight = InitHeight;
		SkeletonTransformData.fHeadHeight = HeadHeight;

		OwningPawn->GetWorldTimerManager().SetTimer(hVRInputTimer, this, &USkeletonIK::VRInputTimer_Tick, TimerInterval, true);
	}
}

FTransform USkeletonIK::GetLastFootTargetTransform(EHandSide FootSide)
{
	FTransform ret;
	if (FootSide == EHandSide::Right) {
		ret = SkeletonTransformData.FootRightTransform_CurrentTarget;
		SkeletonTransformData.FootRightTransform = ret;
	}
	else {
		ret = SkeletonTransformData.FootLeftTransform_CurrentTarget;
		SkeletonTransformData.FootLeftTransform = ret;
	}
	return ret;
}

void USkeletonIK::VRInputTimer_Tick()
{
	if (!IsValid(OwningPawn)) return;

	//byte!
	if (CurrentVRInputIndex == CORR_SAVE_POINTS_NUM)
		CurrentVRInputIndex = 0;
	else
		CurrentVRInputIndex++;

	/*
	FVector tr1loc, tr2loc, hmdloc;
	FRotator tr1rot, tr2rot;
	FQuat hmdrot;

	USteamVRFunctionLibrary::GetHandPositionAndOrientation(0, EControllerHand::Left, tr1loc, tr1rot);
	USteamVRFunctionLibrary::GetHandPositionAndOrientation(1, EControllerHand::Right, tr2loc, tr2rot);
	DeviceHMD->GetCurrentOrientationAndPosition(hmdrot, hmdloc);

	FTransform cam = FTransform(hmdrot, hmdloc);
	*/

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 1. Добавить текущие данные в массивы /////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	VRInputData[CurrentVRInputIndex].HeadLoc = VRCamera->GetComponentLocation() - VRCamera->GetComponentRotation().RotateVector(SkeletonTransformData.HeadOffsetToNeck);
	VRInputData[CurrentVRInputIndex].RightHandLoc = MControllerRight->GetComponentLocation();
	VRInputData[CurrentVRInputIndex].LeftHandLoc = MControllerLeft->GetComponentLocation();

	// to relative???
	VRInputData[CurrentVRInputIndex].HeadRot = VRCamera->GetComponentRotation();
	VRInputData[CurrentVRInputIndex].RightHandRot = MControllerRight->GetComponentRotation();
	VRInputData[CurrentVRInputIndex].LeftHandRot = MControllerLeft->GetComponentRotation();

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2. Посчитать коэффициенты корреляции /////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FRotator CorrelationResultRot1, CorrelationResultRot2;
	float CorrelationResultLoc1, CorrelationResultLoc2;

	// а) Сначала вращение
	float r1, r2, r3;
	GetCorrelationKoef(true); // коррелировать скорости вращения

	//CorrelateArrays(1, CORR_POINTS_NUM, r1, r2, r3);	// голова с правой рукой
	SkIK_CorrelateArraysExt(cora, corb, CORR_POINTS_NUM, r1, r2, r3);
	CorrelationResultRot1 = FRotator(r1, r2, r3);

	//CorrelateArrays(2, CORR_POINTS_NUM, r1, r2, r3);	// голова с правой рукой
	SkIK_CorrelateArraysExt(cora, corc, CORR_POINTS_NUM, r1, r2, r3);
	CorrelationResultRot2 = FRotator(r1, r2, r3);

	// б) Теперь скорости перемещения головени

	SkIK_CorrelateFloatsExt(corloca[2], corlocb[2], CORR_POINTS_NUM, CorrelationResultLoc1);	// голова и правая рука по оси z
	SkIK_CorrelateFloatsExt(corloca[2], corlocc[2], CORR_POINTS_NUM, CorrelationResultLoc2);	// голова и левая рука по оси z

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 3. Анализ коэффициентов корреляции ///////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// сначала вращение
	SkeletonTransformData.bRibcageYaw = (CorrelationResultRot1.Yaw > 0.85f && CorrelationResultRot2.Yaw > 0.85f);

	// и смещение головы
	float sumrot = 0.f, sumloc = 0.f;
	for (int i = 0; i < 4; i++) {
		sumrot += cora[0][i] * cora[0][i] + cora[2][i] * cora[2][i];
		sumloc += corloca[0][i];
	}
	//sumrot /= 4.f;
	SkeletonTransformData.bSittingDown = (CorrelationResultLoc1 > 0.85f && CorrelationResultLoc2 > 0.85f);
	SkeletonTransformData.bRibcagePitch = !SkeletonTransformData.bSittingDown && (sumrot > 3.0f); //&& (FMath::Abs(sumloc) > 1.f);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 3. Проверка на высоту камеры /////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float val = VRCamera->GetComponentLocation().Z - OwningPawn->GetActorLocation().Z; // VRCamera->GetRelativeTransform().GetTranslation().Z - GetActorLocation().Z;
	if (SkeletonTransformData.fCharacterHeight < val) {
		if (nModifyHeightState == HEIGHT_INIT && val > 2.0f /* ignore fluctuations; start with player taking headset */) {
			nModifyHeightState = HEIGHT_MODIFY;
			ModifyHeightStartTime = GetWorld()->GetRealTimeSeconds();
		}
		else if (nModifyHeightState == HEIGHT_MODIFY) {
			if (GetWorld()->GetRealTimeSeconds() - ModifyHeightStartTime > 4.0f /* 4 seconds to set height */) {
				nModifyHeightState = HEIGHT_STABLE;
			}
		}

		if (nModifyHeightState == HEIGHT_MODIFY) {
			// update player height
			SkeletonTransformData.fCharacterHeight = val;

			FVector r = SkeletonTransformData.PelvisOffset.GetTranslation();
			r.X = r.Y = 0.0f;
			r.Z = val - (FMath::Abs(SkeletonTransformData.NeckOffsetToPelvis.Z) + FMath::Abs(SkeletonTransformData.HeadOffsetToNeck.Z) + SkeletonTransformData.fHeadHeight);
			SkeletonTransformData.PelvisOffset.SetTranslation(r);
		}
		else if (nModifyHeightState == HEIGHT_STABLE) {
			SkeletonTransformData.fJumpingOffset = val - SkeletonTransformData.fCharacterHeight;
			// fix pelvis z offset mistake

			/*
			FVector r = SkeletonTransformData.PelvisOffset.GetTranslation();
			r.X = r.Y = 0.0f;
			r.Z = SkeletonTransformData.fJumpingOffset + SkeletonTransformData.fCharacterHeight - (FMath::Abs(SkeletonTransformData.NeckOffsetToPelvis.Z) + FMath::Abs(SkeletonTransformData.HeadOffsetToNeck.Z));
			SkeletonTransformData.PelvisOffset.SetTranslation(r);
			*/
		}
	}
	else {
		//SkeletonTransformData.fJumpingOffset = 0.0f;
	}
}

void USkeletonIK::ResetFootsTimerL_Tick()
{
	if (hResetFootLTimer.IsValid()) {
		ResetFootLocationL = true;
		OwningPawn->GetWorldTimerManager().ClearTimer(hResetFootLTimer);
		hResetFootLTimer.Invalidate();
	}
}

void USkeletonIK::ResetFootsTimerR_Tick()
{
	if (hResetFootRTimer.IsValid()) {
		ResetFootLocationR = true;
		OwningPawn->GetWorldTimerManager().ClearTimer(hResetFootRTimer);
		hResetFootRTimer.Invalidate();
	}
}

void USkeletonIK::GetCorrelationKoef(bool bCalcRotation, int32 StartIndex)
{
	const int num = CORR_POINTS_NUM;
	int i, index, index0, q = 0;
	if (StartIndex < 0) StartIndex = CurrentVRInputIndex;

	// выгрузить разность значений в универсальный массив данных
	// (мы берем значения через одинаковые интервалы, так что дельта [y] пропорциональна скорости;
	// дополнительное деление на дельту [t] не требуется, потому что не скажется на расчете корреляции)
	for (i = StartIndex - num; i <= StartIndex; i++, q++) {
		index = (i > 0) ? i : i + CORR_SAVE_POINTS_NUM;
		index0 = (i > 1) ? i - 1 : CORR_SAVE_POINTS_NUM - 1;

		cora[0][q] = VRInputData[index].HeadRot.Pitch;			cora[1][q] = VRInputData[index].HeadRot.Yaw;			cora[2][q] = VRInputData[index].HeadRot.Roll;
		corb[0][q] = VRInputData[index].RightHandRot.Pitch;		corb[1][q] = VRInputData[index].RightHandRot.Yaw;		corb[2][q] = VRInputData[index].RightHandRot.Roll;
		corc[0][q] = VRInputData[index].LeftHandRot.Pitch;		corc[1][q] = VRInputData[index].LeftHandRot.Yaw;		corc[2][q] = VRInputData[index].LeftHandRot.Roll;

		cora[0][q] -= VRInputData[index0].HeadRot.Pitch;		cora[1][q] -= VRInputData[index0].HeadRot.Yaw;			cora[2][q] -= VRInputData[index0].HeadRot.Roll;
		corb[0][q] -= VRInputData[index0].RightHandRot.Pitch;	corb[1][q] -= VRInputData[index0].RightHandRot.Yaw;		corb[2][q] -= VRInputData[index0].RightHandRot.Roll;
		corc[0][q] -= VRInputData[index0].LeftHandRot.Pitch;	corc[1][q] -= VRInputData[index0].LeftHandRot.Yaw;		corc[2][q] -= VRInputData[index0].LeftHandRot.Roll;

		corloca[0][q] = VRInputData[index].HeadLoc.X;			corloca[1][q] = VRInputData[index].HeadLoc.Y;			corloca[2][q] = VRInputData[index].HeadLoc.Z;
		corlocb[0][q] = VRInputData[index].RightHandLoc.X;		corlocb[1][q] = VRInputData[index].RightHandLoc.Y;		corlocb[2][q] = VRInputData[index].RightHandLoc.Z;
		corlocc[0][q] = VRInputData[index].LeftHandLoc.X;		corlocc[1][q] = VRInputData[index].LeftHandLoc.Y;		corlocc[2][q] = VRInputData[index].LeftHandLoc.Z;

		corloca[0][q] -= VRInputData[index0].HeadLoc.X;			corloca[1][q] -= VRInputData[index0].HeadLoc.Y;			corloca[2][q] -= VRInputData[index0].HeadLoc.Z;
		corlocb[0][q] -= VRInputData[index0].RightHandLoc.X;	corlocb[1][q] -= VRInputData[index0].RightHandLoc.Y;	corlocb[2][q] -= VRInputData[index0].RightHandLoc.Z;
		corlocc[0][q] -= VRInputData[index0].LeftHandLoc.X;		corlocc[1][q] -= VRInputData[index0].LeftHandLoc.Y;		corlocc[2][q] -= VRInputData[index0].LeftHandLoc.Z;
	}
}

inline float USkeletonIK::GetAngleToInterp(float Current, float Target)
{
	float fRet = Current;

	if (FMath::Abs(Current - Target) > 180.0f) {
		if (Current < Target) fRet += 360.0f; else fRet -= 360.0f;
	}

	return fRet;
}

// Internal 3-dimentional correlation function
// a, b - data arrays[3][Num]
// Num - array s ize
// r1, r2, r3 - return values
void SkIK_CorrelateArraysExt(float a[][CORR_POINTS_NUM], float b[][CORR_POINTS_NUM], int32 Num, float &r1, float &r2, float &r3)
{
	float x_[3], y_[3], x2_[3], y2_[3], xy_[3], result[3];

	for (int n = 0; n < 3; n++) {
		x_[n] = 0.0f; y_[n] = 0.0f; x2_[n] = 0.0f; y2_[n] = 0.0f; xy_[n] = 0.0f;
	}

	for (int i = 0; i < Num; i++) {
		for (int n = 0; n < 3; n++) {
			// простая сумма
			x_[n] += a[n][i]; y_[n] += b[n][i];
			// сумма квадратов
			x2_[n] += (a[n][i] * a[n][i]); y2_[n] += (b[n][i] * b[n][i]);
			// сумма произведения
			xy_[n] += (a[n][i] * b[n][i]);
		}
	}

	// от суммы к МО
	for (int n = 0; n < 3; n++) {
		x2_[n] /= (float)Num;
		y2_[n] /= (float)Num;
		x_[n] /= (float)Num;
		y_[n] /= (float)Num;
		xy_[n] /= (float)Num;
	}

	// вычисление r
	float sigma_x2, sigma_y2;
	for (int n = 0; n < 3; n++) {
		sigma_x2 = x2_[n] - x_[n] * x_[n];
		sigma_y2 = y2_[n] - y_[n] * y_[n];
		// r = (среднее произведений - произведение средних) / (произведение сигм)
		result[n] = (xy_[n] - x_[n] * y_[n]) / (FMath::Sqrt(sigma_x2 * sigma_y2));
	}

	// результат
	r1 = result[0];
	r2 = result[1];
	r3 = result[2];
}

void SkIK_CorrelateFloatsExt(float* a, float* b, int32 Num, float &r)
{
	float x_ = 0.0f, y_ = 0.0f, x2_ = 0.0f, y2_ = 0.0f, xy_ = 0.0f, result;

	for (int i = 0; i < Num; i++) {
		// простая сумма
		x_ += a[i];
		y_ += b[i];
		// сумма квадратов
		x2_ += (a[i] * a[i]);
		y2_ += (b[i] * b[i]);
		// сумма произведения
		xy_ += (a[i] * b[i]);
	}

	// от суммы к МО
	x2_ /= (float)Num;
	y2_ /= (float)Num;
	x_ /= (float)Num;
	y_ /= (float)Num;
	xy_ /= (float)Num;

	// вычисление r
	float sigma_x2, sigma_y2;
	sigma_x2 = x2_ - x_ * x_;
	sigma_y2 = y2_ - y_ * y_;
	// r = (среднее произведений - произведение средних) / (произведение сигм)
	result = (xy_ - x_ * y_) / (FMath::Sqrt(sigma_x2 * sigma_y2));

	// результат
	r = result;
}

FVector SkIK_CalcPointRotationCentreExt(float* locdata[CORR_POINTS_NUM])
{
	/*
	1) нормаль к треугольнику 1 по трем точкам
	2) нормаль к треугольнику 2 по трем точкам
	3) место пересечения [массив мест пересечения]
	--> центр вращения
	откладываем от головы длину спины.
	*/

	FVector lin1, LinDir1, lin2, LinDir2;
	FVector v1, v2;

	// 1. Уравнения прямых-нормалей для двух треугольников
	int index = 0;

	lin1 = FVector(locdata[0][index + 1], locdata[1][index + 1], locdata[2][index + 1]);
	v1 = lin1 - FVector(locdata[0][index + 0], locdata[1][index + 0], locdata[2][index + 0]);
	v2 = lin1 - FVector(locdata[0][index + 2], locdata[1][index + 2], locdata[2][index + 2]);
	LinDir1 = v1 * v2;
	LinDir1.Normalize();

	index = 1;

	lin2 = FVector(locdata[0][index + 1], locdata[1][index + 1], locdata[2][index + 1]);
	v1 = lin2 - FVector(locdata[0][index + 0], locdata[1][index + 0], locdata[2][index + 0]);
	v2 = lin2 - FVector(locdata[0][index + 2], locdata[1][index + 2], locdata[2][index + 2]);
	LinDir2 = v1 * v2;
	LinDir2.Normalize();

	// 2. Найдем плоскость W которой принадлежит прямая 2, параллельную прямой 1.
	FVector WNorm = LinDir1 * LinDir2;
	WNorm.Normalize();
	// W = (lin2, WNorm);
	//     WNorm.X * (x - lin2.x) + WNorm.Y * (y - lin2.y) + WNorm.Z * (z - lin2.z) = 0
	//	   WNorm.X * x + WNorm.Y * y + WNorm.Z * z - (WNorm.X * lin2.x + WNorm.Y * lin2.y + WNorm.Z * lin2.z) = 0
	// v * lin2 = WNorm;

	// 3. Найдем плоскость W2, которая проходит через прямую 2, но перпендикулярна плоскости W
	FVector W2Norm = WNorm * LinDir2;
	// W2 = (lin2, W2Norm)
	//      W2Norm.X * x + W2Norm.Y * y + W2Norm.Z * z - (W2Norm.X * lin2.x + W2Norm.Y * lin2.y + W2Norm.Z * lin2.z) = 0

	// 4. Найдем точку пересечение прямой 1 с плоскостью W2
	// прямая 1 = lin1 + LinDir1 * lambda
	// W2Norm.X * (lin1.x + LinDir1.X * lambda) + W2Norm.Y * (lin1.y + LinDir1.Y * lambda) + W2Norm.Z * (lin1.z + LinDir1.Z * lambda) = (W2Norm.X * lin2.x + W2Norm.Y * lin2.y + W2Norm.Z * lin2.z)
	float p1, p2, p3;
	p1 = W2Norm.X * lin2.X + W2Norm.Y * lin2.Y + W2Norm.Z * lin2.Z;
	p2 = W2Norm.X * lin1.X + W2Norm.Y * lin1.Y + W2Norm.Z * lin1.Z;
	p3 = W2Norm.X * LinDir1.X + W2Norm.Y * LinDir1.Y + W2Norm.Z * LinDir1.Z;
	float lambda = (p1 - p2) / p3;

	FVector point = lin1 + lambda * LinDir1;

	return point;
}