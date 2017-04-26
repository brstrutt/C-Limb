// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/AnimInstance.h"
#include "ErosAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class EROS_API UErosAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "SwapLeg")
	void EAI_PlayAnimMontageEvent();
	
	
};
