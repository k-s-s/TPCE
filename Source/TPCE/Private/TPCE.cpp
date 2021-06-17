// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "TPCE.h"
#include "Modules/ModuleManager.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebugger.h"
#include "Jobs/GameplayDebuggerCategory_Jobs.h"
#endif // WITH_GAMEPLAY_DEBUGGER

DEFINE_LOG_CATEGORY(LogTPCE)

#define LOCTEXT_NAMESPACE "TPCE"

class FTPCE : public IModuleInterface
{
	// Begin IModuleInterface Interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	// End IModuleInterface Interface
};

IMPLEMENT_MODULE(FTPCE, TPCE);

void FTPCE::StartupModule()
{
	UE_LOG(LogTPCE, Log, TEXT("Third Person Character Extensions (TPCE) Module Started"));

#if WITH_GAMEPLAY_DEBUGGER
	IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
	GameplayDebuggerModule.RegisterCategory("Jobs", IGameplayDebugger::FOnGetCategory::CreateStatic(&FGameplayDebuggerCategory_Jobs::MakeInstance), EGameplayDebuggerCategoryState::EnabledInGameAndSimulate);
	GameplayDebuggerModule.NotifyCategoriesChanged();
#endif
}

void FTPCE::ShutdownModule()
{
	UE_LOG(LogTPCE, Log, TEXT("Third Person Character Extensions (TPCE) Module Shutdown"));

#if WITH_GAMEPLAY_DEBUGGER
	if (IGameplayDebugger::IsAvailable())
	{
		IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
		GameplayDebuggerModule.UnregisterCategory("Jobs");
		GameplayDebuggerModule.NotifyCategoriesChanged();
	}
#endif
}

#undef LOCTEXT_NAMESPACE
