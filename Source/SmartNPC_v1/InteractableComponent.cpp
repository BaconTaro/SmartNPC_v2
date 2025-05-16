// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableComponent.h"
#include "GPTManager.h"
#include "GlobalGameInstance.h"

// Sets default values for this component's properties
UInteractableComponent::UInteractableComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

FString UInteractableComponent::GetCurrentStageDescription() const
{
	if (const FInteractionStageData* StageData = Stages.Find(CurrentStage))
	{
		return StageData->Description;
	}

	return TEXT("Unknown Stage");
}

FName UInteractableComponent::GetNextStage()
{
    FName nextStage;

    if (const FInteractionStageData* StageData = Stages.Find(CurrentStage))
    {
        nextStage = StageData->NextStage; // 不再加 FName
    }

    return nextStage;
}

FString UInteractableComponent::Interact()
{
        
        if (const FInteractionStageData* StageData = Stages.Find(CurrentStage)) 
        {
            FString Description = StageData->Description;
            UE_LOG(LogTemp, Warning, TEXT("描述是: %s"), *StageData->Description);
            // 构造发送给 LLM 的上下文
            //FString PersonaPrompt = TEXT("");


            // 创建 JSON 对象
            TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
            JsonObject->SetStringField(TEXT("description"), Description);
            JsonObject->SetStringField(TEXT("actionKeyword"), StageData -> ActionKeyword);

            // 构造字符串数组字段
            TArray<TSharedPtr<FJsonValue>> JsonArray;
            for (const FString& Action : StageData->PossibleActions)
            {
                JsonArray.Add(MakeShareable(new FJsonValueString(Action)));
            }
            JsonObject->SetArrayField(TEXT("possibleActions"), JsonArray);

            // 序列化为字符串
            FString OutputString;
            TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
            FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);


            // 可选：切换到下一阶段
            if (!StageData->NextStage.IsNone())
            {
                CurrentStage = StageData->NextStage;
            }

            return Description; // 作为交互结果返回
        }


    

    return TEXT("当前阶段无效");
}

// Called when the game starts
void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

