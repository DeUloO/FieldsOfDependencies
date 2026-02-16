#include <YYToolkit/YYTK_Shared.hpp>
#include <nlohmann/json.hpp>
#include <string.h>
#include <fstream>
#include <unordered_set>

#include "Module.h"

using namespace Aurie;
using namespace YYTK;

using json = nlohmann::json;
using String = std::string;
namespace fs = std::filesystem;

struct Manifest
{
	json manifest;
	String path;
};

static std::map<std::string, json> dependencyMap;
bool FileExists(const std::string& filename) {
	return fs::exists(filename);
}

json ParseManifest(fs::path modFolder) {
	fs::path manifest = "manifest.json";
	fs::path manifestLocation = modFolder / manifest ;
	String modName = modFolder.filename().string();
	if (!fs::exists(manifestLocation))
		return json();
		


	std::ifstream in_stream(manifestLocation);
	if (!in_stream.good())
		g_ModuleInterface->PrintError(__FILE__, __LINE__,
			"[%s v%s] Something went wrong whilst trying to read the manifest for %s at %s. Please contact the author of %s not %s for help with this.",
			MOD_NAME,
			VERSION,
			modName.c_str(),
			manifestLocation.string().c_str(),
			MOD_NAME,
			modName.c_str()
		);

	json manifest_json_object;
	try
	{
		manifest_json_object = json::parse(in_stream);
	}
	catch (const json::parse_error& e)
	{
		g_ModuleInterface->PrintError(
			__FILE__, __LINE__,
			"[%s v%s] Failed to parse manifest for %s at %s: %s",
			MOD_NAME,
			VERSION,
			modName.c_str(),
			manifestLocation.string().c_str(),
			e.what()
		);
		return json();
	}

	if (manifest_json_object.empty()) {
		g_ModuleInterface->PrintWarning(
			"[%s v%s] Skipping %s since its' manifest is empty! Please contact the author of %s about this.",
			MOD_NAME,
			VERSION,
			modName.c_str()
		);
		return manifest_json_object;
	}




	return manifest_json_object;
}

void RegisterDependency(Manifest manifest) {
	
	if (!manifest.manifest.contains("requirements"))
		return;
	String mod_name = manifest.manifest["name"];
	for (const auto& mod_info : manifest.manifest["requirements"])
	{
		String required_mod_name = mod_info["name"];
		dependencyMap[required_mod_name].push_back({
			{ "path", manifest.path},
			{ "version", mod_info["version"] }
			});
		
	}
}

void RegisterDependency(std::vector<Manifest> manifests) {
	for (const auto& manifest : manifests) {
		RegisterDependency(manifest);
	}
}

void SaveDependencyMap(const std::string& filename)
{
	json j = dependencyMap;

	std::ofstream out(filename);
	if (!out)
		return;

	out << j.dump(4);
}

std::vector<Manifest> ParseManifests(String modsDirectory) {
	std::vector<Manifest> manifests = {};
	std::unordered_map<String, String> modNamePathMapping = {};
	for (const auto& entry : fs::directory_iterator(modsDirectory)) {
		fs::path modDirectory = entry.path();

		json _manifest = ParseManifest(modDirectory);
		if (_manifest.empty())
			continue;

		Manifest _manifestStruct = Manifest(_manifest, modDirectory.string());
		manifests.push_back(_manifestStruct);

	}

	return manifests;
}

void BuildRequirementsJson() {
	String gameFolder = fs::current_path().string();
	String modsFolder = gameFolder + "\\mods\\";
	std::vector<Manifest> manifests = ParseManifests(modsFolder);
	RegisterDependency(manifests);
	SaveDependencyMap("mod_data\\requirements.json");
}