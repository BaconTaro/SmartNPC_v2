// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableComponent.h" // 包含你的Component头文件
#include "InteractableActor.generated.h"

UCLASS()
class SMARTNPC_V1_API AInteractableActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractableActor();

	// 可在蓝图访问
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Interaction")
	//UPROPERTY(VisibleAnywhere, Instanced, Category = "Interaction")
	UInteractableComponent* InteractableComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	FString ActorID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable")
	FString ObjectDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable")
	bool Interacted = false;

	UFUNCTION(BlueprintCallable, Category = "Interactable")
	FString GetObjectDescription();

	UFUNCTION(BlueprintCallable, Category = "Interactable")
	FString GetCurrentStageDescription();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	FString Interact();

	FName GetNextStage();

	bool IsActorInteracted();

	void SetActorInteracted(bool result);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
