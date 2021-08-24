// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#pragma once

#include "Kismet/KismetSystemLibrary.h"

#include "KismetSystemLibraryExtensions.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FConsoleVariableChangedFloatSignature, float, NewValue);
DECLARE_DYNAMIC_DELEGATE_OneParam(FConsoleVariableChangedIntSignature, int32, NewValue);
DECLARE_DYNAMIC_DELEGATE_OneParam(FConsoleVariableChangedBoolSignature, bool, bNewValue);

UCLASS(meta=(BlueprintThreadSafe))
class TPCE_API UKismetSystemLibraryEx : public UKismetSystemLibrary
{
	GENERATED_UCLASS_BODY()

public:

	/** Get the path to the settings file of the current project. */
	UFUNCTION(BlueprintPure, Category="Utilities|Paths", meta=(BlueprintThreadSafe))
	static FString GetProjectSettingsPath();

	/**
	 * Registers a float setting variable.
	 *
	 * @param	Section			Section in the config file where to find the setting.
	 * @param	VariableName	Name of the setting variable.
	 * @param	DefaultValue	Default value used if the setting could not be loaded from disk.
	 */
	UFUNCTION(BlueprintCallable, Category="Settings")
	static void RegisterSettingFloat(const FString& Section, const FString& VariableName, float DefaultValue);

	/**
	 * Registers an integer setting variable.
	 *
	 * @param	Section			Section in the config file where to find the setting.
	 * @param	VariableName	Name of the setting variable.
	 * @param	DefaultValue	Default value used if the setting could not be loaded from disk.
	 */
	UFUNCTION(BlueprintCallable, Category="Settings")
	static void RegisterSettingInt(const FString& Section, const FString& VariableName, int32 DefaultValue);

	/**
	 * Registers a boolean setting variable.
	 *
	 * @param	Section			Section in the config file where to find the setting.
	 * @param	VariableName	Name of the setting variable.
	 * @param	DefaultValue	Default value used if the setting could not be loaded from disk.
	 */
	UFUNCTION(BlueprintCallable, Category="Settings")
	static void RegisterSettingBool(const FString& Section, const FString& VariableName, bool bDefaultValue);

	/**
	 * Registers a float setting variable with a callback to be invoked when it changes.
	 *
	 * @param	Section			Section in the config file where to find the setting.
	 * @param	VariableName	Name of the setting variable.
	 * @param	DefaultValue	Default value used if the setting could not be loaded from disk.
	 * @param	ChangedEvent	Delegate called when the setting is changed.
	 * @param	bChangeNow		If True, fire the event immediately once registered.
	 */
	UFUNCTION(BlueprintCallable, Category="Settings")
	static void RegisterSettingFloatWithCallback(const FString& Section, const FString& VariableName, float DefaultValue, FConsoleVariableChangedFloatSignature ChangedEvent, bool bChangeNow);

	/**
	 * Registers an integer setting variable with a callback to be invoked when it changes.
	 *
	 * @param	Section			Section in the config file where to find the setting.
	 * @param	VariableName	Name of the setting variable.
	 * @param	DefaultValue	Default value used if the setting could not be loaded from disk.
	 * @param	ChangedEvent	Delegate called when the setting is changed.
	 * @param	bChangeNow		If True, fire the event immediately once registered.
	 */
	UFUNCTION(BlueprintCallable, Category="Settings")
	static void RegisterSettingIntWithCallback(const FString& Section, const FString& VariableName, int32 DefaultValue, FConsoleVariableChangedIntSignature ChangedEvent, bool bChangeNow);

	/**
	 * Registers a boolean setting variable with a callback to be invoked when it changes.
	 *
	 * @param	Section			Section in the config file where to find the setting.
	 * @param	VariableName	Name of the setting variable.
	 * @param	DefaultValue	Default value used if the setting could not be loaded from disk.
	 * @param	ChangedEvent	Delegate called when the setting is changed.
	 * @param	bChangeNow		If True, fire the event immediately once registered.
	 */
	UFUNCTION(BlueprintCallable, Category="Settings")
	static void RegisterSettingBoolWithCallback(const FString& Section, const FString& VariableName, bool bDefaultValue, FConsoleVariableChangedBoolSignature ChangedEvent, bool bChangeNow);

	/**
	 * Attempts to retrieve the value of the specified float setting variable, if it exists.
	 *
	 * @param	VariableName	Name of the setting variable to find.
	 * @return	The value if found, 0 otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category="Settings")
	static float GetSettingFloat(const FString& VariableName);

	/**
	 * Attempts to retrieve the value of the specified integer setting variable, if it exists.
	 *
	 * @param	VariableName	Name of the setting variable to find.
	 * @return	The value if found, 0 otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category="Settings")
	static int32 GetSettingInt(const FString& VariableName);

	/**
	 * Evaluates, if it exists, whether the specified integer setting variable has a non-zero value (true) or not (false).
	 *
	 * @param	VariableName	Name of the setting variable to find.
	 * @return	True if found and has a non-zero value, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category="Settings")
	static bool GetSettingBool(const FString& VariableName);

	/**
	 * Assigns a new value to the specified float setting variable.
	 *
	 * @param	VariableName	Name of the setting variable to find.
	 * @param	NewValue		Value to assign to the variable.
	 */
	UFUNCTION(BlueprintCallable, Category="Settings")
	static void SetSettingFloat(const FString& VariableName, float NewValue);

	/**
	 * Assigns a new value to the specified integer setting variable.
	 *
	 * @param	VariableName	Name of the setting variable to find.
	 * @param	NewValue		Value to assign to the variable.
	 */
	UFUNCTION(BlueprintCallable, Category="Settings")
	static void SetSettingInt(const FString& VariableName, int32 NewValue);

	/**
	 * Assigns a new value to the specified boolean setting variable.
	 *
	 * @param	VariableName	Name of the setting variable to find.
	 * @param	NewValue		Value to assign to the variable.
	 */
	UFUNCTION(BlueprintCallable, Category="Settings")
	static void SetSettingBool(const FString& VariableName, bool bNewValue);

	/** Saves any registered setting variables to the project settings file. @see GetProjectSettingsPath */
	UFUNCTION(BlueprintCallable, Category="Settings")
	static void SaveSettings();

private:

	static const FString& ProjectSettingsPath();

	template <typename T>
	static void Generic_SetSetting(const FString& VariableName, T NewValue);
};
