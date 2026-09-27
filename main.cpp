#include "nlohmann/json.hpp"
#include "Classes/Enemy.hpp"
#include "Classes/weaponMod.hpp"
#include <fstream>
#include <iostream>
#include <format>
#include <string>
#include <cmath>
#include <algorithm>
// json loading thanks to nlohmann, further info in nlohmann/json.hpp

using json = nlohmann::json;


json loadJsonFile(std::string fileName)
{
    // std::cout << ("Loading " + fileName + "\n");

    std::ifstream jsonFile(fileName);
    
    // std::cout << "Parsing data\n";

    json parsedJson;
    try {
        parsedJson = json::parse(jsonFile);
        jsonFile.close();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error parsing " << fileName << "   :   " << e.what() << std::endl;
    }
    // std::cout << "Parsed without errors (at least I hope)\n";
    return parsedJson;
}




    // TODO: implement changing this based on chosen stance for melee weapons, when melee weapons are fully implemented
    double tempBaseComboLength = 3;





std::tuple<double, double, double> calculateDPSValues(nlohmann::json moddedWeapon, int attackIndex, std::string weaponType, Enemy currEnemy)
{
    // ---------------------------------------- CALCULATE DAMAGE ----------------------------------------
    // std::cout << "I at least got here\n";

    // Calculate totalDamage
    double totalDamage = 0;
    double multishotValue = 0;

    // std::cout << "I got here!\n";

    nlohmann::json currAttack = moddedWeapon["Attacks"][attackIndex];
    for (const auto& damageType : (currAttack["Damage"].items()))
    {
        totalDamage += damageType.value().get<double>();
    }
    if (currAttack.contains("Multishot"))
    {
        multishotValue = currAttack["Multishot"];
    } else
    {
        multishotValue = 1;
    }

    // std::cout << "I got here too!\n";


    std::string triggerType = "";
    if (weaponType == "Primary" || weaponType == "Secondary")
    {
        if (currAttack.contains("Trigger")) // use particular attacks trigger value if it exists, else use weapons trigger value
        {
            triggerType = currAttack["Trigger"];
        }
        else if (moddedWeapon.contains("Trigger"))
        {
            triggerType = moddedWeapon["Trigger"];
        }
        else
        {
            std::cout << "ABORT, Neither the attack nor weapon have a 'Trigger' type" << std::endl;
        }
    }

    double effectiveFireRate = 0;

    if (weaponType == "Melee" ||
    triggerType == "Auto" ||
    triggerType == "Semi-Auto" ||
    triggerType == "Duplex" ||
    triggerType == "Held")
    {
        effectiveFireRate = currAttack["FireRate"];
    }
    else if (triggerType == "Charge")
    {
        effectiveFireRate = (1 / (static_cast<double>(currAttack["ChargeTime"]) + (1 / static_cast<double>(currAttack["FireRate"]))));
    }
    else if (triggerType == "Burst" || triggerType == "Auto Burst")
    {
        effectiveFireRate = (static_cast<double>(currAttack["BurstCount"]) / ((1 / static_cast<double>(currAttack["FireRate"])) + ((static_cast<double>(currAttack["BurstCount"]) - 1) * static_cast<double>(currAttack["BurstDelay"]))));
    }
    else
    {
        std::cout << "ABORT! I couldn't find the trigger type for this attack, or at least it isn't Auto, Semi-Auto, Duplex, Held, Charge, Burst, or Auto Burst" << std::endl;
    }
    
        std::cout << "I got here!\n";

    // Calculate status amounts on enemy
    for (const auto& damageType : (currAttack["Damage"].items()))
    {
        // use each types damage as a proportion of totalDamage to get damage distribution
        // multiply by status chance to get amount applied per hit
        // multiply by multishot to get amount applied per shot
        // multiply by effective fire rate to get procs/second
        // divide by time to expire or something to find amount per second on average considering expiration time
        // cap at max amount
        // add to enemy
        // update damage calculations ot take into account the CC, CD, etc. buffs
        double proportionOfTotalDamage = damageType.value().get<double>() / totalDamage;
        double statusAppliedPerHit = proportionOfTotalDamage * static_cast<double>(currAttack["StatusChance"]);
        double statusAppliedPerShot = statusAppliedPerHit * multishotValue;
        double statusAppliedPerSecond = statusAppliedPerShot * effectiveFireRate;

        // multiply by status duration
        double averageStatusCount = statusAppliedPerSecond * (currEnemy.getStatusDurations())[damageType.key()];
        // round down if above cap
        double finalStatusCount = 0;
        if (averageStatusCount >= (currEnemy.getStatusCaps())[damageType.key()])
        {
            finalStatusCount = (currEnemy.getStatusCaps())[damageType.key()];
        } else
        {
            finalStatusCount = averageStatusCount;
        }

        std::unordered_map<std::string, double> currStatusCounts = currEnemy.getStatusCounts();
        currStatusCounts[damageType.key()] = finalStatusCount;
        currEnemy.setStatusCounts(currStatusCounts);
    }

    double normalShot = totalDamage * (1 + (std::floor(static_cast<double>(currAttack["CritChance"]) + currEnemy.getAddedCritChance()) * (static_cast<double>(currAttack["CritMultiplier"]) - 1.0f + currEnemy.getAddedCritDamage())));
    double criticalShot = totalDamage * (1 + (std::ceil(static_cast<double>(currAttack["CritChance"]) + currEnemy.getAddedCritChance()) * (static_cast<double>(currAttack["CritMultiplier"]) - 1.0f + currEnemy.getAddedCritDamage())));
    double averageShot = totalDamage * ((1 + static_cast<double>(currAttack["CritChance"]) + currEnemy.getAddedCritChance()) * (static_cast<double>(currAttack["CritMultiplier"]) - 1 + currEnemy.getAddedCritDamage()));



    // Starting Values
    double averageBurstDPS = 0;
    double numberOfShotsPerMag = 0;
    double averageSustainedDPS = 0;
    double ammoCostPerShot = 0;
    double percentOfTimeShooting = 0;

    // Gun DPS
    if (weaponType == "Primary" || weaponType == "Secondary")
    {
        if (currAttack.contains("AmmoCost"))
        {
            ammoCostPerShot = currAttack["AmmoCost"];
        }

        // avg burst dps (held but no reloads)
        // apply viral + corrosive to damage now too
        averageBurstDPS = averageShot * effectiveFireRate;

        // used to calculate time reloading
        if (moddedWeapon.contains("Magazine"))
        {
            numberOfShotsPerMag = static_cast<double>(moddedWeapon["Magazine"]) / ammoCostPerShot;
            percentOfTimeShooting = numberOfShotsPerMag / ((effectiveFireRate * static_cast<double>(moddedWeapon["Reload"])) + numberOfShotsPerMag);
        }
        else
        {
            // infinite magazine, so always shooting.
            numberOfShotsPerMag = INFINITY;
            percentOfTimeShooting = 1;

        }

        // percent of time shooting vs reloading

        // Avg sustained dps
        averageSustainedDPS = averageBurstDPS * percentOfTimeShooting;
    }
    // Melee DPS
    else if (weaponType == "Melee")
    {
        // Going to have to add a slot for combo mods to choose from for the melee weapons, then i need to update this
        // TODO: update this calculation once combo mod can be chosen
        averageSustainedDPS = (averageShot * static_cast<double>(currAttack["FireRate"]) / static_cast<double>(moddedWeapon["ComboDur"]));
    }
    return { averageShot, averageBurstDPS, averageSustainedDPS };
}





