#include "nlohmann/json.hpp"
#include "Classes/Enemy.hpp"
#include <fstream>
#include <iostream>
#include <format>
#include <string>
#include <cmath>
// json loading thanks to nlohmann, further info in nlohmann/json.hpp

using json = nlohmann::json;


json loadJsonFile(std::string fileName)
{
    std::cout << ("Loading " + fileName + "\n");

    std::ifstream jsonFile(fileName);
    
    std::cout << "Parsing data\n";

    json parsedJson;
    try {
        parsedJson = json::parse(jsonFile);
        jsonFile.close();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error parsing " << fileName << "   :   " << e.what() << std::endl;
    }
    std::cout << "Parsed without errors (at least I hope)\n";
    return parsedJson;
}




    // TODO: implement changing this based on chosen stance for melee weapons, when melee weapons are implemented
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
        averageSustainedDPS = (averageShot * static_cast<double>(currAttack["FireRate"]) / tempBaseComboLength);
    }
    return { averageShot, averageBurstDPS, averageSustainedDPS };
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
    if (weaponGeneralClass == "Primary") {
        selectedWeaponType = wikiPrimaryWeaponData;
    } else if (weaponGeneralClass == "Secondary") {
        selectedWeaponType = wikiSecondaryWeaponData;
    } else if (weaponGeneralClass == "Melee") {
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

    // while (currentModIndex < upgradesData["ExportUpgrades"].size())
    // {
    //     nlohmann::json currentMod = upgradesData["ExportUpgrades"][currentModIndex];
    //     if (currentMod["type"] != "PRIMARY" &&
    //     currentMod["type"] != "SECONDARY" &&
    //     currentMod["type"] != "MELEE")
    //     { // skip mod if not primary/secondary/melee
    //         currentModIndex++;
    //         continue;
    //     }
    //     if (currentWeaponType == "LongGuns") // primary weapon
    //     {
    //         if (currentMod["type"] == "PRIMARY") // primary mod
    //         {

    //         }
    //         else
    //         { // skip
    //             continue;
    //         }
    //     }
    //     else if (currentWeaponType == "Pistols") // secondary weapon
    //     {
    //         if (currentMod["type"] == "SECONDARY") // secondary mod
    //         {

    //         }
    //         else
    //         { // skip
    //             continue;
    //         }
    //     }
    //     else if (currentWeaponType == "Melee") // melee weapon
    //     {
    //         if (currentMod["type"] == "MELEE") // melee mod
    //         {

    //         }
    //         else
    //         { // skip
    //             continue;
    //         }
    //     }
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
                averageShot = tempAverageShot; // save mod config too
            }
            if (tempAverageBurstDPS > averageBurstDPS)
            {
                averageBurstDPS = tempAverageBurstDPS; // save mod config too
            }
            if (tempAverageSustainedDPS > averageSustainedDPS)
            {
                averageSustainedDPS = tempAverageSustainedDPS; // save mod config too
            }
            std::cout << "For the: " << currentWeapon["Name"] << "'s attack number " << attackIndex << " the stats are as follows: Average shot: " << tempAverageShot << ", Average burst DPS: " << tempAverageBurstDPS << ", Average Sustained DPS: " << tempAverageSustainedDPS << std::endl;
        }
        attackIndex++;
    }
    return 0;
}


