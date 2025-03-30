// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Delegates/DelegateCombinations.h"
#include <Interfaces/IHttpRequest.h>
#include "GPTManager.generated.h"
/**
 * 
 */

 // 广播事件：对话完成后的回复
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGPTReplyReceived, const FString&, GPTReply);

UCLASS()
class SMARTNPC_V1_API UGPTManager : public UObject
{
	GENERATED_BODY()
public:
    UGPTManager();

    // 单例访问
    static UGPTManager* GetGPTManager(UObject* WorldContext);

    // 蓝图可调用的发送消息方法
    UFUNCTION(BlueprintCallable, Category = "GPT")
    void SendMessageWithContext(const FString& PersonaPrompt, const TArray<FString>& History, const FString& Message);


    // GPT 回复广播（给 HUD 使用）
    UPROPERTY(BlueprintAssignable, Category = "GPT")
    FOnGPTReplyReceived OnGPTReplyReceived;

    FString LoadedSystemPrompt;

    void LoadPromptConfig();
    void LogConversationToFile(const FString& Role, const FString& Message);

    UFUNCTION(BlueprintCallable)
    void LoadPromptFromTxt();

private:
    void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

};
