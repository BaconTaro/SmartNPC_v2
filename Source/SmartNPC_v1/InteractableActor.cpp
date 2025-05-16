// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableActor.h"

// Sets default values
AInteractableActor::AInteractableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(TEXT("InteractableComponent"));
	AddOwnedComponent(InteractableComponent); // ��ѡ�����Ϊӵ��

}

FName AInteractableActor::GetNextStage()
{
	UInteractableComponent* InteractableComp = FindComponentByClass<UInteractableComponent>();
	if (InteractableComp)
	{
		return InteractableComp->GetNextStage();
	}
	return FName();
	
}

bool AInteractableActor::IsActorInteracted()
{
	return Interacted;
}

void AInteractableActor::SetActorInteracted(bool result)
{
	Interacted = result;
}

// Called when the game starts or when spawned
void AInteractableActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AInteractableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FString AInteractableActor::GetObjectDescription()
{
	return ObjectDescription;
}

FString AInteractableActor::GetCurrentStageDescription()
{
	UInteractableComponent* InteractableComp = FindComponentByClass<UInteractableComponent>();
	if (InteractableComp)
	{
		return InteractableComp->GetCurrentStageDescription();
	}
	return TEXT("�޽������");
}

FString AInteractableActor::Interact_Implementation()
{
	if (UInteractableComponent* Comp = FindComponentByClass<UInteractableComponent>())
	{
		return Comp->Interact();
	}
	return TEXT("�������޷�������");
}

