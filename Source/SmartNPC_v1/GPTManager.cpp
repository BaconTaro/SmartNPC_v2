#include "GPTManager.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "SmartNPCCharacter.h"
#include <Kismet/GameplayStatics.h>

UGPTManager::UGPTManager()
{
    // 构造函数逻辑（可以为空）
}

void UGPTManager::SendMessageWithContext(const FString& PersonaPrompt, const TArray<FString>& History, const FString& Message)
{
    // 构建 JSON 根对象
    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject());
    RootObject->SetStringField(TEXT("model"), TEXT("Pro/deepseek-ai/DeepSeek-V3"));
    RootObject->SetBoolField(TEXT("stream"), false);
    RootObject->SetNumberField(TEXT("max_tokens"), 512);
    RootObject->SetNumberField(TEXT("temperature"), 0.7f);
    RootObject->SetNumberField(TEXT("top_p"), 0.7f);
    RootObject->SetNumberField(TEXT("top_k"), 50);
    RootObject->SetNumberField(TEXT("frequency_penalty"), 0.5f);
    RootObject->SetNumberField(TEXT("n"), 1);

    // 构建 messages 数组
    TArray<TSharedPtr<FJsonValue>> Messages;

    // 原来的传入参数改为 optional
    if (!LoadedSystemPrompt.IsEmpty())
    {
        FString SystemContext = BuildSystemContext();
        FString SystemContextFormatted = FString::Printf(TEXT("{\n%s\n}"), *SystemContext);
        FString CombinedSystemPrompt = LoadedSystemPrompt + TEXT("\n\n 大楼里可能有很多房间或位置，以下是你所处的房间或位置和时节点：\n") + SystemContextFormatted;

        TSharedPtr<FJsonObject> SystemPrompt = MakeShareable(new FJsonObject());
        SystemPrompt->SetStringField(TEXT("role"), TEXT("system"));
        SystemPrompt->SetStringField(TEXT("content"), CombinedSystemPrompt);
        Messages.Add(MakeShareable(new FJsonValueObject(SystemPrompt)));

        LogConversationToFile(TEXT("System"), CombinedSystemPrompt);
    }

    // 添加对话历史
    for (int32 i = 0; i < History.Num(); ++i)
    {
        TSharedPtr<FJsonObject> HistoryMessage = MakeShareable(new FJsonObject());
        HistoryMessage->SetStringField(TEXT("role"), (i % 2 == 0) ? TEXT("user") : TEXT("assistant"));
        HistoryMessage->SetStringField(TEXT("content"), History[i]);
        Messages.Add(MakeShareable(new FJsonValueObject(HistoryMessage)));
    }

    // 添加当前玩家消息
    TSharedPtr<FJsonObject> UserMessage = MakeShareable(new FJsonObject());
    UserMessage->SetStringField(TEXT("role"), TEXT("user"));
    UserMessage->SetStringField(TEXT("content"), Message);
    Messages.Add(MakeShareable(new FJsonValueObject(UserMessage)));

    RootObject->SetArrayField(TEXT("messages"), Messages);
    LogConversationToFile(TEXT("User"), Message);
    // 序列化为 JSON 字符串
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

    // 创建 HTTP 请求
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(TEXT("https://api.siliconflow.cn/v1/chat/completions"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("Authorization"), TEXT("Bearer sk-clmslcrlrjokqaouvzjedgqjsukwpqukwmtjlrskgdarxlux")); // 👈 记得替换你的 Key
    Request->SetContentAsString(RequestBody);

    Request->OnProcessRequestComplete().BindUObject(this, &UGPTManager::OnResponseReceived);
    Request->ProcessRequest();
}
FString UGPTManager::BuildSystemContext()
{
    FString Context;

    // 1. NPC 位置信息
    UWorld* World = GetWorld();
    if (!World) return TEXT("");

    TArray<AActor*> FoundNPCs;
    UGameplayStatics::GetAllActorsOfClass(World, ASmartNPCCharacter::StaticClass(), FoundNPCs);

    for (AActor* Actor : FoundNPCs)
    {
        ASmartNPCCharacter* NPC = Cast<ASmartNPCCharacter>(Actor);
        if (NPC)
        {
            Context += FString::Printf(TEXT("当前房间：%s\n上一个房间：%s\n\n"),
                *NPC->CurrentLocation, *NPC->LastLocation);
        }
    }

    // 🚀 可在这里继续添加更多系统信息，例如时间、任务、环境等

    return Context;
}
void UGPTManager::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) // 回调
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

    const TArray<TSharedPtr<FJsonValue>>* Choices;
    if (!JsonObject->TryGetArrayField(TEXT("choices"), Choices) || Choices->Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("返回的 choices 无效"));
        return;
    }

    TSharedPtr<FJsonObject> FirstChoice = (*Choices)[0]->AsObject();
    TSharedPtr<FJsonObject> MessageObject = FirstChoice->GetObjectField(TEXT("message"));
    FString GPTReply = MessageObject->GetStringField(TEXT("content"));

    UE_LOG(LogTemp, Warning, TEXT("GPT 回复：%s"), *GPTReply);

    // 👇 向蓝图广播回复，这个GPTReply 到此还没被parse，先留着，看看全貌，这个东西会整个被打印到游戏对话框内，方便debug用。
    OnGPTReplyReceived.Broadcast(GPTReply);
    LogConversationToFile(TEXT("GPT"), GPTReply);

    // 现在parse一下GPTReply，然后广播给蓝图。
    FParsedCommand Parsed;
    if (ParseGPTReply(GPTReply, Parsed))
    {
        // ✅ 成功解析，可以传给 NPC 控制逻辑
        UE_LOG(LogTemp, Log, TEXT("Action: %s, Target: %s, Direction: %s, Question: %s"),
            *Parsed.Action, *Parsed.Target, *Parsed.Direction, *Parsed.Question);

        // 示例：广播给蓝图 NPC 使用
        OnParsedCommand.Broadcast(Parsed);
    }
}

