#include <YYToolkit/YYTK_Shared.hpp>
#include "Module.h"
#include "ManifestUtils.h"
using namespace Aurie;
using namespace YYTK;


const char* const VERSION = "1.0.0";
const char* const MOD_NAME = "FieldsOfDependencies";


EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);
	AurieStatus last_status = AURIE_SUCCESS;

	last_status = ObGetInterface(
		"YYTK_Main",
		(AurieInterfaceBase*&)(g_ModuleInterface)
	);

	if (!AurieSuccess(last_status))
	{
		g_ModuleInterface->PrintError(
			__FILE__,
			__LINE__,
			"[%s v%s] Failed to get YYTK_Main interface! Reason: %s. Is YYToolkit installed?",
			MOD_NAME,
			VERSION,
			AurieStatusToString(last_status)
		);

		return last_status;
	}

	last_status = g_ModuleInterface->GetGlobalInstance(&global_instance);

	if (!AurieSuccess(last_status))
	{
		g_ModuleInterface->PrintError(
			__FILE__,
			__LINE__,
			"[%s v%s] Failed to get the global instance! Reason: %s.",
			MOD_NAME,
			VERSION,
			AurieStatusToString(last_status)
		);
	}

	BuildRequirementsJson();

	return AURIE_SUCCESS;
}