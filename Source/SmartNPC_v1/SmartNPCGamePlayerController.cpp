// Fill out your copyright notice in the Description page of Project Settings.


#include "SmartNPCGamePlayerController.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"
#include "Misc/FileHelper.h"

FTimerHandle VoiceCheckTimer;
FString LastReadText;

void ASmartNPCGamePlayerController::BeginPlay()
{
    Super::BeginPlay();

        // 设置定时器
    GetWorld()->GetTimerManager().SetTimer(
        VoiceCheckTimer,
        this,
        &ASmartNPCGamePlayerController::CheckVoiceResult,
        0.5f,
        true
    );
}

void ASmartNPCGamePlayerController::CheckVoiceResult()
{
    FString ResultPath = FPaths::ProjectDir() + TEXT("Script/result.txt");
    FString NewText;

    if (FFileHelper::LoadFileToString(NewText, *ResultPath))
    {
        if (NewText != LastReadText)
        {
            FString DeltaText = NewText.RightChop(LastReadText.Len());
            LastReadText = NewText;

            UE_LOG(LogTemp, Warning, TEXT("新识别文字：%s"), *DeltaText);
            OnVoiceTextRecived.Broadcast(FText::FromString(DeltaText));
        }
    }
}

void ASmartNPCGamePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    InputComponent->BindKey(EKeys::G, IE_Pressed, this, &ASmartNPCGamePlayerController::ToggleVoiceRecognition);
}

void ASmartNPCGamePlayerController::ToggleVoiceRecognition()
{
    if (!bIsVoiceRunning)
    {
        StartPythonScript();
    }
    else
    {
        bool loadTextcomplete = LoadRecognitionResult();
        if (loadTextcomplete) {
            // GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("进入了 if 分支，识别文本已加载"));
            // OnVoiceTextRecived.Broadcast(FText::FromString(RecognizedText));
        } 
        else 
        {
            
        }
        StopPythonScript();
    }

    bIsVoiceRunning = !bIsVoiceRunning;
}

void ASmartNPCGamePlayerController::StartPythonScript()
{
    FString ProjectDir = FPaths::ProjectDir();
    FString PythonExe = TEXT("pyw"); // 要求系统 PATH 中可调用 python
    FString ScriptPath = FPaths::ConvertRelativePathToFull(ProjectDir + TEXT("Script/Voice2Text.py"));
    FString ResultPath = FPaths::ConvertRelativePathToFull(ProjectDir + TEXT("Script/result.txt"));
    FString StopPath = FPaths::ConvertRelativePathToFull(ProjectDir + TEXT("Script/stop.txt"));
    // 清除 stop.txt 
    if (FPaths::FileExists(StopPath))
    {
        // 方法1：清空文件内容
        // FFileHelper::SaveStringToFile(TEXT(""), *ResultPath);
        // 或者方法2：删除旧文件（可选）
        IFileManager::Get().Delete(*StopPath);
    }

    // 清除 result.txt 内容
    if (FPaths::FileExists(ResultPath))
    {
        // 方法1：清空文件内容
        FFileHelper::SaveStringToFile(TEXT(""), *ResultPath);
        // 或者方法2：删除旧文件（可选）
        // IFileManager::Get().Delete(*ResultPath);
    }
    else
    {
        // 如果文件不存在，可以创建一个空文件（可选）
        FFileHelper::SaveStringToFile(TEXT(""), *ResultPath);
    }



    // 生成命令行
    FString CommandLine = FString::Printf(TEXT("\"%s\""), *ScriptPath);
    UE_LOG(LogTemp, Warning, TEXT("Launching Python: %s %s"), *PythonExe, *CommandLine);

    PythonProcHandle = FPlatformProcess::CreateProc(
        *PythonExe,
        *CommandLine,
        true, true, true,
        nullptr, 0,
        nullptr, nullptr
    );
}

void ASmartNPCGamePlayerController::StopPythonScript()
{
    FString StopFile = FPaths::ProjectDir() + TEXT("Script/stop.txt");
    FFileHelper::SaveStringToFile(TEXT("stop"), *StopFile);
    UE_LOG(LogTemp, Warning, TEXT("stop.txt written"));

    // 可选：强制关闭进程
    // if (PythonProcHandle.IsValid())
    // {
    //     FPlatformProcess::TerminateProc(PythonProcHandle, true);
    //     FPlatformProcess::CloseProc(PythonProcHandle);
    //     PythonProcHandle.Reset();
    // }

    if (PythonProcHandle.IsValid())
    {
        FPlatformProcess::TerminateProc(PythonProcHandle, true); // 强杀
        FPlatformProcess::CloseProc(PythonProcHandle);
        PythonProcHandle.Reset();
    }
}

bool ASmartNPCGamePlayerController::LoadRecognitionResult()
{
    FString ResultPath = FPaths::ProjectDir() + TEXT("Script/result.txt");
    FString FileContent;

    if (FPaths::FileExists(ResultPath) && FFileHelper::LoadFileToString(FileContent, *ResultPath))
    {
        RecognizedText = FileContent;
        UE_LOG(LogTemp, Warning, TEXT("识别内容: %s"), *RecognizedText);
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("未找到 result.txt 或读取失败"));
        return false;
    }
}