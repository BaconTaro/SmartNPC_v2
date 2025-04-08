// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "SmartNPCCharacter.generated.h"

UCLASS()
class SMARTNPC_V1_API ASmartNPCCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASmartNPCCharacter();
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Location")
	FString CurrentLocation;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Location")
	FString LastLocation;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
