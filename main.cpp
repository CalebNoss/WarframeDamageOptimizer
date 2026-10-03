#include "nlohmann/json.hpp"
#include "Classes/Enemy.hpp"
#include "Classes/weaponMod.hpp"
#include "Classes/weaponModConfig.hpp"
#include "Classes/Weapon.hpp"
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







std::tuple<double, double, double> calculateDPSValues(Weapon& moddedWeapon, attackData& currAttack, std::string& weaponType, Enemy& currEnemy)
{
    // ---------------------------------------- CALCULATE DAMAGE ----------------------------------------
    // std::cout << "Starting damage calcs\n";

    // Calculate totalDamage
    double totalDamage = 0;
    double multishotValue = 0;


    for (auto& damageType : currAttack.damage)
    {
        totalDamage += damageType;
        // if (damageType.second != 0)
        // {
        //     std::cout << "The " << damageType.first << " damage is: " << damageType.second << std::endl;
        // }
    }
    if (currAttack.multishot == 0)
    {
        multishotValue = 1;
    }
    else
    {
        multishotValue = currAttack.multishot;
    }


    

    // std::cout<< "The total damage is: " << totalDamage << std::endl;

    // std::cout << "I summed total damage and got multishot value!\n";


    std::string triggerType = "";
    if (weaponType == "Primary" || weaponType == "Secondary")
    {
        if (currAttack.triggerType != "") // use particular attacks trigger value if it exists, else use weapons trigger value
        {
            triggerType = currAttack.triggerType;
        }
        else if (moddedWeapon.triggerType != "")
        {
            triggerType = moddedWeapon.triggerType;
        }
        else
        {
            std::cout << "ABORT, Neither the attack nor weapon have a 'Trigger' type" << std::endl;
        }
    }

    double effectiveFireRate = 0;

    // std::cout << "I got trigger types!\n";


    if (weaponType == "Melee" ||
    triggerType == "Auto" ||
    triggerType == "Semi-Auto" ||
    triggerType == "Duplex" ||
    triggerType == "Held")
    {
        effectiveFireRate = currAttack.fireRate;
    }
    else if (triggerType == "Charge")
    {
        effectiveFireRate = (1 / (moddedWeapon.reloadRate + (1 / currAttack.fireRate)));
    }
    else if (triggerType == "Burst" || triggerType == "Auto Burst")
    {
        effectiveFireRate = ((currAttack.burstCount) / ((1 / currAttack.fireRate) + ((currAttack.burstCount - 1) * currAttack.burstDelay)));
    }
    else
    {
        std::cout << "ABORT! I couldn't find the trigger type for this attack, or at least it isn't Auto, Semi-Auto, Duplex, Held, Charge, Burst, or Auto Burst" << std::endl;
    }
    
    // std::cout << "I calculated the fire rate!\n";

    int distinctStatusCount = 0;

    // Calculate status amounts on enemy
    std::array<double, 14>* currStatusCounts = currEnemy.getStatusCounts();
    for (int i = 0; i < 14; i++)
    {
        if (currAttack.damage[i] != 0)  // skip if this damage doesn't exist
        {
            // use each types damage as a proportion of totalDamage to get damage distribution
            // multiply by status chance to get amount applied per hit
            // multiply by multishot to get amount applied per shot
            // multiply by effective fire rate to get procs/second
            // divide by time to expire or something to find amount per second on average considering expiration time
            // cap at max amount
            // add to enemy
            // update damage calculations ot take into account the CC, CD, etc. buffs
            double proportionOfTotalDamage = currAttack.damage[i] / totalDamage;
            double statusAppliedPerHit = proportionOfTotalDamage * currAttack.statusChance;
            double statusAppliedPerShot = statusAppliedPerHit * multishotValue;
            double statusAppliedPerSecond = statusAppliedPerShot * effectiveFireRate;

            // std::cout << "I got here too!\n";

            // multiply by status duration
            double averageStatusCount = statusAppliedPerSecond * ((currEnemy.getStatusDurations()).at(i) * moddedWeapon.statusDuration);
            // round down if above cap
            double finalStatusCount = 0;
            if (averageStatusCount >= (currEnemy.getStatusCaps()).at(i))
            {
                finalStatusCount = (currEnemy.getStatusCaps()).at(i);
            } else
            {
                finalStatusCount = averageStatusCount;
            }

            if (finalStatusCount >= 0)
            {
                distinctStatusCount++;
            }

            (*currStatusCounts)[i] = finalStatusCount;
        }
    }

    double baseDMGModValue = (1 + (moddedWeapon.baseDamageModifier / 100));
    double gunCOModValue = 0;
    if (currAttack.shotType == "Hit-Scan")
    {
        // normal addative gunCO;
        baseDMGModValue += ((moddedWeapon.gunCOModifier / 100) * distinctStatusCount);
    }
    else if (currAttack.shotType == "Projectile")
    {
        if (moddedWeapon.className == "Bow")
        {
            // gunCO apples to uncharged shot, so half the bonus (charged shot is 2x uncharged, this should balance it)
            gunCOModValue += ((moddedWeapon.gunCOModifier / 100) / 2);
        }
        else
        {
            gunCOModValue += (moddedWeapon.gunCOModifier / 100);
        }
    }

    if (gunCOModValue != 0)
    {
        totalDamage = 0;
        for (auto& damageType : currAttack.damage)
        {
            damageType = damageType * (1 + (gunCOModValue * distinctStatusCount));
            totalDamage += damageType;
            // if (damageType.second != 0)
            // {
            //     std::cout << "The " << damageType.first << " damage is: " << damageType.second << std::endl;
            // }
        }
    }

    double statusAndModdedCritChance = ((currAttack.critChance) + currEnemy.getAddedCritChance());
    double statusAndModdedCritMultiplier = (currAttack.critMultiplier - 1.0f + currEnemy.getAddedCritDamage());

    double normalShot = totalDamage * (1 + (std::floor(statusAndModdedCritChance) * (statusAndModdedCritMultiplier)));
    double criticalShot = totalDamage * (1 + (std::ceil(statusAndModdedCritChance) * (statusAndModdedCritMultiplier)));
    double averageShot = totalDamage * ((1 + (statusAndModdedCritChance) * (statusAndModdedCritMultiplier)));

    double averageSingleShot = (totalDamage * (1 + ((currAttack.critChance) * (currAttack.critMultiplier - 1))));
        // std::cout << "I got here three!\n";
    // std::cout << "totalDamage is: " << totalDamage << " crit chance is: " << currAttack.critChance << " crit damage is: " << currAttack.critMultiplier << " and avg single shot dmg is: " << averageSingleShot << std::endl;


    // Starting Values
    double averageBurstDPS = 0;
    double numberOfShotsPerMag = 0;
    double averageSustainedDPS = 0;
    double ammoCostPerShot = 1;
    double percentOfTimeShooting = 0;

    // Gun DPS
    if (weaponType == "Primary" || weaponType == "Secondary")
    {
        if (currAttack.ammoCost != 0)
        {
            ammoCostPerShot = currAttack.ammoCost;
        }

        // avg burst dps (held but no reloads)
        // apply viral + corrosive to damage now too
        averageBurstDPS = averageShot * effectiveFireRate;

        // used to calculate time reloading
        if (moddedWeapon.magazineCapacity != 0)
        {
            numberOfShotsPerMag = moddedWeapon.magazineCapacity / ammoCostPerShot;
            percentOfTimeShooting = numberOfShotsPerMag / ((effectiveFireRate * moddedWeapon.reloadSpeed) + numberOfShotsPerMag);
        }
        else
        {
            // infinite magazine, so always shooting.
            numberOfShotsPerMag = INFINITY;
            percentOfTimeShooting = 1;
        }

        // percent of time shooting vs reloading
        // std::cout << "Percent of time shooting: " << percentOfTimeShooting << std::endl;
        // std::cout << "Number of shots per mag: " << numberOfShotsPerMag << std::endl;
        // std::cout << "Effective fire rate: " << effectiveFireRate << std::endl;
        // std::cout << "Ammo cost per shot: " << ammoCostPerShot << std::endl;
        // std::cout << "Magazine Capacity: " << moddedWeapon.magazineCapacity << std::endl;
        // std::cout << "Reload speed: " << moddedWeapon.reloadSpeed << std::endl;
        // Avg sustained dps
        averageSustainedDPS = averageBurstDPS * percentOfTimeShooting;
    }
    // Melee DPS
    else if (weaponType == "Melee")
    {
        // Going to have to add a slot for combo mods to choose from for the melee weapons, then i need to update this
        // TODO: update this calculation once combo mod can be chosen
        averageSustainedDPS = (averageShot * currAttack.fireRate / moddedWeapon.comboDuration);
    }
    return { averageSingleShot, averageBurstDPS, averageSustainedDPS };
}





