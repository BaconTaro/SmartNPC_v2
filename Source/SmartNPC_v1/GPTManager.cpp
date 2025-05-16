#include "GPTManager.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "SmartNPCCharacter.h"
#include "InteractableActor.h"
#include "EngineUtils.h"
#include <Kismet/GameplayStatics.h>

UGPTManager::UGPTManager()
{
    // 构造函数逻辑（可以为空）
}

void UGPTManager::SendMessageWithContext(const FString& PersonaPrompt, const TArray<FAIMessage>& History, const FString& Message)
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
        FString CombinedSystemPrompt = LoadedSystemPrompt + TEXT("") + SystemContextFormatted;

        TSharedPtr<FJsonObject> SystemPrompt = MakeShareable(new FJsonObject());
        SystemPrompt->SetStringField(TEXT("role"), TEXT("system"));
        SystemPrompt->SetStringField(TEXT("content"), CombinedSystemPrompt);
        Messages.Add(MakeShareable(new FJsonValueObject(SystemPrompt)));

        LogConversationToFile(TEXT("System"), CombinedSystemPrompt);
    }
    
    // 倒序添加对话历史
    for (int32 i = History.Num() - 1; i >= 0; --i)
    {
        const FAIMessage& Msg = History[i];

        TSharedPtr<FJsonObject> HistoryMessage = MakeShareable(new FJsonObject());
        HistoryMessage->SetStringField(TEXT("role"), Msg.Role);
        HistoryMessage->SetStringField(TEXT("content"), Msg.Content);

        Messages.Add(MakeShareable(new FJsonValueObject(HistoryMessage)));

        // 转换为 FString 并记录日志（记录最后一条）
        FString OutputString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
        FJsonSerializer::Serialize(Messages.Last()->AsObject().ToSharedRef(), Writer); // 可根据需要记录某条

        LogConversationToFile(TEXT("History"), OutputString);
    }

    //// 添加当前玩家消息
    //TSharedPtr<FJsonObject> UserMessage = MakeShareable(new FJsonObject());
    //UserMessage->SetStringField(TEXT("role"), TEXT("user"));
    //UserMessage->SetStringField(TEXT("content"), Message);
    //Messages.Add(MakeShareable(new FJsonValueObject(UserMessage)));
    //LogConversationToFile(TEXT("User"), Message);

    RootObject->SetArrayField(TEXT("messages"), Messages);
    // 序列化为 JSON 字符串
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

    // 创建 HTTP 请求
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(TEXT("https://api.siliconflow.cn/v1/chat/completions"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("Authorization"), TEXT("Bearer sk-mmzjqbguiaqoupbunjgnrilaczmwsmsjzltbgtfitsaykdto")); // 👈 记得替换你的 Key
    Request->SetContentAsString(RequestBody);

    Request->OnProcessRequestComplete().BindUObject(this, &UGPTManager::OnResponseReceived);
    Request->ProcessRequest();

    LogConversationToFile(TEXT("FullRequest"), RequestBody);
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
    LogConversationToFile(TEXT("ResponseContent: "), ResponseContent);
    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("JSON 解析失败！"));
        LogConversationToFile(TEXT("Game System: "), ResponseContent);
        return;
    }

    const TArray<TSharedPtr<FJsonValue>>* Choices;
    if (!JsonObject->TryGetArrayField(TEXT("choices"), Choices) || !Choices || Choices->Num() == 0)
    {
        FString ChoicesStr;

        if (Choices)
        {
            // 打印内容：可视化空数组内容，而不是强行序列化
            ChoicesStr = FString::Printf(TEXT("Choices is valid but empty. Num = %d"), Choices->Num());
        }
        else
        {
            ChoicesStr = TEXT("Choices 指针为 nullptr");
        }

        UE_LOG(LogTemp, Error, TEXT("返回的 choices 无效，内容：%s"), *ChoicesStr);
        return;
    }

    TSharedPtr<FJsonObject> FirstChoice = (*Choices)[0]->AsObject();
    TSharedPtr<FJsonObject> MessageObject = FirstChoice->GetObjectField(TEXT("message"));
    FString GPTReply = MessageObject->GetStringField(TEXT("content"));

    UE_LOG(LogTemp, Warning, TEXT("GPT 回复：%s"), *GPTReply);



    LogConversationToFile(TEXT("GPT"), GPTReply);

    // 现在parse一下GPTReply，然后广播给蓝图。
    FParsedCommand Parsed;
    if (ParseGPTReply(GPTReply, Parsed))
    {
        // ✅ 成功解析，可以传给 NPC 控制逻辑
        UE_LOG(LogTemp, Log, TEXT("Action: %s, Target: %s, Direction: %s, Question: %s"),
            *Parsed.Action, *Parsed.Target, *Parsed.Speak, *Parsed.Mood);
        // 👇 向蓝图广播回复，这个GPTReply 到此还没被parse，先留着，看看全貌，这个东西会整个被打印到游戏对话框内，方便debug用。
        OnGPTReplyReceived.Broadcast(GPTReply, Parsed);
        // 示例：广播给蓝图 NPC 使用
        //OnParsedCommand.Broadcast(Parsed);
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
        JsonObject->TryGetStringField(TEXT("speak"), OutCommand.Speak);
        JsonObject->TryGetStringField(TEXT("mood"), OutCommand.Mood);

        //根据 action 和 target 执行交互
        if (OutCommand.Action == TEXT("interact"))
        {
            for (TActorIterator<AInteractableActor> It(GetWorld()); It; ++It)
            {
                AInteractableActor* TargetActor = *It;
                if (TargetActor && TargetActor->GetFName().ToString() == OutCommand.Target)
                {
                    OutCommand.Action = TEXT("speak");

                    LogConversationToFile(TEXT("GPT New Action："), OutCommand.Action);
                    // 检查是否已交互，若是，修改 Action 为 speak
                    if (TargetActor->IsActorInteracted())
                    {
                        UE_LOG(LogTemp, Warning, TEXT("已交互，修改 Action 为 speak"));
                        

                        // 也可以直接退出，避免再次交互：
                        return true;
                    }

                    FName nextStage = TargetActor->GetNextStage();

                    if (nextStage.IsNone() || nextStage.IsEqual(TEXT("NULL")) || nextStage.IsEqual(TEXT("")))
                    {
                        break; // 找到后停止
                    }
                    else 
                    {
                        TargetActor->SetActorInteracted(true); //先设置已经被交互了。

                        FString Result = TargetActor->Interact();
                        LogConversationToFile(TEXT("GAME SYSTEM"), Result);
                        UE_LOG(LogTemp, Log, TEXT("交互结果: %s"), *Result);

                        //可以这样将 Result 转换为一个结构化的 FAIMessage，并插入到 ChatHistory 中，确保它是 system 角色发出的消息：
                        FAIMessage SystemMessage;
                        SystemMessage.Role = TEXT("system");
                        SystemMessage.Content = Result;

                        for (auto Char : SystemMessage.Content)
                        {
                            UE_LOG(LogTemp, Warning, TEXT("Char code: %d"), Char);
                        }

                        for (TCHAR C : SystemMessage.Content)
                        {
                            UE_LOG(LogTemp, Warning, TEXT("Char code2: %d"), C);
                        }

                        ChatHistory.Insert(SystemMessage, 0); // 插入到开头

                        LoadPromptFromTxt(); // 重新更新环境信息

                        SendMessageWithContext(TEXT(""), ChatHistory, Result);
                        break; // 找到后停止
                    }

                    
                }
            }
        }


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

// 这个是从GlobalGameInstance.cpp 调用的
void UGPTManager::LoadPromptFromTxt()
{
    FString FilePath = FPaths::ProjectContentDir() + TEXT("Config/PromptTemplate.txt");

    FString TxtContent;

    if (FFileHelper::LoadFileToString(TxtContent, *FilePath))
    {
        LoadedSystemPrompt = TxtContent;
        LoadedSystemPrompt = LoadedSystemPrompt + GenerateEnvironmentPrompt();

        if (!LoadedSystemPrompt.EndsWith("\n")) {
            LoadedSystemPrompt.AppendChar('\n');
        }

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
    FString LogFilePath = LogDir / TEXT("GPTConversationLog.log");

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


// 给所有的可互动的actor都放到prompt内
// 以后可以做距离检测，目前先全放到里面
FString UGPTManager::GenerateEnvironmentPrompt()
{
    FString Result;
    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);

     //房间名（可选）
    RootObject->SetStringField("room", "Office");

    // 环境描述（可选）
    RootObject->SetStringField("description", "Your own office, temporarily safe.");

    // 获取场景中所有 InteractableActor
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AInteractableActor::StaticClass(), FoundActors);

    TArray<TSharedPtr<FJsonValue>> ObjectArray;
    for (AActor* Actor : FoundActors)
    {
        AInteractableActor* Interactable = Cast<AInteractableActor>(Actor);
        if (Interactable)
        {
            TSharedPtr<FJsonObject> Obj = MakeShareable(new FJsonObject);
            Obj->SetStringField("id", Interactable->GetFName().ToString());
            Obj->SetStringField("description", Interactable->GetObjectDescription());
            if (Interactable->IsActorInteracted()) 
            {
                Obj->SetStringField("interact", Interactable->GetCurrentStageDescription());
            }
            ObjectArray.Add(MakeShareable(new FJsonValueObject(Obj)));
        }
    }

    RootObject->SetArrayField("objects", ObjectArray);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

    return OutputString;
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
