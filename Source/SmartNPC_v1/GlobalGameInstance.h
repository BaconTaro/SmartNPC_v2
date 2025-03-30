// GlobalGameInstance.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GPTManager.h"
#include "GlobalGameInstance.generated.h"

UCLASS()
class SMARTNPC_V1_API UGlobalGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "GPT")
    UGPTManager* GetGPTManager();

protected:
    virtual void Init() override;

private:
    UPROPERTY()
    UGPTManager* GPTManagerInstance;
};