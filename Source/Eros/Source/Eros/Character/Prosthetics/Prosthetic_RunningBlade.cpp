#include "Eros.h"

#include "../ErosCharacter.h"
#include "Prosthetic_RunningBlade.h"

FErosCharacterMovement AProsthetic_RunningBlade::ApplyModifiers(FErosCharacterMovement CharacterMovement) const
{
	CharacterMovement.MaxWalkSpeed *= RunningSpeedModifier;

	return CharacterMovement;
}