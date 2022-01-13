// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "TPCEUncooked.h"

class FTPCEUncookedModule : public IModuleInterface
{
	// Begin IModuleInterface
	virtual void StartupModule() override;
	// End IModuleInterface
};

IMPLEMENT_MODULE(FTPCEUncookedModule, TPCEUncooked);

void FTPCEUncookedModule::StartupModule()
{
	// Not sure if necessary. See https://forums.unrealengine.com/t/4-24-deprecation-of-developer-module-type-uncookedonly-not-working-on-nativized-bp-assets/134758
	/*
	const FString LongName = FPackageName::ConvertToLongScriptPackageName(TEXT("TPCEUncooked"));
	if (UPackage* Package = Cast<UPackage>(StaticFindObjectFast(UPackage::StaticClass(), nullptr, *LongName, false, false)))
	{
		Package->SetPackageFlags(PKG_EditorOnly);
	}
	*/
}
