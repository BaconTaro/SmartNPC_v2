// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SmartNPCGamePlayerController.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceTextRecived, const FText&, intext);
UCLASS()
class SMARTNPC_V1_API ASmartNPCGamePlayerController : public APlayerController
{
	GENERATED_BODY()
public:
    virtual void SetupInputComponent() override;
	FTimerHandle VoiceCheckTimer;
	FString LastReadText;

	virtual void BeginPlay() override;
	UPROPERTY(BlueprintAssignable)
    FOnVoiceTextRecived OnVoiceTextRecived;

	UFUNCTION(BlueprintCallable)
	void ToggleVoiceRecognition();           // 绑定 G 键
private:
    FProcHandle PythonProcHandle;
    bool bIsVoiceRunning = false;
    FString RecognizedText;

    
    void StartPythonScript();                // 启动语音识别
    void StopPythonScript();                 // 停止语音识别
    bool LoadRecognitionResult();            // 读取 result.txt 并存入变量	
	void CheckVoiceResult();				 // 动态自动读取
};
