// Fill out your copyright notice in the Description page of Project Settings.

#include "PotSS.h"
#include "NavigationLocatior.h"


// Sets default values
ANavigationLocatior::ANavigationLocatior()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ANavigationLocatior::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANavigationLocatior::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

