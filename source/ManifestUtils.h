#include <YYToolkit/YYTK_Shared.hpp>
#include <nlohmann/json.hpp>
#include <string.h>
#include <fstream>
#include <unordered_set>
#include <miniz/miniz.h>

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
	bool isZip = false;
};

static std::map<std::string, json> dependencyMap;
bool FileExists(const std::string& filename) {
	return fs::exists(filename);
}


std::string ReadFileFromZip(const std::string& zipPath, const std::string& filePath) {
	mz_zip_archive zip_archive;
	memset(&zip_archive, 0, sizeof(zip_archive));

	if (!mz_zip_reader_init_file(&zip_archive, zipPath.c_str(), 0)) {
		return "";
	}

	std::string content;

	for (unsigned int i = 0; i < mz_zip_reader_get_num_files(&zip_archive); i++) {
		mz_zip_archive_file_stat file_stat;
		if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) {
			continue;
		}
		if (fs::path(file_stat.m_filename).filename() == filePath) {
			content.resize(file_stat.m_uncomp_size);

			bool success = mz_zip_reader_extract_to_mem(
				&zip_archive,
				i,
				content.data(),
				content.size(),
				0
			);

			if (!success) {
				content.clear();
			}
			break;
		}
	}

	mz_zip_reader_end(&zip_archive);
	return content;
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

json ParseZipManifest(const fs::path& zipPath)
{
	String modName = zipPath.filename().string();

	std::string manifestContent = ReadFileFromZip(zipPath.string(), "manifest.json");
	if (manifestContent.empty())
		return json();

	json manifest_json_object;

	try
	{
		manifest_json_object = json::parse(manifestContent);
	}
	catch (const json::parse_error& e)
	{
		g_ModuleInterface->PrintError(
			__FILE__, __LINE__,
			"[%s v%s] Failed to parse manifest for %s inside %s: %s",
			MOD_NAME,
			VERSION,
			modName.c_str(),
			zipPath.string().c_str(),
			e.what()
		);
		return json();
	}

	if (manifest_json_object.empty())
	{
		g_ModuleInterface->PrintWarning(
			"[%s v%s] Skipping %s since its' manifest is empty! Please contact the author of %s about this.",
			MOD_NAME,
			VERSION,
			modName.c_str(),
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
			{ "version", mod_info["version"] },
			{ "zip", manifest.isZip }
			});
		
	}
}

void RegisterDependency(std::vector<Manifest> manifests) {
	for (const auto& manifest : manifests) {
		RegisterDependency(manifest);
	}
}

void SaveDependencyMap(const std::string& filename) {
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
		if (fs::is_directory(entry)) {
			json _manifest = ParseManifest(modDirectory);
			if (!_manifest.empty()) {
				Manifest _manifestStruct = Manifest(_manifest, modDirectory.string());
				manifests.push_back(_manifestStruct);
			}
		} else if (modDirectory.extension() == ".zip" ||
			fs::path(entry.path()).extension().string() == ".ZIP") {
			json _manifest = ParseZipManifest(modDirectory);
			if (!_manifest.empty())
			{

				Manifest _manifestStruct{ _manifest, modDirectory.string(), true };
				manifests.push_back(_manifestStruct);
			}
		}

	}

	return manifests;
}

void BuildRequirementsJson() {
	String gameFolder = fs::current_path().string();
	String modsFolder = gameFolder + "\\mods\\";
	String modDataFolder = gameFolder + "mod_data";
	std::vector<Manifest> manifests = ParseManifests(modsFolder);
	RegisterDependency(manifests);
	if (!std::filesystem::exists(modDataFolder))
	{
		g_ModuleInterface->Print(CM_LIGHTYELLOW, "[%s %s] - The \"mod_data\" directory was not found. Creating directory: %s", MOD_NAME, VERSION, modDataFolder.c_str());
		std::filesystem::create_directory(modDataFolder);
	}
	SaveDependencyMap("mod_data\\requirements.json");
}