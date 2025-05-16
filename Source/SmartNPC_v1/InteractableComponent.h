// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableComponent.generated.h"

USTRUCT(BlueprintType)
struct FActionKeywords
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    TArray<FString> Keywords;
};

// ✅ 可用于蓝图的交互阶段结构体
USTRUCT(BlueprintType)
struct FInteractionStageData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FName NeedItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FName GiveItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FName NextStage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString ActionKeyword;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    TArray<FString> PossibleActions;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SMARTNPC_V1_API UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractableComponent();

    // 当前阶段的描述信息（供蓝图和C++访问）
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    FString GetCurrentStageDescription() const;

    // 当前阶段名称
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FName CurrentStage;

    // 阶段映射表
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    TMap<FName, FInteractionStageData> Stages;

    // 背包数据：可选扩展
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    TMap<FName, FInteractionStageData> Inventory;

    FName GetNextStage();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    FString Interact();
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
