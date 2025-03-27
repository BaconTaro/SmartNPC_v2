#include "ChatGPTHandler.h"
#include "Http.h"
#include "Json.h"
#include "JsonUtilities/Public/JsonUtilities.h"
#include "Components/ActorComponent.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

UChatGPTHandler::UChatGPTHandler()
{
    PrimaryComponentTick.bCanEverTick = true; // 组件可以在游戏中进行周期性更新
}

void UChatGPTHandler::BeginPlay()
{
    Super::BeginPlay();

    // 从配置文件加载 Prompt
    PersonaPrompt = LoadPromptFromConfig(RoleName);

    UE_LOG(LogTemp, Warning, TEXT("为角色 [%s] 加载 Prompt: %s"), *RoleName.ToString(), *PersonaPrompt);
}

void UChatGPTHandler::SendRequestToGPT(const FString& UserMessage)
{
    CreateHttpRequest(UserMessage);
}

void UChatGPTHandler::CreateHttpRequest(const FString& UserMessage)
{
    // 这里的请求体根据需要调整
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetStringField(TEXT("model"), TEXT("gpt-3.5-turbo"));

    TArray<TSharedPtr<FJsonValue>> MessagesArray;
    TSharedPtr<FJsonObject> MessageObject = MakeShareable(new FJsonObject());
    MessageObject->SetStringField(TEXT("role"), TEXT("user"));
    MessageObject->SetStringField(TEXT("content"), UserMessage);
    MessagesArray.Add(MakeShareable(new FJsonValueObject(MessageObject)));

    JsonObject->SetArrayField(TEXT("messages"), MessagesArray);

    FString Content;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Content);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(TEXT("https://api.openai.com/v1/chat/completions"));
    HttpRequest->SetVerb(TEXT("POST"));
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    HttpRequest->SetHeader(TEXT("Authorization"), TEXT("Bearer YOUR_API_KEY"));
    HttpRequest->SetContentAsString(Content);

    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UChatGPTHandler::OnResponseReceived);
    HttpRequest->ProcessRequest();
}

void UChatGPTHandler::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("请求失败或响应无效！"));
        return;
    }

    FString ResponseContent = Response->GetContentAsString();
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("JSON 解析失败！"));
        return;
    }

    // 检查 choices 字段
    const TArray<TSharedPtr<FJsonValue>>* ChoicesArray;
    if (!JsonObject->TryGetArrayField(TEXT("choices"), ChoicesArray) || !ChoicesArray || ChoicesArray->Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("choices 数组为空或不存在！"));
        return;
    }

    // 获取 message 对象
    TSharedPtr<FJsonObject> ChoiceObject = (*ChoicesArray)[0]->AsObject();
    if (!ChoiceObject.IsValid() || !ChoiceObject->HasField(TEXT("message")))
    {
        UE_LOG(LogTemp, Error, TEXT("choices[0] 没有有效的 message 字段"));
        return;
    }

    TSharedPtr<FJsonObject> MessageObject = ChoiceObject->GetObjectField(TEXT("message"));
    FString GPTReply = MessageObject->GetStringField(TEXT("content"));

    UE_LOG(LogTemp, Warning, TEXT("GPT 回复：%s"), *GPTReply);

    // 👉 你可以在这里调用一个蓝图事件广播，把 GPTReply 发给 UI 显示
}

void UChatGPTHandler::SendMessageToGPT(const FString& Message)
{
    CreateHttpRequest(Message); // 这个是你之前写的 HTTP 请求逻辑
}

FString UChatGPTHandler::LoadPromptFromConfig(const FName& RoleKey)
{
    FString FilePath = FPaths::ProjectContentDir() + "Config/NPCPrompts.json";
    FString JsonRaw;

    if (!FFileHelper::LoadFileToString(JsonRaw, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("无法读取 NPCPrompts.json"));
        return TEXT("");
    }

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonRaw);

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("解析 JSON 失败"));
        return TEXT("");
    }

    FString OutPrompt;
    if (!JsonObject->TryGetStringField(RoleKey.ToString(), OutPrompt))
    {
        UE_LOG(LogTemp, Warning, TEXT("未找到角色：%s"), *RoleKey.ToString());
        return TEXT("");
    }

    return OutPrompt;
}