// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Delegates/DelegateCombinations.h"
#include "ChatGPTHandler.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SMARTNPC_V1_API UChatGPTHandler : public UActorComponent
{
    GENERATED_BODY()

public:
    UChatGPTHandler();

    // 玩家消息与 GPT 的交互
    UFUNCTION(BlueprintCallable, Category = "GPT")
    void SendMessageToGPT(const FString& Message);
    FString LoadPromptFromConfig(const FName& RoleKey);

    // ChatGPTHandler.h
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FName RoleName;

    // 蓝图事件：向蓝图广播回复内容
    UFUNCTION(BlueprintImplementableEvent, Category = "ChatGPT")
    void OnChatGPTReplyReceived(const FString& GPTReply);

protected:
    virtual void BeginPlay() override;
    void SendRequestToGPT(const FString& UserMessage);
    FString PersonaPrompt;
    TArray<FString> MessageHistory;

private:
    // 用于处理 HTTP 请求完成的回调函数
    void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    // 创建 HTTP 请求的函数
    void CreateHttpRequest(const FString& UserMessage);
};