struct weaponType {
    int weaponTypeID;
    std::string name;
    int parentTypeID;
};

// returns vector of strings of all compatible mod types
std::vector<std::string> getValidModTypes(std::string& weaponTypeName, std::vector<weaponType> weaponTypeTree)
{
    int weaponTypeID = -1;
    for (weaponType currWeaponType : weaponTypeTree)
    {
        if (currWeaponType.name == weaponTypeName)
        {
            weaponTypeID = currWeaponType.weaponTypeID;
        }
    }

    std::vector<std::string> validModTypes = {};
    int parentWeaponTypeID = 9999;
    while (parentWeaponTypeID != -1)
    {
        parentWeaponTypeID = weaponTypeTree[weaponTypeID - 1].parentTypeID;
        if (weaponTypeTree[weaponTypeID - 1].name == "Assault Saw" ||
        weaponTypeTree[weaponTypeID - 1].name == "Bayonet" ||
        weaponTypeTree[weaponTypeID - 1].name == "Blade and Whip" ||
        weaponTypeTree[weaponTypeID - 1].name == "Claws" ||
        weaponTypeTree[weaponTypeID - 1].name == "Dagger" ||
        weaponTypeTree[weaponTypeID - 1].name == "Dual Nikanas" ||
        weaponTypeTree[weaponTypeID - 1].name == "Dual Swords" ||
        weaponTypeTree[weaponTypeID - 1].name == "Fist" ||
        weaponTypeTree[weaponTypeID - 1].name == "Gunblade" ||
        weaponTypeTree[weaponTypeID - 1].name == "Hammer" ||
        weaponTypeTree[weaponTypeID - 1].name == "Heavy Blade" ||
        weaponTypeTree[weaponTypeID - 1].name == "Heavy Scythe" ||
        weaponTypeTree[weaponTypeID - 1].name == "Machete" ||
        weaponTypeTree[weaponTypeID - 1].name == "Nikana" ||
        weaponTypeTree[weaponTypeID - 1].name == "Nunchaku" ||
        weaponTypeTree[weaponTypeID - 1].name == "Rapier" ||
        weaponTypeTree[weaponTypeID - 1].name == "Scythe" ||
        weaponTypeTree[weaponTypeID - 1].name == "Sparring" ||
        weaponTypeTree[weaponTypeID - 1].name == "Staff" ||
        weaponTypeTree[weaponTypeID - 1].name == "Sword" ||
        weaponTypeTree[weaponTypeID - 1].name == "Sword and Shield" ||
        weaponTypeTree[weaponTypeID - 1].name == "Two-Handed Nikana" ||
        weaponTypeTree[weaponTypeID - 1].name == "Tonfa" ||
        weaponTypeTree[weaponTypeID - 1].name == "Warfan" ||
        weaponTypeTree[weaponTypeID - 1].name == "Whip")
        {
            continue;   //  only used for stance mods
        }
        else if (weaponTypeTree[weaponTypeID - 1].name == "Glaive")
        {
            validModTypes.push_back("Thrown Melee");
        }
        else if (weaponTypeTree[weaponTypeID - 1].name == "Polearm")
        {
            validModTypes.push_back("Polearms");
        }
        else if (weaponTypeTree[weaponTypeID - 1].name == "Sniper Rifle")
        {
            validModTypes.push_back("Sniper");
        }
        else
        {
            validModTypes.push_back(weaponTypeTree[weaponTypeID - 1].name);
        }
        weaponTypeID = parentWeaponTypeID;
    }

    return validModTypes;
}




