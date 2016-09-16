// Fill out your copyright notice in the Description page of Project Settings.

#include "PotSS.h"
#include "SlidingPawn.h"


// Sets default values
ASlidingPawn::ASlidingPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	CameraRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CameraRoot"));
	CameraRoot->SetupAttachment(GetRootComponent());

	MainCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));
	MainCamera->SetupAttachment(CameraRoot);
	MainCamera->RelativeLocation = FVector::ZeroVector;
	MainCamera->bUsePawnControlRotation = false;
}