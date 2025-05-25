// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

UINTERFACE(MinimalAPI)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

class XYZHOMEWORK_API IInteractable
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE(FOnInteraction)

	/** Executes the interaction logic of this object. */
	virtual void Interact(APawn* InteractingPawn) PURE_VIRTUAL(IInteractable::Interact,);
	/** Returns the name of an input action associated with this interactable object (action bound to the input component of a character). */
	virtual FName GetActionName() PURE_VIRTUAL(IInteractable::GetActionName, return FName(NAME_None););
	/** Is the Interact() function bound to a delegate. */
	virtual bool HasOnInteractionCallback() PURE_VIRTUAL(IInteractable::HasOnInteractionCallback, return false;);
	/** Binds the Interact() function to the 'FunctionName' delegate. */
	virtual FDelegateHandle AddOnInteractionDelegate(UObject* Object, FName FunctionName) PURE_VIRTUAL(IInteractable::AddOnInteractionDelegate, return FDelegateHandle(););
	/** Unbinds the Interact() function from a delegate. */
	virtual void RemoveOnInteractionDelegate(FDelegateHandle DelegateHandle) PURE_VIRTUAL(IInteractable::RemoveOnInteractionDelegate,);
};
