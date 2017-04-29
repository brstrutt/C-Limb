#include "Eros.h"

#include "ConveyorBelt.h"
#include "ConveyorBeltSection.h"

AConveyorBelt::AConveyorBelt()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Start = CreateDefaultSubobject<USceneComponent>(TEXT("Start"));
	Start->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	End = CreateDefaultSubobject<USceneComponent>(TEXT("End"));
	End->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	Collider = CreateDefaultSubobject<UBoxComponent>(TEXT("EndDetection"));
	Collider->AttachToComponent(End, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	DefaultActive = true;
}

void AConveyorBelt::BeginPlay()
{
	Super::BeginPlay();

	FVector Distance = End->GetComponentLocation() - Start->GetComponentLocation();
	Direction = Distance.GetUnsafeNormal();

	// Spawn the initial conveyor belt pieces.
	
	// Don't spawn the last piece as it won't call on overlap begin.
	int PiecesToSpawn = (Distance.Size() / PieceLength) - 1;

	for (FVector Location = Start->GetComponentLocation() + (Direction * (PieceLength / 2.0f)); PiecesToSpawn > 0; Location += (Direction * PieceLength), PiecesToSpawn--)
	{
		AConveyorBeltSection* Section = GetWorld()->SpawnActor<AConveyorBeltSection>(SectionPiece.GetDefaultObject()->GetClass());
		Section->SetActorLocation(Location);
		Section->SetActorRotation(Direction.Rotation());

		ConveyorBelt.Add(Section);
	}

	// We want to know when pieces reach the end.

	FScriptDelegate ReachedEndDelegate;
	ReachedEndDelegate.BindUFunction(this, FName("OnOverlapBegin"));
	Collider->OnComponentBeginOverlap.Add(ReachedEndDelegate);

	SetActorTickEnabled(DefaultActive);
}

void AConveyorBelt::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdatePieces(DeltaSeconds);
}

void AConveyorBelt::ActivateConveyor(bool Active)
{
	SetActorTickEnabled(Active);
}

void AConveyorBelt::BeginDestroy()
{
	Super::BeginDestroy();

	for (int i = 0; i < ConveyorBelt.Num(); ++i)
	{
		ConveyorBelt[i]->Destroy();
	}
}

void AConveyorBelt::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AConveyorBeltSection* Section = Cast<AConveyorBeltSection>(OtherActor))
	{
		MoveToBeginning(*Section);
	}
}

void AConveyorBelt::UpdatePieces(float DeltaSeconds)
{
	FVector MovementDelta = Direction * Speed * DeltaSeconds;

	for (AConveyorBeltSection* Section : ConveyorBelt)
	{
		Section->SetActorLocation(Section->GetActorLocation() + MovementDelta);
	}
}

void AConveyorBelt::MoveToBeginning(AConveyorBeltSection& Section)
{
	float ShortestDistance = FLT_MAX;
	AConveyorBeltSection* Closest = nullptr;

	for (int Index = 0; Index < ConveyorBelt.Num(); Index++)
	{
		if (ConveyorBelt[Index] != &Section)
		{
			float Distance = (Start->GetComponentLocation() - ConveyorBelt[Index]->GetActorLocation()).Size();

			if (Distance < ShortestDistance)
			{
				ShortestDistance = Distance;
				Closest = ConveyorBelt[Index];
			}
		}
	}

	if (Closest != nullptr)
	{
		Section.SetActorLocation(Closest->GetActorLocation() - (Direction * (float)PieceLength));

		float Distance = (Section.GetActorLocation() - Closest->GetActorLocation()).Size();
	}
}