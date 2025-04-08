// GlobalGameInstance.cpp

#include "GlobalGameInstance.h"

void UGlobalGameInstance::Init()
{
    Super::Init();


}

// cpp 文件
UGPTManager* UGlobalGameInstance::GetGPTManager()
{
    if (!GPTManagerInstance)
    {
        GPTManagerInstance = NewObject<UGPTManager>(this);  // 不再 AddToRoot
    }
    GPTManagerInstance->LoadPromptFromTxt();


    return GPTManagerInstance;
}