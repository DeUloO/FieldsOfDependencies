# Fields of Dependencies

A [YYToolkit (YYTK)](github.com/AurieFramework/YYToolkit) mod that automatically builds a `requirements.json` file in your game's `mod_data` folder. This mod is designed for **mod creators** who want to use the functionality of other YYTK mods (such as Annanomoly's [Dynamic Portraits](https://www.nexusmods.com/fieldsofmistria/mods/535)).

---

## Why This Mod Exists

Previously, using mods like [Crys's Dynamic Portraits - Balor](https://www.nexusmods.com/fieldsofmistria/mods/530) that used Annanomoly's [Dynamic Portraits](https://www.nexusmods.com/fieldsofmistria/mods/535) required users to move the mod_data folder from a mod. This mod **automates the process** by providing a centralised `requirements.json` file, allowing other mods to check for and locate their dependencies automatically.

> **Note:** This mod does **not** currently verify if the requirements are actually met. It simply provides a unified way for mods to declare and locate where they are needed through dependencies.

---

## For Mod Creators wanting to use a dependency
Your manifest.json only needs to declare the extra field "requirements" as has been done in the example below:
```json
{
  "name": "Crys's Flowers of Mistria",
  "author": "Crys234 & D3Ulo",
  "version": "1.0",
  "requirements":
    [
        {
            "name": "Crop Variants",
            "version": "1.0"
        }
	]
}
```

## For Mod Creators that are a aiming to be a dependency
Your mod can read the `requirements.json` file to find the paths of required mods.
The Fields of Dependencies automatically creates this file at:
```
.../Fields_of_Mistria/mod_data/
```
The key in the Json is your mods' name as defined in your own `manifest.json`.

Here is an example of the `requirements.json`:
```json
{
    "Crop Variants": [
        {
            "path": "Z:\\SteamLibrary\\steamapps\\common\\Fields of Mistria\\mods\\Crys's Crops of Mistria",
            "version": "1.0"
        },
        {
            "path": "Z:\\SteamLibrary\\steamapps\\common\\Fields of Mistria\\mods\\Crys's Flowers of Mistria",
            "version": "1.0"
        }
    ]
}
```
---

## Future Plans

- Add automatic verification of requirements.
- Probably nothing else?

---

## Contributing

Pull requests and suggestions are welcome! Please open an issue or submit a PR for any improvements or bug fixes.

---
