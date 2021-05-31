// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "CoreMinimal.h"
#include "Components/InputComponent.h"

#include "ExtInputComponent.generated.h"

/**
 * Input component with extended functionality.
 */
UCLASS()
class TPCE_API UExtInputComponent : public UInputComponent
{
	GENERATED_BODY()

public:

	/**
	 * Binds a delegate function to an Action defined in the project settings.
	 * Returned reference is only guaranteed to be valid until another action is bound.
	 */
	template<class UserClass>
	FInputActionBinding& BindAction(const FName ActionName, const EInputEvent KeyEvent, UserClass* Object, typename FInputActionHandlerSignature::TUObjectMethodDelegate< UserClass >::FMethodPtr Func, const FText& Description)
	{
		FInputActionBinding AB(ActionName, KeyEvent);
		AB.ActionDelegate.BindDelegate(Object, Func);
		return AddActionBindingWithDescription(MoveTemp(AB), Description);
	}

	/**
	 * Binds a delegate function to an Action defined in the project settings.
	 * Returned reference is only guaranteed to be valid until another action is bound.
	 */
	template<class UserClass>
	FInputActionBinding& BindAction(const FName ActionName, const EInputEvent KeyEvent, UserClass* Object, typename FInputActionHandlerWithKeySignature::TUObjectMethodDelegate< UserClass >::FMethodPtr Func, const FText& Description)
	{
		FInputActionBinding AB(ActionName, KeyEvent);
		AB.ActionDelegate.BindDelegate(Object, Func);
		return AddActionBindingWithDescription(MoveTemp(AB), Description);
	}

	/**
	* Binds a delegate function to an Action defined in the project settings.
	* Returned reference is only guaranteed to be valid until another action is bound.
	*/
	template< class DelegateType, class UserClass, typename... VarTypes >
	FInputActionBinding& BindAction(const FName ActionName, const EInputEvent KeyEvent, UserClass* Object, typename DelegateType::template TUObjectMethodDelegate< UserClass >::FMethodPtr Func, VarTypes... Vars, const FText& Description)
	{
		FInputActionBinding AB(ActionName, KeyEvent);
		AB.ActionDelegate.BindDelegate<DelegateType>(Object, Func, Vars...);
		return AddActionBindingWithDescription(MoveTemp(AB), Description);
	}

	/**
	 * Gets the description attached with the action binding with the specified index, if available.
	 *
	 * @param Handle The handle of the binding.
	 * @return True if the binding had a description attached to it.
	 */
	bool GetActionBindingDescriptionForHandle(const int32 Handle, FText& Text) const;

	/**
	 * Sets the description attached with the action binding with the specified index.
	 *
	 * @param Handle The handle of the binding.
	 * @param Text Description of the action binding.
	 */
	void SetActionBindingDescriptionForHandle(const int32 Handle, const FText& Text);

	/**
	 * Adds the specified action binding.
	 *
	 * @param Binding The binding to add.
	 * @return The last binding in the list.
	 * @see ClearActionBindings, GetActionBinding, GetNumActionBindings, RemoveActionBinding
	 */
	FInputActionBinding& AddActionBindingWithDescription(FInputActionBinding Binding, const FText& Text);

private:

	/** Map of handles to descriptions. */
	TMap<int32, FText> ActionBindingDescriptions;
};
