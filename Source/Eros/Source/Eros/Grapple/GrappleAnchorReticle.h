#pragma once

#include "GameFramework/Actor.h"
#include "GrappleAnchorReticle.generated.h"

class UGrappleAnchor;
class UWidgetComponent;

UCLASS()
class EROS_API AGrappleAnchorReticle : public AActor
{
	GENERATED_BODY()
	
public:	

	AGrappleAnchorReticle();

	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaSeconds) override;

	virtual void SetTarget(UGrappleAnchor const* Anchor);

	virtual void Hide();

protected:

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "FadeChange"))
	void UpdateFadeValue(float FadeValue);

private:

	/* Time to fade in or out. */
	UPROPERTY(EditDefaultsOnly, Category = Reticle)
	float FadeTime;

	/* Time to move to the next anchor on the SAME target. */
	UPROPERTY(EditDefaultsOnly, Category = Reticle)
	float MoveTime;

	/* UI reticle. */
	UPROPERTY(EditDefaultsOnly, Category = Reticle)
	UWidgetComponent* Reticle;

	UGrappleAnchor const* Target;

	bool bFadeIn;
	bool bFadeOut;

	float ReticleFadeValue;
};
