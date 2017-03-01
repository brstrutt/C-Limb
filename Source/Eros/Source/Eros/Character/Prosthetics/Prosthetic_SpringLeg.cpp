#include "Eros.h"

#include "../ErosCharacter.h"
#include "Prosthetic_SpringLeg.h"

FErosCharacterMovement AProsthetic_SpringLeg::ApplyModifiers(FErosCharacterMovement CharacterMovement) const
{
	CharacterMovement.JumpZVelocity *= JumpForceModifier;
	CharacterMovement.MaxWalkSpeed *= RunSpeedModifier;

	return CharacterMovement;
}