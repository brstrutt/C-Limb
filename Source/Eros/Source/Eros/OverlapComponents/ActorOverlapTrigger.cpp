#include "Eros.h"
#include "ActorOverlapTrigger.h"

UActorOverlapTrigger::UActorOverlapTrigger()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UActorOverlapTrigger::BeginPlay()
{
	Super::BeginPlay();

	FScriptDelegate DelBegin;
	DelBegin.BindUFunction(this, FName("OnOverlapBegin"));
	GetOwner()->OnActorBeginOverlap.AddUnique(DelBegin);

	FScriptDelegate DelEnd;
	DelEnd.BindUFunction(this, FName("OnOverlapEnd"));
	GetOwner()->OnActorEndOverlap.AddUnique(DelEnd);
}

void UActorOverlapTrigger::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	OverlapBegin(OverlappedActor, OtherActor);
}

void UActorOverlapTrigger::OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor)
{
	OverlapEnd(OverlappedActor, OtherActor);
}