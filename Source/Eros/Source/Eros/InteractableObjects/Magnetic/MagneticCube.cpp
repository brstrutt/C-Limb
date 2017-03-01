#include "Eros.h"

#include "../../Character/Prosthetics/Prosthetic_MagnetArm.h"
#include "MagneticCube.h"

AMagneticCube::AMagneticCube()
{	
	RootComponent = Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	Mesh->SetSimulatePhysics(false);

	Collider = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerBlock"));
	Collider->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
}

bool AMagneticCube::Interact(AProsthetic& Prosthetic)
{
	Mesh->SetSimulatePhysics(true);

	if (const AProsthetic_MagnetArm* MagneticArm = Cast<AProsthetic_MagnetArm>(&Prosthetic))
	{
		FVector TargetDirection = MagneticArm->GetCurrentMagneticRayStart() - MagneticArm->GetCurrentMagneticRayEnd();
		TargetDirection.Normalize();

		//Reverses the pull direction based off mouse click
		if (MagneticArm->GetProstheticState() == EActionState::AS_Primary)
		{
			TargetDirection *= -1;
		}

		//Locks movement to either just X or Y movement
		//if (TargetDirection.GetAbs().X > TargetDirection.GetAbs().Y)
		//{
		//	TargetDirection *= FVector(1.0f, 0.0f, 0.0f);
		//}
		//else
		//{
		//	TargetDirection *= FVector(0.0f, 1.0f, 0.0f);
		//}

		Mesh->AddForce(TargetDirection * MagneticArm->GetMagnetStrength());
		
		//Mesh->SetWorldLocation( - (TargetDirection * MagneticArm->GetMagnetStrength()), true);
		//SetActorLocation(GetLocation() + (TargetDirection * MagneticArm->GetMagnetStrength()));

		//Adds the force to the targeted object
		//if (MagneticArm->GetCurrentMagneticRayStart().Z < MagneticArm->GetCurrentMagneticRayEnd().Z)
		//{
		//	Mesh->AddForce(TargetDirection * MagneticArm->GetMagnetStrength());
		//}

		UE_LOG(LogTemp, Warning, TEXT("%s"), *TargetDirection.ToCompactString());
		UE_LOG(LogTemp, Warning, TEXT("%s"), *GetName());
		
		return true;
	}

	return false;
}

bool AMagneticCube::StopInteract()
{
	Mesh->SetSimulatePhysics(false);

	return true;
}