struct weaponType {
    int weaponTypeID;
    std::string name;
    int parentTypeID;
};

// returns vector of strings of all compatible mod types
std::vector<std::string> getValidModTypes(std::string& weaponTypeName, std::vector<weaponType>& weaponTypeTree)
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
    // std::cout << "I at least ran the main function\n";
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
        else
        {
            std::cout << "Invalid! Please check capitalization and enter again" << std::endl;
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


    nlohmann::json currentChosenWeapon = selectedWeaponType[weaponName];


    // filter to only check valid mod options
    int currentModIndex = 0;
    std::string currentWeaponType = currentChosenWeapon["Class"];

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
        if (currMod->pruneThis == true)
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

    // Parse weapon into weapon dictionary      -       so I can make it do every weapon later
    std::vector<Weapon> weaponList = {};
    // for (auto& [weaponName, weaponData] : selectedWeaponType.items())
    // {
    std::vector<attackData> attackList = {};
    for (auto& currentAttack : currentChosenWeapon["Attacks"])
    {
        // for (auto& [damageType, damageAmount] : currentAttack.value("Damage", std::unordered_map<std::string, double>{}))
        // {
        //     std::cout << damageType << ": " << damageAmount << std::endl;
        // }
        attackList.push_back(
            attackData(
                currentAttack.value("AttackIndex", 0.0),       //  attackIndex
                currentAttack.value("AttackName", ""),          //  attackName
                currentAttack.value("ShotType", ""),            //  shotType
                currentAttack.value("CritChance", -999.0),        //  critChance
                currentAttack.value("CritMultiplier", -999.0),    //  critMultiplier
                currentAttack.value("StatusChance", -999.0),      //  statusChance
                currentAttack.value("FireRate", -999.0),          //  fireRate
                currentAttack.value("AmmoCost", 1.0),          //  ammoCost
                currentAttack.value("Multishot", 1.0),         //  multishot
                currentAttack.value("BurstCount", 1.0),        //  burstCount
                currentAttack.value("BurstDelay", 0.0),        //  burstDelay
                currentAttack.value("Trigger", ""),             //  triggerType
                currentAttack.value("ForcedProcs", std::vector<std::string>{}),    //  forcedProcs
                currentAttack.value("Damage", std::unordered_map<std::string, double>{})    //  damage
            )
        );
    }
    weaponList.push_back(Weapon(
            currentChosenWeapon.value("Name", ""),                //  name
            currentChosenWeapon.value("Class", ""),               //  className
            currentChosenWeapon.value("Family", ""),              //  weaponFamily
            currentChosenWeapon.value("Trigger", ""),             //  triggerType
            currentChosenWeapon.value("Magazine", 0.0),          //  magazineCapacity
            currentChosenWeapon.value("Reload", 0.0),            //  reloadSpeed
            currentChosenWeapon.value("Disposition", 0.0),       //  rivenDisposition
            currentChosenWeapon.value("Spool", 0.0),             //  spoolSpeed
            currentChosenWeapon.value("ReloadDelay", 0.0),       //  reloadDelay     -   charge weapons
            currentChosenWeapon.value("ReloadRate", 0.0),        //  reloadRate      -   charge weapons
            currentChosenWeapon.value("ComboDur", 0.0),          //  comboDuration
            currentChosenWeapon.value("HeavyAttack", 0.0),       //  heavyAttackDamage
            currentChosenWeapon.value("CompatibilityTags", std::vector<std::string>{}),   //  compatibilityTags
            currentChosenWeapon.value("DefaultUpgrades", std::vector<std::string>{}),     //  innateUpgrades
            attackList                                      //  attackList
        ));
    // }

    

    //  outer layer is for each attack, inner layer is for each type of DPS, then the innermost is the list of mods
    std::vector<std::vector<std::vector<std::string>>> optimalModChoices = {};
    //  outer layer is for each attack, inner layer is for each type of DPS
    std::vector<std::vector<double>> optimalStats = {};
    for (int i = 0; i < weaponList.at(0).attackList.size(); i++)
    {
        std::vector<std::string> singleShotMods = {};
        std::vector<std::string> burstDPSMods = {};
        std::vector<std::string> sustainedDPSMods = {};
        // first vector is a list of the best mods for single shot dps, second vector is a list of the best mods for burst dps, third vector is a list of the bestmods for sustained dps
        std::vector<std::vector<std::string>> attacksModLayouts = {singleShotMods, burstDPSMods, sustainedDPSMods};
        optimalModChoices.push_back(attacksModLayouts);

        // first entry is single shot average damage, second entry is burst dps, and third entry is sustained dps
        std::vector<double> attacksStats = {0, 0, 0};
        optimalStats.push_back(attacksStats);
    }



    // ---------------------------------------- BASE MOD CONFIG ----------------------------------------
    weaponModConfig currentModConfig = weaponModConfig();
    // ---------------------------------------- BASE ENEMY INFO ----------------------------------------
    Enemy currEnemy = Enemy();
    std::array<double, 14> baseStatusCounts = {0};

    unsigned long long totalCalculations = 1;
    for (unsigned long long k = 1; k <= 8; k++)
    {
        totalCalculations = totalCalculations * (validMods.size() - 8 + k) / k;
    }
    unsigned long long completedCalculations = 0;


    for (auto& currentWeapon : weaponList)
    {
        // TODO: figure out how to make this be while there are still mod combinations left
        // nested for loops, each starting at 1 higher index, each ending 1 index earlier from the end of valid mods vector
        // then also make a loop that tries each permutation of elemental mods to try their configs
        for (int modSlotOneIndex = 0; modSlotOneIndex < validMods.size() - 7; modSlotOneIndex++)
        {
            // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
            bool skipModSlotOne = false;
            for (std::string& incompatibleMod : validMods[modSlotOneIndex].incompatibleMods)
            {
                if (std::find(currentModConfig.currentMods.begin(), currentModConfig.currentMods.end(), incompatibleMod) != currentModConfig.currentMods.end())
                {   // if there is an incompatible mod already in the current mod config
                    skipModSlotOne = true;
                }
            }
            if (skipModSlotOne)
            {
                continue;   //  skip to next mod in this slot
            }

            currentModConfig.addMod(validMods[modSlotOneIndex], modSlotOneIndex);
            // Start lower level loop
            for (int modSlotTwoIndex = modSlotOneIndex + 1; modSlotTwoIndex < validMods.size() - 6; modSlotTwoIndex++)
            {
                // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
                bool skipModSlotTwo = false;
                for (std::string& incompatibleMod : validMods[modSlotTwoIndex].incompatibleMods)
                {
                    if (std::find(currentModConfig.currentMods.begin(), currentModConfig.currentMods.end(), incompatibleMod) != currentModConfig.currentMods.end())
                    {   // if there is an incompatible mod already in the current mod config
                        skipModSlotTwo = true;
                    }
                }
                if (skipModSlotTwo)
                {
                    continue;   //  skip to next mod in this slot
                }

                currentModConfig.addMod(validMods[modSlotTwoIndex], modSlotTwoIndex);
                // Start lower level loop
                for (int modSlotThreeIndex = modSlotTwoIndex + 1; modSlotThreeIndex < validMods.size() - 5; modSlotThreeIndex++)
                {
                    // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
                    bool skipModSlotThree = false;
                    for (std::string& incompatibleMod : validMods[modSlotThreeIndex].incompatibleMods)
                    {
                        if (std::find(currentModConfig.currentMods.begin(), currentModConfig.currentMods.end(), incompatibleMod) != currentModConfig.currentMods.end())
                        {   // if there is an incompatible mod already in the current mod config
                            skipModSlotThree = true;
                        }
                    }
                    if (skipModSlotThree)
                    {
                        continue;   //  skip to next mod in this slot
                    }

                    currentModConfig.addMod(validMods[modSlotThreeIndex], modSlotThreeIndex);
                    // Start lower level loop
                    for (int modSlotFourIndex = modSlotThreeIndex + 1; modSlotFourIndex < validMods.size() - 4; modSlotFourIndex++)
                    {
                        // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
                        bool skipModSlotFour = false;
                        for (std::string& incompatibleMod : validMods[modSlotFourIndex].incompatibleMods)
                        {
                            if (std::find(currentModConfig.currentMods.begin(), currentModConfig.currentMods.end(), incompatibleMod) != currentModConfig.currentMods.end())
                            {   // if there is an incompatible mod already in the current mod config
                                skipModSlotFour = true;
                            }
                        }
                        if (skipModSlotFour)
                        {
                            continue;   //  skip to next mod in this slot
                        }

                        currentModConfig.addMod(validMods[modSlotFourIndex], modSlotFourIndex);
                        // Start lower level loop
                        for (int modSlotFiveIndex = modSlotFourIndex + 1; modSlotFiveIndex < validMods.size() - 3; modSlotFiveIndex++)
                        {
                            // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
                            bool skipModSlotFive = false;
                            for (std::string& incompatibleMod : validMods[modSlotFiveIndex].incompatibleMods)
                            {
                                if (std::find(currentModConfig.currentMods.begin(), currentModConfig.currentMods.end(), incompatibleMod) != currentModConfig.currentMods.end())
                                {   // if there is an incompatible mod already in the current mod config
                                    skipModSlotFive = true;
                                }
                            }
                            if (skipModSlotFive)
                            {
                                continue;   //  skip to next mod in this slot
                            }

                            currentModConfig.addMod(validMods[modSlotFiveIndex], modSlotFiveIndex);
                            // Start lower level loop
                            for (int modSlotSixIndex = modSlotFiveIndex + 1; modSlotSixIndex < validMods.size() - 2; modSlotSixIndex++)
                            {
                                // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
                                bool skipModSlotSix = false;
                                for (std::string& incompatibleMod : validMods[modSlotSixIndex].incompatibleMods)
                                {
                                    if (std::find(currentModConfig.currentMods.begin(), currentModConfig.currentMods.end(), incompatibleMod) != currentModConfig.currentMods.end())
                                    {   // if there is an incompatible mod already in the current mod config
                                        skipModSlotSix = true;
                                    }
                                }
                                if (skipModSlotSix)
                                {
                                    continue;   //  skip to next mod in this slot
                                }

                                currentModConfig.addMod(validMods[modSlotSixIndex], modSlotSixIndex);
                                // Start lower level loop
                                for (int modSlotSevenIndex = modSlotSixIndex + 1; modSlotSevenIndex < validMods.size() - 1; modSlotSevenIndex++)
                                {
                                    // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
                                    bool skipModSlotSeven = false;
                                    for (std::string& incompatibleMod : validMods[modSlotSevenIndex].incompatibleMods)
                                    {
                                        if (std::find(currentModConfig.currentMods.begin(), currentModConfig.currentMods.end(), incompatibleMod) != currentModConfig.currentMods.end())
                                        {   // if there is an incompatible mod already in the current mod config
                                            skipModSlotSeven = true;
                                        }
                                    }
                                    if (skipModSlotSeven)
                                    {
                                        continue;   //  skip to next mod in this slot
                                    }

                                    currentModConfig.addMod(validMods[modSlotSevenIndex], modSlotSevenIndex);
                                    // Start lower level loop
                                    for (int modSlotEightIndex = modSlotSevenIndex + 1; modSlotEightIndex < validMods.size(); modSlotEightIndex++)
                                    {
                                        // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
                                        bool skipModSlotEight = false;
                                        for (std::string& incompatibleMod : validMods[modSlotEightIndex].incompatibleMods)
                                        {
                                            if (std::find(currentModConfig.currentMods.begin(), currentModConfig.currentMods.end(), incompatibleMod) != currentModConfig.currentMods.end())
                                            {   // if there is an incompatible mod already in the current mod config
                                                skipModSlotEight = true;
                                            }
                                        }
                                        if (skipModSlotEight)
                                        {
                                            continue;   //  skip to next mod in this slot
                                        }
                        
                                        currentModConfig.addMod(validMods[modSlotEightIndex], modSlotEightIndex);

                                        // auto weaponBeforeMod = currentWeapon;

                                        // ---------------------------------------- APPLY MOD CONFIG ----------------------------------------
                                        currentWeapon.applyModConfig(currentModConfig);
                                        
                                        // ---------------------------------------- FOR EACH ATTACK ----------------------------------------
                                        for (auto& currentAttack : currentWeapon.attackList)
                                        {                    
                                            // ---------------------------------------- RESET ENEMY INFO ----------------------------------------
                                            currEnemy.setStatusCounts(baseStatusCounts);

                                            // ---------------------------------------- PRINT CONFIG STATS ----------------------------------------
                                            // for (auto& [k, v] : currentModConfig.weaponModifiers)
                                            // {
                                            //     if (v != 0)
                                            //     {
                                            //         std::cout << k << ": " << v << std::endl;
                                            //     }
                                            // }
                                            // for (auto& [k, v] : currentModConfig.statusTypeModifiers)
                                            // {
                                            //     if (v != 0)
                                            //     {
                                            //         std::cout << k << ": " << v << std::endl;
                                            //     }
                                            // }

                                            // ---------------------------------------- CALCULATE DAMAGE ----------------------------------------
                                            auto [tempAverageShot, tempAverageBurstDPS, tempAverageSustainedDPS] = calculateDPSValues(currentWeapon, currentAttack, weaponGeneralClass, currEnemy);
                                                    // std::cout << "I calculated the DPS!\n";
                                            if (tempAverageShot > optimalStats.at(currentAttack.attackIndex - 1).at(0))
                                            {
                                                optimalStats[currentAttack.attackIndex - 1][0] = tempAverageShot;
                                                optimalModChoices[currentAttack.attackIndex - 1][0] = currentModConfig.currentMods;
                                            }
                                            if (tempAverageBurstDPS > optimalStats.at(currentAttack.attackIndex - 1).at(1))
                                            {
                                                optimalStats[currentAttack.attackIndex - 1][1] = tempAverageBurstDPS;
                                                optimalModChoices[currentAttack.attackIndex - 1][1] = currentModConfig.currentMods;
                                            }
                                            if (tempAverageSustainedDPS > optimalStats.at(currentAttack.attackIndex - 1).at(2))
                                            {
                                                optimalStats[currentAttack.attackIndex - 1][2] = tempAverageSustainedDPS;
                                                optimalModChoices[currentAttack.attackIndex - 1][2] = currentModConfig.currentMods;
                                            }
                                        }

                                        if ((++completedCalculations % 1000000) == 0)
                                        {   //  print remaining calculation number every 100k completed calcs
                                            std::cout << "Remaining: " << (totalCalculations - completedCalculations) << '\n';
                                        }

                                        // ---------------------------------------- REMOVE MOD CONFIG --------------------------------------
                                        currentWeapon.removeModConfig(currentModConfig);

                                        currentModConfig.removeMod(validMods[modSlotEightIndex], modSlotEightIndex);    //  Last entry should always be this mod as it is about to move to a lower level
                                        // Move up a loop
                                        /*
                                        //  This block checks to see if the mod was different before and after mods were applied (also uncomment the above dfeinition of weaponBeforeMod)
                                        auto weaponAfterModRemoved = currentWeapon;

                                        if ((weaponAfterModRemoved.magazineCapacity - weaponBeforeMod.magazineCapacity) != 0)
                                        {
                                            std::cout << "Magazine Capacity" << ": " << weaponBeforeMod.magazineCapacity << " -> " << weaponAfterModRemoved.magazineCapacity << "\n";
                                        }
                                        if ((weaponAfterModRemoved.reloadSpeed - weaponBeforeMod.reloadSpeed) != 0)
                                        {
                                            std::cout << "Reload Speed" << ": " << weaponBeforeMod.reloadSpeed << " -> " << weaponAfterModRemoved.reloadSpeed << "\n";
                                        }
                                        if ((weaponAfterModRemoved.baseDamageModifier - weaponBeforeMod.baseDamageModifier) != 0)
                                        {
                                            std::cout << "Base Damage Mod" << ": " << weaponBeforeMod.baseDamageModifier << " -> " << weaponAfterModRemoved.baseDamageModifier << "\n";
                                        }
                                        if ((weaponAfterModRemoved.attackList[0].fireRate - weaponBeforeMod.attackList[0].fireRate) != 0)
                                        {
                                            std::cout << "Fire Rate" << ": " << weaponBeforeMod.attackList[0].fireRate << " -> " << weaponAfterModRemoved.attackList[0].fireRate << "\n";
                                        }
                                        if ((weaponAfterModRemoved.attackList[0].critChance - weaponBeforeMod.attackList[0].critChance) != 0)
                                        {
                                            std::cout << "Crit Chance" << ": " << weaponBeforeMod.attackList[0].critChance << " -> " << weaponAfterModRemoved.attackList[0].critChance << "\n";
                                        }
                                        if ((weaponAfterModRemoved.attackList[0].critMultiplier - weaponBeforeMod.attackList[0].critMultiplier) != 0)
                                        {
                                            std::cout << "Crit Multiplier" << ": " << weaponBeforeMod.attackList[0].critMultiplier << " -> " << weaponAfterModRemoved.attackList[0].critMultiplier << "\n";
                                        }
                                        if ((weaponAfterModRemoved.attackList[0].multishot - weaponBeforeMod.attackList[0].multishot) != 0)
                                        {
                                            std::cout << "Multishot" << ": " << weaponBeforeMod.attackList[0].multishot << " -> " << weaponAfterModRemoved.attackList[0].multishot << "\n";
                                        }
                                        if (currentModConfig.currentMods.size() != 1)
                                        {
                                            std::cout << "Applying " << currentModConfig.currentMods.size() << " mods" << std::endl;
                                        }
                                        for (auto& currAttack : weaponBeforeMod.attackList)
                                        {
                                            for (auto& [k, v] : currAttack.damage)
                                            {
                                                if ((v - weaponAfterModRemoved.attackList[currAttack.attackIndex - 1].damage[k]) != 0)
                                                {
                                                    std::cout << k << ": " << currAttack.damage[k] << " -> " << v << std::endl;
                                                }
                                            }
                                        }
                                            */
                                    }
                                    currentModConfig.removeMod(validMods[modSlotSevenIndex], modSlotSevenIndex);    //  Last entry should always be this mod as it is about to move to a lower level
                                    // Move up a loop
                                }
                                currentModConfig.removeMod(validMods[modSlotSixIndex], modSlotSixIndex);    //  Last entry should always be this mod as it is about to move to a lower level
                                // Move up a loop
                            }
                            currentModConfig.removeMod(validMods[modSlotFiveIndex], modSlotFiveIndex);    //  Last entry should always be this mod as it is about to move to a lower level
                            // Move up a loop
                        }
                        currentModConfig.removeMod(validMods[modSlotFourIndex], modSlotFourIndex);    //  Last entry should always be this mod as it is about to move to a lower level
                        // Move up a loop
                    }
                    currentModConfig.removeMod(validMods[modSlotThreeIndex], modSlotThreeIndex);    //  Last entry should always be this mod as it is about to move to a lower level
                    // Move up a loop
                }
                currentModConfig.removeMod(validMods[modSlotTwoIndex], modSlotTwoIndex);    //  Last entry should always be this mod as it is about to move to a lower level
                // Move up a loop
            }
            currentModConfig.removeMod(validMods[modSlotOneIndex], modSlotOneIndex);    //  Last entry should always be this mod as it is about to move to a lower level
            // Move up a loop
        }
        for (int i = 0; i < currentWeapon.attackList.size(); i++)
        {
            std::cout << "For the: " << currentWeapon.name << "'s " << currentWeapon.attackList[i].attackName << " attack, the calculated best stats are as follows: Average shot: " << optimalStats.at(i).at(0) << ", Average burst DPS: " << optimalStats.at(i).at(1) << ", Average Sustained DPS: " << optimalStats.at(i).at(2) << std::endl;
            std::cout << "Using the following mods for single shot: \n";
            for (int j = 0; j < 8; j++)
            {
                std::cout << optimalModChoices.at(i).at(0).at(j) << "\n";
            }
            std::cout << "Using the following mods for burst DPS: \n";
            for (int j = 0; j < 8; j++)
            {
                std::cout << optimalModChoices.at(i).at(1).at(j) << "\n";
            }
            std::cout << "Using the following mods for sustained DPS: \n";
            for (int j = 0; j < 8; j++)
            {
                std::cout << optimalModChoices.at(i).at(2).at(j) << "\n";
            }
        }
    }
    
    // waits for user to hit enter to leave program
    std::cin.get();
    return 0;
}


