// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "Kismet/KismetSystemLibraryExtensions.h"

#include "HAL/IConsoleManager.h"

UKismetSystemLibraryEx::UKismetSystemLibraryEx(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FString UKismetSystemLibraryEx::GetProjectSettingsPath()
{
	return FPaths::ConvertRelativePathToFull(ProjectSettingsPath());
}

const FString& UKismetSystemLibraryEx::ProjectSettingsPath()
{
	static const FString Filename = FPaths::GeneratedConfigDir() + TEXT("Settings.ini");
	return Filename;
}

void EnsureConfigFileExists(const FString& Filename)
{
	if (!GConfig->FindConfigFile(Filename))
	{
		GConfig->LoadFile(Filename);
		if (!GConfig->FindConfigFile(Filename))
		{
			GConfig->Add(Filename, FConfigFile());
		}
	}
}

void UKismetSystemLibraryEx::RegisterSettingFloat(const FString& Section, const FString& VariableName, float DefaultValue)
{
	const FString& Filename = ProjectSettingsPath();

	// Get initial value from settings file, ensure it exists so it gets saved later
	EnsureConfigFileExists(Filename);
	if (!GConfig->GetFloat(*Section, *VariableName, DefaultValue, Filename))
	{
		GConfig->SetFloat(*Section, *VariableName, DefaultValue, Filename);
	}

	IConsoleManager::Get().UnregisterConsoleObject(*VariableName, /*bKeepState=*/false);
	IConsoleVariable* Variable = IConsoleManager::Get().RegisterConsoleVariable(*VariableName, DefaultValue, *Section);
}

void UKismetSystemLibraryEx::RegisterSettingInt(const FString& Section, const FString& VariableName, int32 DefaultValue)
{
	const FString& Filename = ProjectSettingsPath();

	// Get initial value from settings file, ensure it exists so it gets saved later
	EnsureConfigFileExists(Filename);
	if (!GConfig->GetInt(*Section, *VariableName, DefaultValue, Filename))
	{
		GConfig->SetInt(*Section, *VariableName, DefaultValue, Filename);
	}

	IConsoleManager::Get().UnregisterConsoleObject(*VariableName, /*bKeepState=*/false);
	IConsoleVariable* Variable = IConsoleManager::Get().RegisterConsoleVariable(*VariableName, DefaultValue, *Section);
}

void UKismetSystemLibraryEx::RegisterSettingBool(const FString& Section, const FString& VariableName, bool bDefaultValue)
{
	const FString& Filename = ProjectSettingsPath();

	// Get initial value from settings file, ensure it exists so it gets saved later
	EnsureConfigFileExists(Filename);
	if (!GConfig->GetBool(*Section, *VariableName, bDefaultValue, Filename))
	{
		GConfig->SetBool(*Section, *VariableName, bDefaultValue, Filename);
	}

	IConsoleManager::Get().UnregisterConsoleObject(*VariableName, /*bKeepState=*/false);
	IConsoleVariable* Variable = IConsoleManager::Get().RegisterConsoleVariable(*VariableName, bDefaultValue, *Section);
}

void UKismetSystemLibraryEx::RegisterSettingFloatWithCallback(const FString& Section, const FString& VariableName, float DefaultValue, FConsoleVariableChangedFloatSignature ChangedEvent, bool bChangeNow)
{
	const FString& Filename = ProjectSettingsPath();

	// Get initial value from settings file, ensure it exists so it gets saved later
	EnsureConfigFileExists(Filename);
	if (!GConfig->GetFloat(*Section, *VariableName, DefaultValue, Filename))
	{
		GConfig->SetFloat(*Section, *VariableName, DefaultValue, Filename);
	}

	IConsoleManager::Get().UnregisterConsoleObject(*VariableName, /*bKeepState=*/false);
	IConsoleVariable* Variable = IConsoleManager::Get().RegisterConsoleVariable(*VariableName, DefaultValue, *Section);
	FConsoleVariableDelegate Delegate = FConsoleVariableDelegate::CreateLambda([ChangedEvent](IConsoleVariable* Variable) -> void
		{
			ChangedEvent.ExecuteIfBound(Variable->GetFloat());
		});
	Variable->OnChangedDelegate().Add(Delegate);

	if (bChangeNow)
	{
		ChangedEvent.ExecuteIfBound(DefaultValue);
	}
}

void UKismetSystemLibraryEx::RegisterSettingIntWithCallback(const FString& Section, const FString& VariableName, int32 DefaultValue, FConsoleVariableChangedIntSignature ChangedEvent, bool bChangeNow)
{
	const FString& Filename = ProjectSettingsPath();

	// Get initial value from settings file, ensure it exists so it gets saved later
	EnsureConfigFileExists(Filename);
	if (!GConfig->GetInt(*Section, *VariableName, DefaultValue, Filename))
	{
		GConfig->SetInt(*Section, *VariableName, DefaultValue, Filename);
	}

	IConsoleManager::Get().UnregisterConsoleObject(*VariableName, /*bKeepState=*/false);
	IConsoleVariable* Variable = IConsoleManager::Get().RegisterConsoleVariable(*VariableName, DefaultValue, *Section);
	FConsoleVariableDelegate Delegate = FConsoleVariableDelegate::CreateLambda([ChangedEvent](IConsoleVariable* Variable) -> void
		{
			ChangedEvent.ExecuteIfBound(Variable->GetInt());
		});
	Variable->OnChangedDelegate().Add(Delegate);

	if (bChangeNow)
	{
		ChangedEvent.ExecuteIfBound(DefaultValue);
	}
}

void UKismetSystemLibraryEx::RegisterSettingBoolWithCallback(const FString& Section, const FString& VariableName, bool bDefaultValue, FConsoleVariableChangedBoolSignature ChangedEvent, bool bChangeNow)
{
	const FString& Filename = ProjectSettingsPath();

	// Get initial value from settings file, ensure it exists so it gets saved later
	EnsureConfigFileExists(Filename);
	if (!GConfig->GetBool(*Section, *VariableName, bDefaultValue, Filename))
	{
		GConfig->SetBool(*Section, *VariableName, bDefaultValue, Filename);
	}

	IConsoleManager::Get().UnregisterConsoleObject(*VariableName, /*bKeepState=*/false);
	IConsoleVariable* Variable = IConsoleManager::Get().RegisterConsoleVariable(*VariableName, bDefaultValue, *Section);
	FConsoleVariableDelegate Delegate = FConsoleVariableDelegate::CreateLambda([ChangedEvent](IConsoleVariable* Variable) -> void
		{
			ChangedEvent.ExecuteIfBound(Variable->GetBool());
		});
	Variable->OnChangedDelegate().Add(Delegate);

	if (bChangeNow)
	{
		ChangedEvent.ExecuteIfBound(bDefaultValue);
	}
}

float UKismetSystemLibraryEx::GetSettingFloat(const FString& VariableName)
{
	if (TConsoleVariableData<float>* Variable = IConsoleManager::Get().FindTConsoleVariableDataFloat(*VariableName))
	{
		return Variable->GetValueOnGameThread();
	}
	return {};
}

int32 UKismetSystemLibraryEx::GetSettingInt(const FString& VariableName)
{
	if (TConsoleVariableData<int32>* Variable = IConsoleManager::Get().FindTConsoleVariableDataInt(*VariableName))
	{
		return Variable->GetValueOnGameThread();
	}
	return {};
}

bool UKismetSystemLibraryEx::GetSettingBool(const FString& VariableName)
{
	if (TConsoleVariableData<int32>* Variable = IConsoleManager::Get().FindTConsoleVariableDataInt(*VariableName))
	{
		return Variable->GetValueOnGameThread() != 0;
	}
	return {};
}

template <typename T>
void UKismetSystemLibraryEx::Generic_SetSetting(const FString& VariableName, T NewValue)
{
	if (IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(*VariableName))
	{
		const FString& Filename = ProjectSettingsPath();
		GConfig->SetFloat(Variable->GetHelp(), *VariableName, NewValue, Filename);

		Variable->SetWithCurrentPriority(NewValue);
	}
}

void UKismetSystemLibraryEx::SetSettingFloat(const FString& VariableName, float NewValue)
{
	Generic_SetSetting(VariableName, NewValue);
}

void UKismetSystemLibraryEx::SetSettingInt(const FString& VariableName, int32 NewValue)
{
	Generic_SetSetting(VariableName, NewValue);
}

void UKismetSystemLibraryEx::SetSettingBool(const FString& VariableName, bool bNewValue)
{
	Generic_SetSetting(VariableName, bNewValue);
}

void UKismetSystemLibraryEx::SaveSettings()
{
	const FString& Filename = ProjectSettingsPath();
	if (FConfigFile* Ini = GConfig->FindConfigFile(Filename))
	{
		Ini->Write(Filename);
	}
}
