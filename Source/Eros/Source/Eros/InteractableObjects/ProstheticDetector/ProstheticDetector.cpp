#include "Eros.h"

#include "../../Character/ErosCharacter.h"
#include "../../Character/Prosthetics/Prosthetic.h"
#include "ProstheticContainer.h"
#include "ProstheticDetector.h"

AProstheticDetector::AProstheticDetector()
{
	PrimaryActorTick.bCanEverTick = false;
	bOpen = true;

	RootComponent = DetectorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DetectorMesh"));

	DetectorCollider = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DetectorCollider"));
	DetectorCollider->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	TemporaryDoor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TemporaryDoor"));
	TemporaryDoor->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	// Delegate for moving into prosthetic pickup range.
	FScriptDelegate ProstheticInRangeDel;
	ProstheticInRangeDel.BindUFunction(this, FName("OnOverlapBegin"));
	DetectorCollider->OnComponentBeginOverlap.Add(ProstheticInRangeDel);

	// Delegate for moving out of prosthetic pickup range.
	FScriptDelegate ProstheticOutOfRangeDel;
	ProstheticOutOfRangeDel.BindUFunction(this, FName("OnOverlapEnd"));
	DetectorCollider->OnComponentEndOverlap.Add(ProstheticOutOfRangeDel);
}

void AProstheticDetector::BeginPlay()
{
	Super::BeginPlay();

	TemporaryDoor->SetHiddenInGame(true, true);
	TemporaryDoor->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
}

void AProstheticDetector::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AErosCharacter* Character = Cast<AErosCharacter>(OtherActor);

	if (Character != nullptr && Character->HasProstheticAttached(Container->GetAcceptedProsthetic()))
	{
		Close();
	}
}

void AProstheticDetector::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<AErosCharacter>(OtherActor))
	{
		Open();
	}
}

void AProstheticDetector::Open()
{
	if (bOpen) { return; }
	
	UE_LOG(LogTemp, Warning, TEXT("Open!"));

	TemporaryDoor->SetHiddenInGame(true, true);
	TemporaryDoor->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	bOpen = true;
}

void AProstheticDetector::Close()
{
	if (!bOpen) { return; }
	
	UE_LOG(LogTemp, Warning, TEXT("Closed!"));

	TemporaryDoor->SetHiddenInGame(false, true);
	TemporaryDoor->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);

	bOpen = false;
}