// main function
int main()
{
    std::cout << "I at least ran the main function\n";
    json wikiPrimaryWeaponData = loadJsonFile("wikiData/wikiExportPrimary.json");
    json wikiSecondaryWeaponData = loadJsonFile("wikiData/wikiExportSecondary.json");
    json wikiMeleeWeaponData = loadJsonFile("wikiData/wikiExportMelee.json");
    json warframeData = loadJsonFile("ExportWarframes_en.json");
    json SentinelsData = loadJsonFile("ExportSentinels_en.json");
    json wikiModsData = loadJsonFile("wikiData/wikiExportMods.json");
    json wikiArcaneData = loadJsonFile("wikiData/wikiExportArcanes.json");
    std::cout << "I at least loaded the data!\n";
    

    // retrieve weapon (eventually this will loop to do this for every weapon, or for a specified weapon)

    
    // ---------------------------------------- USER SETTINGS ----------------------------------------
    bool heatArmorStrip = false;
    int maxDrain = 999;
    // ---------------------------------------- USER SETTINGS ----------------------------------------
    std::string weaponName = "";
    std::string weaponGeneralClass = "";
    bool weaponGeneralClassChosen = false;
    while (!weaponGeneralClassChosen)
    {
        std::cout << "What type of weapon do you want to optimize? Type 'Primary', 'Secondary' or 'Melee'\n";
        std::cin >> weaponGeneralClass;
        if (weaponGeneralClass == "Primary" || weaponGeneralClass == "Secondary" || weaponGeneralClass == "Melee")
        {
            std::cout << "Great! Which " << weaponGeneralClass << " do you want to optimize? Be sure to capitalize each part of it's name, like 'Dual Coda Torxica'\n";
            weaponGeneralClassChosen = true;
        }
    }
    json selectedWeaponType;
    std::vector<weaponType> weaponTypeTree;
    if (weaponGeneralClass == "Primary") {
        // Define weapon type structure
        weaponTypeTree = {
            {1, "Primary", -1},
            {2, "Rifle", 1},
            {3, "Shotgun", 1},
            {4, "Assault Rifle", 2},
            {5, "Bow", 2},
            {6, "Sniper Rifle", 2},   //  Mods use "Sniper", weapons use "Sniper Rifle"
            {7, "Launcher", 4},
            {8, "Speargun", 7},
            {9, "Crossbow", 5}
        };

        selectedWeaponType = wikiPrimaryWeaponData;
    } else if (weaponGeneralClass == "Secondary") {
        // Define weapon type structure
        weaponTypeTree = {
            {1, "Secondary", -1},
            {2, "Pistol", 1},
            {3, "Thrown", 1},
            {4, "Tome", 1},
            {5, "Dual Pistols", 2},
            {6, "Dual Shotguns", 2},
            {7, "Shotgun Sidearm", 2},
            {8, "Crossbow", 2}
        };
        selectedWeaponType = wikiSecondaryWeaponData;
    } else if (weaponGeneralClass == "Melee") {
        // Define weapon type structure
        weaponTypeTree = {
            {1, "Melee", -1},
            {2, "Assault Saw", 1},
            {3, "Bayonet", 1},
            {4, "Blade and Whip", 1},
            {5, "Claws", 1},
            {6, "Dagger", 1},
            {7, "Dual Daggers", 1},
            {8, "Dual Nikanas", 1},
            {9, "Dual Swords", 1},
            {10, "Fist", 1},
            {11, "Glaive", 1},
            {12, "Gunblade", 1},
            {13, "Hammer", 1},
            {14, "Heavy Blade", 1},
            {15, "Heavy Scythe", 1},
            {16, "Machete", 1},
            {17, "Nikana", 1},
            {18, "Nunchaku", 1},
            {19, "Polearm", 1},
            {20, "Rapier", 1},
            {21, "Scythe", 1},
            {22, "Sparring", 1},
            {23, "Staff", 1},
            {24, "Sword", 1},
            {25, "Sword and Shield", 1},
            {26, "Two-Handed Nikana", 1},
            {27, "Tonfa", 1},
            {28, "Warfan", 1},
            {29, "Whip", 1}
        };
        selectedWeaponType = wikiMeleeWeaponData;
    }

    // clear newline from input
    std::cin.ignore();

    bool foundWeaponToOptimize = false;
    while (!foundWeaponToOptimize)
    {
        // change to getline so it reads until newline instead of until a whitespace character
        std::getline(std::cin, weaponName);
        std::cout << "Trying to optimize " << weaponName << std::endl;
        if (!selectedWeaponType.contains(weaponName))
        {
            std::cout << "Couldn't find a weapon called " << weaponName << ", please double check spelling and capitalization and try again" << std::endl;
        }
        else
        {
            foundWeaponToOptimize = true;
        }
    }


    nlohmann::json currentWeapon = selectedWeaponType[weaponName];

    double averageShot = 0;
    double averageBurstDPS = 0;
    double averageSustainedDPS = 0;
    std::vector<std::string> singleShotMods = {};
    std::vector<std::string> singleBurstDPSMods = {};
    std::vector<std::string> singleSustainedDPSMods = {};


    // filter to only check valid mod options
    std::vector<nlohmann::json> compatibleMods {};
    int currentModIndex = 0;
    std::string currentWeaponType = currentWeapon["Class"];

    /// Skip any mods with IsFlawed that is set to true
    /// If any entry in "Incompatible" starts with "Primed" there is a prime variant, so ignore the base    -   done in loading mods, not needed in this file


    std::vector<weaponMod> validMods = {};
    for (auto& [modName, modData] : wikiModsData["Mods"].items())
    {
        std::vector<std::string> validModTypes = getValidModTypes(currentWeaponType, weaponTypeTree);

        // prune flawed mods here too
        if (modData.value("IsFlawed", false))
        {   //  skp this mod, is flawed
            continue;
        }
        else if (std::find(validModTypes.begin(), validModTypes.end(), modData["Type"]) != validModTypes.end()) // melee mod
        {
            // push entries or default to empty strings/vectors of strings if that field doesn't exist for this mod
            validMods.push_back(weaponMod(modData.value("Name", ""), modData.value("Type", ""), modData.value("Description", ""), modData.value("Set", ""), modData.value("Class", ""), modData.value("IncompatibilityTags", std::vector<std::string>{}), modData.value("Incompatible", std::vector<std::string>{}), modData.value("UpgradeTypes", std::vector<std::string>{})));
            //  std::cout << "Adding " << modData.value("Name", " ") << modData.contains("Description") << std::endl;
        }
        else
        { // skip this mod, not valid
            continue;
        }
    }

    // prune extra mods
    for (auto currMod = validMods.begin(); currMod != validMods.end();)
    {   // iterate from start until end
        if (currMod->getPruneThis() == true)
        {   //  if mod should be pruned, erase it, iterator auto skips to the next entry
            currMod = validMods.erase(currMod);
            //  std::cout << "Pruning " << currMod->name << std::endl;
        }
        else
        {   //  if not pruned then iterate to next valid mod
            currMod++;
        }
    }
    

    // for (weaponMod currMod : validMods)
    // {
    //     std::cout << currMod.name << " is compatible with " << currentWeapon["Name"]  << " and I am accounting for it" << std::endl;
    // }

    int attackIndex = 1;
    while (attackIndex <= currentWeapon["Attacks"].size())
    {
        // TODO: figure out how to make this be while there are still mod combinations left
        // nested for loops, each starting at 1 higher index, each ending 1 index earlier from the end of valid mods vector
        // then also make a loop that tries each permutation of elemental mods to try their configs
        for (int i = 0; i <= 0; i++)
        {
            nlohmann::json moddedWeapon = currentWeapon;
            std::vector<std::string> currentModConfig = {};
            // ---------------------------------------- CHOOSE MODS HERE ----------------------------------------

            // will edit modded entry based on mods here? maybe? maybe try each mod then pass to damage calc?
            // ---------------------------------------- CHOOSE MODS HERE ----------------------------------------


            // ---------------------------------------- BASE ENEMY INFO ----------------------------------------
            Enemy currEnemy = Enemy();



            
            // ---------------------------------------- CALCULATE DAMAGE ----------------------------------------

            auto [tempAverageShot, tempAverageBurstDPS, tempAverageSustainedDPS] = calculateDPSValues(moddedWeapon, (attackIndex - 1), weaponGeneralClass, currEnemy);
            if (tempAverageShot > averageShot)
            {
                averageShot = tempAverageShot; // TODO: save mod config too
            }
            if (tempAverageBurstDPS > averageBurstDPS)
            {
                averageBurstDPS = tempAverageBurstDPS; // TODO: save mod config too
            }
            if (tempAverageSustainedDPS > averageSustainedDPS)
            {
                averageSustainedDPS = tempAverageSustainedDPS; // TODO: save mod config too
            }
            std::cout << "For the: " << currentWeapon["Name"] << "'s attack number " << attackIndex << " the stats are as follows: Average shot: " << tempAverageShot << ", Average burst DPS: " << tempAverageBurstDPS << ", Average Sustained DPS: " << tempAverageSustainedDPS << std::endl;
        }
        attackIndex++;
    }
    return 0;
}


