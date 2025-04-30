// Fill out your copyright notice in the Description page of Project Settings.


#include "SmartNPCCharacter.h"

// Sets default values
ASmartNPCCharacter::ASmartNPCCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASmartNPCCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentLocation = "Office";

	LastLocation = "None";

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("当前房间：%s"), *CurrentLocation));
	//UE_LOG(LogTemp, Warning, TEXT("当前房间：%s"), *CurrentLocation);
	//AAIController* AICon = Cast<AAIController>(GetController());
	//if (AICon)
	//{
	//	FVector TargetLocation = FVector(500, 0, 0); // 目的地位置
	//	AICon->MoveToLocation(TargetLocation);
	//}
	
}

// Called every frame
void ASmartNPCCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASmartNPCCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

