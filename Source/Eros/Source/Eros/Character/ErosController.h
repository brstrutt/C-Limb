#pragma once

#include "GameFramework/PlayerController.h"
#include "ErosController.generated.h"

class AErosCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInputChanged, bool, NewState);


UCLASS()
class EROS_API AErosController : public APlayerController
{
	GENERATED_BODY()

public:

	AErosController();

	UPROPERTY(BlueprintAssignable)
	FOnInputChanged OnInputChanged;

	virtual void SetupInputComponent() override;

	virtual void Possess(APawn* aPawn) override;

	UFUNCTION(BlueprintCallable, Category = Tutorial)
	FORCEINLINE bool GetShowProstheticTutorials() const { return bShowProstheticTutorials; }
	
	UFUNCTION(BlueprintCallable, Category = Input)
	FORCEINLINE bool GetInputEnabled() const { return bInputEnabled; }

	UFUNCTION(BlueprintCallable, Category = Input)
	void SetInputEnabled(bool enable);

	UFUNCTION(BlueprintCallable, Category = Reset)
	void NewGame();

	void SetShowProstheticTutorials(bool show);

private:

	/* Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(EditDefaultsOnly, Category = Camera)
	float BaseTurnRate;

	/* Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(EditDefaultsOnly, Category = Camera)
	float BaseLookUpRate;

	bool bShowProstheticTutorials;

	bool bInputEnabled;

	/* The character being controlled. */
	AErosCharacter* Character;

	/* Functions to bind to input. */
	void Jump();
	void StopJump();

	void Crouch();

	void SwapLegProsthetic();
	void SwapArmProsthetic();

	void ProstheticArmPrimaryActionBegin();
	void ProstheticArmPrimaryActionEnd();

	void ProstheticArmSecondaryActionBegin();
	void ProstheticArmSecondaryActionEnd();

	void Interact();

	void Respawn();

	void MoveForward(float Value);
	void MoveRight(float Value);

	void TurnAtRate(float Value);
	void LookUpAtRate(float Value);
};