bool UGPTManager::ParseGPTReply(const FString& GPTReply, FParsedCommand& OutCommand)
{
    // 移除开头的 ```json 和结尾的 ```
    FString CleanedJson = GPTReply;

    CleanedJson.TrimStartAndEndInline(); // 去除首尾空格

    // 判断是否包含 Markdown 标记
    if (CleanedJson.StartsWith("```"))
    {
        int32 StartIndex = CleanedJson.Find("{");
        int32 EndIndex = CleanedJson.Find("```", ESearchCase::IgnoreCase, ESearchDir::FromEnd);

        if (StartIndex != INDEX_NONE && EndIndex != INDEX_NONE && EndIndex > StartIndex)
        {
            CleanedJson = CleanedJson.Mid(StartIndex, EndIndex - StartIndex);
        }
    }


    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(CleanedJson);
    if (FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        JsonObject->TryGetStringField(TEXT("action"), OutCommand.Action);
        JsonObject->TryGetStringField(TEXT("target"), OutCommand.Target);
        JsonObject->TryGetStringField(TEXT("direction"), OutCommand.Direction);
        JsonObject->TryGetStringField(TEXT("question"), OutCommand.Question);

        return true;
    }

    UE_LOG(LogTemp, Error, TEXT("❌ 无法解析 GPT 回复中的 JSON 格式：\n%s"), *GPTReply);
    return false;
}

void UGPTManager::LoadPromptConfig()
{
    FString ConfigPath = FPaths::ProjectContentDir() + TEXT("Config/GPTConfig.json");
    FString JsonRaw;
    UE_LOG(LogTemp, Warning, TEXT("Full Config Path: %s"), *ConfigPath);
    if (FFileHelper::LoadFileToString(JsonRaw, *ConfigPath))
    {
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonRaw);

        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            LoadedSystemPrompt = JsonObject->GetStringField("system_prompt");
            UE_LOG(LogTemp, Log, TEXT("System Prompt Loaded: %s"), *LoadedSystemPrompt);
        }
        else 
        {
            UE_LOG(LogTemp, Log, TEXT("System Prompt NOT Loaded"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("加载 GPTConfig.json 失败"));
    }
}

void UGPTManager::LoadPromptFromTxt()
{
    FString FilePath = FPaths::ProjectContentDir() + TEXT("Config/PromptTemplate.txt");

    FString TxtContent;

    if (FFileHelper::LoadFileToString(TxtContent, *FilePath))
    {
        LoadedSystemPrompt = TxtContent;
        UE_LOG(LogTemp, Log, TEXT("成功加载 PromptTemplate.txt"));
        UE_LOG(LogTemp, Log, TEXT("Prompt 内容:\n%s"), *LoadedSystemPrompt);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("加载 PromptTemplate.txt 失败！路径：%s"), *FilePath);
    }
}

void UGPTManager::LogConversationToFile(const FString& Role, const FString& Message)
{
    FString LogDir = FPaths::ProjectLogDir();  // 日志目录（通常是 Saved/Logs）
    FString LogFilePath = LogDir / TEXT("GPTConversationLog.txt");

    FString TimeStamp = FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S"));
    FString FullMessage = FString::Printf(TEXT("[%s][%s]: %s\n"), *TimeStamp, *Role, *Message);

    FFileHelper::SaveStringToFile(
        FullMessage,
        *LogFilePath,
        FFileHelper::EEncodingOptions::AutoDetect,
        &IFileManager::Get(),
        FILEWRITE_Append
    );
}



//UGPTManager* UGPTManager::GetGPTManager(UObject* WorldContext)
//{
//    static UGPTManager* GPTInstance = nullptr;
//
//    if (!GPTInstance)
//    {
//        GPTInstance = NewObject<UGPTManager>(WorldContext);
//        GPTInstance->AddToRoot(); // 防止被GC清除
//    }
//
//    return GPTInstance;
//}
