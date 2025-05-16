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
    FString Speak;

    UPROPERTY(BlueprintReadWrite)
    FString Mood;
};

// 后续再说
//UENUM(BlueprintType) 
//enum class EAIMessageRole : uint8
//{
//    User       UMETA(DisplayName = "User"),
//    Assistant  UMETA(DisplayName = "Assistant"),
//    System     UMETA(DisplayName = "System")
//};

USTRUCT(BlueprintType)

struct FAIMessage
{
    GENERATED_BODY()

    // user / assistant / system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Message")
    FString Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Message")
    FString Content;
};
// 广播事件：对话完成后的回复
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGPTReplyReceived, const FString&, GPTReply, const FParsedCommand&, Command);

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
    void SendMessageWithContext(const FString& PersonaPrompt, const TArray<FAIMessage>& History, const FString& Message);


    // GPT 回复广播（给 HUD 使用）
    UPROPERTY(BlueprintAssignable, Category = "GPT")
    FOnGPTReplyReceived OnGPTReplyReceived;

    UPROPERTY(BlueprintAssignable)
    FOnParsedCommand OnParsedCommand;

    FString LoadedSystemPrompt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    TArray<FAIMessage> ChatHistory;

    void LoadPromptConfig();
    void LogConversationToFile(const FString& Role, const FString& Message);

    FString GenerateEnvironmentPrompt();

    UFUNCTION(BlueprintCallable)
    void LoadPromptFromTxt();

private:
    FString BuildSystemContext();
    void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    bool ParseGPTReply(const FString& GPTReply, FParsedCommand& OutCommand);

};


