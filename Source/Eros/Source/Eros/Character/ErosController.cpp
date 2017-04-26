#include "Eros.h"

#include "ErosCharacter.h"
#include "ErosController.h"

AErosController::AErosController()
{
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	bShowProstheticTutorials = false;
}

void AErosController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("Jump", IE_Pressed, this, &AErosController::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &AErosController::StopJump);
	InputComponent->BindAxis("MoveForward", this, &AErosController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AErosController::MoveRight);
	InputComponent->BindAction("Crouch", IE_Pressed, this, &AErosController::Crouch);

	// MOUSE camera rotation
	InputComponent->BindAxis("Turn", this, &APlayerController::AddYawInput);
	InputComponent->BindAxis("LookUp", this, &APlayerController::AddPitchInput);

	// CONTROLLER camera rotation
	InputComponent->BindAxis("TurnRate", this, &AErosController::TurnAtRate);
	InputComponent->BindAxis("LookUpRate", this, &AErosController::LookUpAtRate);

	// Controls for picking up/dropping prosthetics.
	InputComponent->BindAction("SwapLegProsthetic", IE_Pressed, this, &AErosController::SwapLegProsthetic);
	InputComponent->BindAction("SwapArmProsthetic", IE_Pressed, this, &AErosController::SwapArmProsthetic);

	// Controls for Left and right click.
	InputComponent->BindAction("PrimaryAction", IE_Pressed, this, &AErosController::ProstheticArmPrimaryActionBegin);
	InputComponent->BindAction("SecondaryAction", IE_Pressed, this, &AErosController::ProstheticArmSecondaryActionBegin);
	InputComponent->BindAction("PrimaryAction", IE_Released, this, &AErosController::ProstheticArmPrimaryActionEnd);
	InputComponent->BindAction("SecondaryAction", IE_Released, this, &AErosController::ProstheticArmSecondaryActionEnd);

	// Interacting with interactable objects.
	InputComponent->BindAction("Interact", IE_Pressed, this, &AErosController::Interact);

	InputComponent->BindAction("Respawn", IE_Pressed, this, &AErosController::Respawn);
}

void AErosController::SetInputEnabled(bool enable)
{ 
	bInputEnabled = enable;

	OnInputChanged.Broadcast(enable);
}

void AErosController::NewGame()
{
	if (Character)
	{
		Character->ResetForNewGame();
	}
}

void AErosController::Possess(APawn* aPawn)
{
	Super::Possess(aPawn);

	Character = Cast<AErosCharacter>(aPawn);
	bInputEnabled = true;
}

void AErosController::SetShowProstheticTutorials(bool show)
{
	bShowProstheticTutorials = show;
}

void AErosController::Jump()
{
	if (bInputEnabled && Character)
	{
		Character->Jump();
	}
}

void AErosController::StopJump()
{
	if (bInputEnabled && Character)
	{
		Character->StopJumping();
	}
}

void AErosController::Crouch()
{
	if (bInputEnabled && Character)
	{
		Character->Crouch();
	}
}

void AErosController::SwapLegProsthetic()
{
	if (bInputEnabled && Character)
	{
		Character->SwapLegProsthetic();
	}
}

void AErosController::SwapArmProsthetic()
{
	if (bInputEnabled && Character)
	{
		Character->SwapArmProsthetic();
	}
}

void AErosController::ProstheticArmPrimaryActionBegin()
{
	if (bInputEnabled && Character)
	{
		Character->PrimaryActionBegin();
	}
}

void AErosController::ProstheticArmPrimaryActionEnd()
{
	if (bInputEnabled && Character)
	{
		Character->PrimaryActionEnd();
	}
}

void AErosController::ProstheticArmSecondaryActionBegin()
{
	if (bInputEnabled && Character)
	{
		Character->SecondaryActionBegin();
	}
}

void AErosController::ProstheticArmSecondaryActionEnd()
{
	if (bInputEnabled && Character)
	{
		Character->SecondaryActionEnd();
	}
}

void AErosController::Interact()
{
	if (bInputEnabled && Character)
	{
		Character->Interact();
	}
}

void AErosController::MoveForward(float Value)
{
	if (bInputEnabled && Character)
	{
		Character->MoveForward(Value);
	}
}

void AErosController::MoveRight(float Value)
{
	if (bInputEnabled && Character)
	{
		Character->MoveRight(Value);
	}
}

void AErosController::TurnAtRate(float Value)
{
	if (bInputEnabled && Character)
	{
		AddYawInput(Value * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
}

void AErosController::LookUpAtRate(float Value)
{
	if (bInputEnabled && Character)
	{
		AddPitchInput(Value * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
}

void AErosController::Respawn()
{
	if (bInputEnabled && Character)
	{
		//Character->KillPlayer();
	}
}