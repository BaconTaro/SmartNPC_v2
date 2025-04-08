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


 //  放在类外部（类定义之前），这是标准 UE4 做法。 
 //  这个是LLM返回的json格式，有："action","Target","direction"和"question"之类的，具体取决于我的prompt怎么写。 后续会更新。
USTRUCT(BlueprintType)
struct FParsedCommand
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Action;

    UPROPERTY(BlueprintReadWrite)
    FString Target;

    UPROPERTY(BlueprintReadWrite)
    FString Direction;

    UPROPERTY(BlueprintReadWrite)
    FString Question;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnParsedCommand, const FParsedCommand&, Command);
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

    UPROPERTY(BlueprintAssignable)
    FOnParsedCommand OnParsedCommand;

    FString LoadedSystemPrompt;

    void LoadPromptConfig();
    void LogConversationToFile(const FString& Role, const FString& Message);

    UFUNCTION(BlueprintCallable)
    void LoadPromptFromTxt();

private:
    FString BuildSystemContext();
    void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    bool ParseGPTReply(const FString& GPTReply, FParsedCommand& OutCommand);

};


