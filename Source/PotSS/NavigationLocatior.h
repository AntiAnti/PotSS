// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "NavigationLocatior.generated.h"

UCLASS(Blueprintable)
class POTSS_API ANavigationLocatior : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANavigationLocatior();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Navigation")
	void SetState(bool IsAchievable);
	virtual void SetState_Implementation(bool IsAchievable) { bIsAchievable = IsAchievable; };

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Navigation")
	void SetVisibility(bool IsVisible);
	virtual void SetVisibility_Implementation(bool IsVisible) { bIsVisible = IsVisible; };

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Navigation")
	void SetNavigationAreaType(bool IsCustom);
	virtual void SetNavigationAreaType_Implementation(bool IsCustom) { bIsCustomNavArea = IsCustom; };

	UFUNCTION(BlueprintCallable, Category = "Navigation")
	bool IsVisible() { return bIsVisible; };

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Navigation")
	bool bIsVisible;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Navigation")
	bool bIsAchievable;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Navigation")
	bool bIsCustomNavArea;
};
