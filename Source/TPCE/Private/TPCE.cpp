// This source code is licensed under the MIT license found in the LICENSE file in the root directory of this source tree.

#include "TPCE.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogTPCE)

#define LOCTEXT_NAMESPACE "TPCE"

class FTPCE: public IModuleInterface
{
public:

	virtual void StartupModule() override
	{
		UE_LOG(LogTPCE, Log, TEXT("Third Person Character Extensions (TPCE) Module Started"));
	}

	virtual void ShutdownModule() override
	{
		UE_LOG(LogTPCE, Log, TEXT("Third Person Character Extensions (TPCE) Module Shutdown"));
	}
};

IMPLEMENT_MODULE(FTPCE, TPCE);

#undef LOCTEXT_NAMESPACE