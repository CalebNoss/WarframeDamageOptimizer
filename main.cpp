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




    // TODO: implement correct charge time data, for now default to 0.6 seconds, it seemed like it was close to the average. This means extremely skewed data for any and all charge weapons
    float tempChargeVariable = 0.6;
    // TODO: implement correct burst count & delay data, for now default to 3 projectiles and 0.09, they seem like they are close to the average. This means extremely skewed data for any and all burst weapons
    float tempBurstCountVariable = 3;
    float tempBurstDelayVariable = 0.09;
    // TODO: implement Auto Burst fire rate calculation, only 2 weapons so skipped for early development

    // TODO: implement changing this based on chosen stance for melee weapons, when melee weapons are implemented
    float tempBaseComboLength = 3;





std::tuple<float, float, float> calculateDPSValues(nlohmann::json moddedWeapon, int attackIndex)
{
    // ---------------------------------------- CALCULATE DAMAGE ----------------------------------------
    
    // Calculate totalDamage
    float totalDamage = 0;
    for (const auto& damageType : (moddedWeapon["Attacks"][attackIndex]["Damage"].items()))
    {
        totalDamage += damageType.value().get<double>();
    }
    
    // Average Shot/Hit of normal/critical/average hit
    float normalShot = static_cast<float>(moddedWeapon["Attacks"][attackIndex][""]) * (1 + (std::floor(static_cast<float>(moddedWeapon["criticalChance"])) * (static_cast<float>(moddedWeapon["criticalMultiplier"]) - 1.0f)));
    float criticalShot = static_cast<float>(moddedWeapon["totalDamage"]) * (1 + (std::ceil(static_cast<float>(moddedWeapon["criticalChance"])) * (static_cast<float>(moddedWeapon["criticalMultiplier"]) - 1.0f)));
    float averageShot = static_cast<float>(moddedWeapon["totalDamage"]) * ((1 + static_cast<float>(moddedWeapon["criticalChance"])) * (static_cast<float>(moddedWeapon["criticalMultiplier"]) - 1));

    // Starting Values
    float averageBurstDPS = 0;
    float effectiveFireRate = 0;
    float numberOfShotsPerMag = 0;
    float averageSustainedDPS = 0;
    float ammoCostPerShot = 1;
    float percentOfTimeShooting = 0;

    // Gun DPS
    if (moddedWeapon["Class"] == "Pistols" || moddedWeapon["Class"] == "LongGuns")
    {
        // calculate effective fire rate
        if (moddedWeapon["trigger"] == "AUTO" ||
        moddedWeapon["trigger"] == "SEMI" ||
        moddedWeapon["trigger"] == "DUPLEX" ||
        moddedWeapon["trigger"] == "HELD")
        {
            effectiveFireRate = moddedWeapon["fireRate"];
            if (moddedWeapon["trigger"] == "HELD")
            {
                ammoCostPerShot = 0.5; // not always true, but for early it works //TODO: update this
            }
        }
        else if (moddedWeapon["trigger"] == "CHARGE")
        {
            effectiveFireRate = (1 / (tempChargeVariable + (1 / static_cast<float>(moddedWeapon["fireRate"]))));
        }
        else if (moddedWeapon["trigger"] == "BURST")
        {
            effectiveFireRate = (tempBurstCountVariable / ((1 / static_cast<float>(moddedWeapon["fireRate"])) + ((tempBurstCountVariable - 1) * tempBurstDelayVariable)));
        }

        // avg burst dps (held but no reloads)
        averageBurstDPS = averageShot * effectiveFireRate;



        // used to calculate time reloading
        if (moddedWeapon.contains("Magazine"))
        {
            numberOfShotsPerMag = static_cast<float>(moddedWeapon["Magazine"]) / ammoCostPerShot;
            percentOfTimeShooting = numberOfShotsPerMag / ((effectiveFireRate * static_cast<float>(moddedWeapon["Reload"])) + numberOfShotsPerMag);
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
    else if (moddedWeapon["Class"] == "Melee")
    {
        averageSustainedDPS = (averageShot * static_cast<float>(moddedWeapon["fireRate"]) / tempBaseComboLength);
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

    float averageShot = 0;
    float averageBurstDPS = 0;
    float averageSustainedDPS = 0;
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
        for (int i = 0; i <= 1; i++)
        {
            nlohmann::json moddedWeapon = currentWeapon;
            std::vector<std::string> currentModConfig = {};
            // ---------------------------------------- CHOOSE MODS HERE ----------------------------------------

            // will edit modded entry based on mods here? maybe? maybe try each mod then pass to damage calc?
            // ---------------------------------------- CHOOSE MODS HERE ----------------------------------------


            // ---------------------------------------- BASE ENEMY INFO ----------------------------------------
            Enemy currEnemy = Enemy();



            
            // ---------------------------------------- CALCULATE DAMAGE ----------------------------------------

            auto [tempAverageShot, tempAverageBurstDPS, tempAverageSustainedDPS] = calculateDPSValues(moddedWeapon, (attackIndex - 1));
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
        }
        std::cout << "For the: " << currentWeapon["Name"] << "'s attack number " << attackIndex << " the stats are as follows: Average shot: " << averageShot << ", Average burst DPS: " << averageBurstDPS << ", Average Sustained DPS: " << averageSustainedDPS << std::endl;
        attackIndex++;
    }
    return 0;
}