// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "Components/ExtInputComponent.h"

FInputActionBinding& UExtInputComponent::AddActionBindingWithDescription(FInputActionBinding Binding, const FText& Text)
{
	// Adding support for input binding descriptions from the engine side would be easy, just add a field to FInputBinding
	// None of UInputComponent's functions are marked virtual, so it's not possible to know when a binding is removed either
	// This workaround only applies to action bindings as they have an unique handle that can be associated with the user provided text

	FInputActionBinding& NewBinding = Super::AddActionBinding(MoveTemp(Binding));
	ActionBindingDescriptions.Add(NewBinding.GetHandle(), Text);

	return NewBinding;
}

bool UExtInputComponent::GetActionBindingDescription(const int32 BindingIndex, FText& Text) const
{
	FInputActionBinding& Binding = GetActionBinding(BindingIndex);

	if (const FText* TextP = ActionBindingDescriptions.Find(Binding.GetHandle()))
	{
		Text = *TextP;
		return true;
	}

	return false;
}
