#pragma once
#include "nlohmann/json.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "Classes/Enemy.hpp"
#include "Classes/weaponModConfig.hpp"
#include "Classes/Weapon.hpp"
#include <fstream>
#include <iostream>
#include <format>
#include <string>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <d3d11.h>
#include <tchar.h>
#include <omp.h>
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


std::array<double, 14> baseStatusDurations = {
    6,          // 0  = Impact = 6
    10,         // 1  = Puncture = 10
    6,          // 2  = Slash = 6
    6,          // 3  = Heat = 6
    6,          // 4  = Cold = 6
    6,          // 5  = Electricity = 6
    6,          // 6  = Toxin = 6
    1.5,        // 7  = Blast = 1.5
    8,          // 8  = Corrosive = 8
    6,          // 9 = Gas = 6
    6,          // 10 = Magnetic = 6
    12,         // 11 = Radiation = 12
    6,          // 12 = Viral = 6
    8           // 13 = Tau = 8
};
std::array<double, 14> statusCaps = {
            5,              // 0  = Impact = 5
            5,              // 1  = Puncture = 5
            1061109567,     // 2  = Slash = 1061109567
            1061109567,     // 3  = Heat = 1061109567
            10,             // 4  = Cold = 10
            1061109567,     // 5  = Electricity = 1061109567
            1061109567,     // 6  = Toxin = 1061109567
            10,             // 7  = Blast = 10
            10,             // 8  = Corrosive = 10
            10,             // 9 = Gas = 10
            10,             // 10 = Magnetic = 10
            10,             // 11 = Radiation = 10
            10,             // 12 = Viral = 10
            10              // 13 = Tau = 10}
        };
std::array<double, 5> dotModifier = {
    0.35,
    0.5,
    0.5,
    0.5,
    0.5
};

std::tuple<double, double, double> calculateDPSValuesRanged(Weapon& moddedWeapon, attackData& currAttack, int weaponTypeIndex, Enemy& currEnemy, weaponModConfig& currModConfig)
{
    // ---------------------------------------- CALCULATE DAMAGE ----------------------------------------
    // std::cout << "Starting damage calcs\n";

    // Initialize variables and get to local vars
    int distinctStatusCount = 0;
    double totalDamage = 0;
    double magazineCapacityModifier     = 1 + currModConfig.weaponModifiers[2];
    double criticalChanceModifier       = 1 + currModConfig.weaponModifiers[3];
    double criticalDamageModifier       = 1 + currModConfig.weaponModifiers[4];
    double baseDMGModValue              = 1 + currModConfig.weaponModifiers[5];
    double statusDurationModifier       = 1 + currModConfig.weaponModifiers[6];
    double statusChanceModifier         = 1 + currModConfig.weaponModifiers[7];
    double statusDamageModifier         = 1 + currModConfig.weaponModifiers[8];
    double reloadSpeedModifier          = 1 + currModConfig.weaponModifiers[9];
    double gunCOModValue                = 1 + currModConfig.weaponModifiers[10];


    double multishotMultiplier = currModConfig.locksMultishot ? 1.0 : (1 + currModConfig.weaponModifiers[1]);
    double multishotValue = currAttack.multishot * multishotMultiplier;
    
    double fireRateModifier = currModConfig.locksFireRate ? 1.0 : (1 + currModConfig.weaponModifiers[0]);
    double moddedFireRate = currAttack.fireRate * (fireRateModifier);

    uint16_t damageTypesMask = currAttack.damageTypesMask ^ currModConfig.getStatusTypeMask();

    std::array<double, 14> damageTypes = currAttack.damage;
    double totalBaseDamage             = currAttack.totalBaseDamage;
    const double impactDamageAmount = (currAttack.damage[0] * (1 + currModConfig.statusTypeModifiers[0]));   // Impact
    const double punctureDamageAmount = (currAttack.damage[1] * (1 + currModConfig.statusTypeModifiers[1]));   // Puncture
    const double slashDamageAmount = (currAttack.damage[2] * (1 + currModConfig.statusTypeModifiers[2]));   // Slash
    const double heatDamageAmount = damageTypes[3]    + (totalBaseDamage * currModConfig.statusTypeModifiers[3]);             // Heat
    const double coldDamageAmount = damageTypes[4]    + (totalBaseDamage * currModConfig.statusTypeModifiers[4]);             // Cold
    const double electricDamageAmount = damageTypes[5]    + (totalBaseDamage * currModConfig.statusTypeModifiers[5]);             // Electricity
    const double toxinDamageAmount = damageTypes[6]    + (totalBaseDamage * currModConfig.statusTypeModifiers[6]);             // Toxin
    const double blastDamageAmount = damageTypes[7]    + (totalBaseDamage * currModConfig.statusTypeModifiers[7]);             // Blast
    const double corrosiveDamageAmount = damageTypes[8]    + (totalBaseDamage * currModConfig.statusTypeModifiers[8]);             // Corrosive
    const double gasDamageAmount = damageTypes[9]    + (totalBaseDamage * currModConfig.statusTypeModifiers[9]);             // Gas
    const double magneticDamageAmount = damageTypes[10]   + (totalBaseDamage * currModConfig.statusTypeModifiers[10]);           // Magnetic
    const double radiationDamageAmount = damageTypes[11]   + (totalBaseDamage * currModConfig.statusTypeModifiers[11]);           // Radiation
    const double viralDamageAmount = damageTypes[12]   + (totalBaseDamage * currModConfig.statusTypeModifiers[12]);           // Viral
    const double tauDamageAmount = damageTypes[13];           // Tau


    double totalMiscDamage      = (impactDamageAmount + punctureDamageAmount) + (slashDamageAmount + tauDamageAmount);  // Impact | Puncture | Slash | Tau
    double totalBasicDamage     = (heatDamageAmount + coldDamageAmount) + (electricDamageAmount + toxinDamageAmount);    // Heat | Cold | Electric | Toxin
    double totalCombinedDamage  = (blastDamageAmount + corrosiveDamageAmount) + (gasDamageAmount + magneticDamageAmount) + (radiationDamageAmount + viralDamageAmount);   // Blast | Corrosive | Gas | Magnetic | Radiation | Viral
    totalDamage     =   totalMiscDamage + totalBasicDamage + totalCombinedDamage;


    damageTypes[0]  = impactDamageAmount;
    damageTypes[1]  = punctureDamageAmount;
    damageTypes[2]  = slashDamageAmount;
    damageTypes[3]  = heatDamageAmount;
    damageTypes[4]  = coldDamageAmount;
    damageTypes[5]  = electricDamageAmount;
    damageTypes[6]  = toxinDamageAmount;
    damageTypes[7]  = blastDamageAmount;
    damageTypes[8]  = corrosiveDamageAmount;
    damageTypes[9]  = gasDamageAmount;
    damageTypes[10] = magneticDamageAmount;
    damageTypes[11] = radiationDamageAmount;
    damageTypes[12] = viralDamageAmount;





    // std::cout<< "The total damage is: " << totalDamage << std::endl;

    // std::cout << "I summed total damage and got multishot value!\n";


    double effectiveFireRate = moddedFireRate;

    
    // std::cout << "I calculated the fire rate!\n";


    // Calculate status amounts on enemy
    std::array<double, 14>* currStatusCounts = currEnemy.getStatusCounts();
    
    // setup data so it isn't calculated every loop for optimizing performance
    double totalDamageInverse = 1 / totalDamage;
    double avgStatusCountConstants = (totalDamageInverse * currAttack.statusChance) * (multishotValue * effectiveFireRate) * (statusDurationModifier * statusChanceModifier); // status duration & chance mods at the end
    
    for (int i = 0; i < 14; i++)
    {
        if ((damageTypesMask & 1) == 1)  // skip if this damage doesn't exist
        {
            // use each types damage as a proportion of totalDamage to get damage distribution
            // multiply by status chance to get amount applied per hit
            // multiply by multishot to get amount applied per shot
            // multiply by effective fire rate to get procs/second
            // divide by time to expire or something to find amount per second on average considering expiration time
            // cap at max amount
            // add to enemy
            // update damage calculations ot take into account the CC, CD, etc. buffs
            // double proportionOfTotalDamage = currAttack.damage[i] / totalDamage;
            //  double statusAppliedPerHit = proportionOfTotalDamage * currAttack.statusChance;
            //  double statusAppliedPerShot = statusAppliedPerHit * multishotValue;
            //  double statusAppliedPerSecond = statusAppliedPerShot * effectiveFireRate;

            double averageStatusCount = damageTypes[i] * avgStatusCountConstants * baseStatusDurations[i];

            // round down if above cap
            double finalStatusCount = (std::min)(averageStatusCount, statusCaps[i]);

            distinctStatusCount += (finalStatusCount >= 1);

            (*currStatusCounts)[i] = finalStatusCount;
        }
        damageTypesMask >>= 1;  // move to the right one to look at the next status type
    }

    char shotTypeFirstChar = (currAttack.shotType.empty()) ? '1' : currAttack.shotType[0];
    double gunCOMultiplier = 0;
    if (shotTypeFirstChar == 'H')
    {
        // normal addative gunCO;
        baseDMGModValue += (gunCOModValue * distinctStatusCount);
    }
    else if (shotTypeFirstChar == 'P')
    {
        if (moddedWeapon.className[0] == 'B')
        {
            // gunCO apples to uncharged shot, so half the bonus (charged shot is 2x uncharged, this should balance it)
            gunCOModValue *= 0.5;
        }
        gunCOMultiplier += (gunCOModValue * distinctStatusCount);
    }


    // combined gunCO and damage mod multiplier
    double damageMultiplier = baseDMGModValue * (1 + (gunCOMultiplier * distinctStatusCount));

    // apply gunCO and base damage mods
    totalDamage = totalDamage * damageMultiplier;

    double moddedCritChance = currAttack.critChance * criticalChanceModifier;
    double moddedCritMultiplier = currAttack.critMultiplier * criticalDamageModifier;



    double statusAndModdedCritChance = (moddedCritChance + currEnemy.getAddedCritChance());
    double statusAndModdedCritMultiplier = (moddedCritMultiplier + currEnemy.getAddedCritDamage());

    double averageShot = totalDamage * ((1 + (statusAndModdedCritChance) * (statusAndModdedCritMultiplier)));

    double averageSingleShot = (totalDamage * (1 + (moddedCritChance * (moddedCritMultiplier - 1))));
        // std::cout << "I got here three!\n";
    // std::cout << "totalDamage is: " << totalDamage << " crit chance is: " << currAttack.critChance << " crit damage is: " << currAttack.critMultiplier << " and avg single shot dmg is: " << averageSingleShot << std::endl;

    double baseAvgDot = totalDamage * multishotValue * statusDamageModifier * 6 * statusDurationModifier;
    double avgSlashDot = baseAvgDot * dotModifier[0];
    double avgElectricityDot = baseAvgDot * dotModifier[1] * (1 + currModConfig.statusTypeModifiers[5]);
    double avgHeatDot = baseAvgDot * dotModifier[2] * (1 + currModConfig.statusTypeModifiers[3]);
    double avgToxinDot = baseAvgDot * dotModifier[3] * (1 + currModConfig.statusTypeModifiers[6]);
    double avgGasDot = baseAvgDot * dotModifier[4] * (1 + currModConfig.statusTypeModifiers[9]);

    double totalAvgDot = (avgSlashDot + avgElectricityDot + avgHeatDot + avgToxinDot + avgGasDot) * totalDamageInverse;
    double avgTotalAvgDot = totalAvgDot * currAttack.statusChance * statusChanceModifier * ((1 + (statusAndModdedCritChance) * (statusAndModdedCritMultiplier)));;

    // Starting Values
    double averageBurstDPS = avgTotalAvgDot;
    double numberOfShotsPerMag = 0;
    double averageSustainedDPS = avgTotalAvgDot;
    double ammoCostPerShotInverse = 1;
    double percentOfTimeShooting = 0;

    if (currAttack.ammoCost)  // anything but 0
    {
        ammoCostPerShotInverse = 1 / currAttack.ammoCost;
    }

    // avg burst dps (held but no reloads)
    // apply viral + corrosive to damage now too
    averageBurstDPS += averageShot * effectiveFireRate;

    // used to calculate time reloading
    if (moddedWeapon.magazineCapacity)  // anything but 0
    {
        numberOfShotsPerMag = (moddedWeapon.magazineCapacity * magazineCapacityModifier) * ammoCostPerShotInverse;
        percentOfTimeShooting = numberOfShotsPerMag / ((effectiveFireRate * moddedWeapon.reloadSpeed * reloadSpeedModifier) + numberOfShotsPerMag);
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
    averageSustainedDPS += averageBurstDPS * percentOfTimeShooting;
    return { averageSingleShot, averageBurstDPS, averageSustainedDPS };
}
std::tuple<double, double, double> calculateDPSValuesRangedCharge(Weapon& moddedWeapon, attackData& currAttack, int weaponTypeIndex, Enemy& currEnemy, weaponModConfig& currModConfig)
{
    // ---------------------------------------- CALCULATE DAMAGE ----------------------------------------
    // Initialize variables and get to local vars
    int distinctStatusCount = 0;
    double totalDamage = 0;
    double magazineCapacityModifier     = 1 + currModConfig.weaponModifiers[2];
    double criticalChanceModifier       = 1 + currModConfig.weaponModifiers[3];
    double criticalDamageModifier       = 1 + currModConfig.weaponModifiers[4];
    double baseDMGModValue              = 1 + currModConfig.weaponModifiers[5];
    double statusDurationModifier       = 1 + currModConfig.weaponModifiers[6];
    double statusChanceModifier         = 1 + currModConfig.weaponModifiers[7];
    double statusDamageModifier         = 1 + currModConfig.weaponModifiers[8];
    double reloadSpeedModifier          = 1 + currModConfig.weaponModifiers[9];
    double gunCOModValue                = 1 + currModConfig.weaponModifiers[10];


    double multishotMultiplier = currModConfig.locksMultishot ? 1.0 : (1 + currModConfig.weaponModifiers[1]);
    double multishotValue = currAttack.multishot * multishotMultiplier;
    
    double fireRateModifier = currModConfig.locksFireRate ? 1.0 : (1 + currModConfig.weaponModifiers[0]);
    double moddedFireRate = currAttack.fireRate * (fireRateModifier);

    std::array<double, 14> damageTypes = currAttack.damage;
    double totalBaseDamage             = currAttack.totalBaseDamage;
    const double impactDamageAmount = (currAttack.damage[0] * (1 + currModConfig.statusTypeModifiers[0]));   // Impact
    const double punctureDamageAmount = (currAttack.damage[1] * (1 + currModConfig.statusTypeModifiers[1]));   // Puncture
    const double slashDamageAmount = (currAttack.damage[2] * (1 + currModConfig.statusTypeModifiers[2]));   // Slash
    const double heatDamageAmount = damageTypes[3]    + (totalBaseDamage * currModConfig.statusTypeModifiers[3]);             // Heat
    const double coldDamageAmount = damageTypes[4]    + (totalBaseDamage * currModConfig.statusTypeModifiers[4]);             // Cold
    const double electricDamageAmount = damageTypes[5]    + (totalBaseDamage * currModConfig.statusTypeModifiers[5]);             // Electricity
    const double toxinDamageAmount = damageTypes[6]    + (totalBaseDamage * currModConfig.statusTypeModifiers[6]);             // Toxin
    const double blastDamageAmount = damageTypes[7]    + (totalBaseDamage * currModConfig.statusTypeModifiers[7]);             // Blast
    const double corrosiveDamageAmount = damageTypes[8]    + (totalBaseDamage * currModConfig.statusTypeModifiers[8]);             // Corrosive
    const double gasDamageAmount = damageTypes[9]    + (totalBaseDamage * currModConfig.statusTypeModifiers[9]);             // Gas
    const double magneticDamageAmount = damageTypes[10]   + (totalBaseDamage * currModConfig.statusTypeModifiers[10]);           // Magnetic
    const double radiationDamageAmount = damageTypes[11]   + (totalBaseDamage * currModConfig.statusTypeModifiers[11]);           // Radiation
    const double viralDamageAmount = damageTypes[12]   + (totalBaseDamage * currModConfig.statusTypeModifiers[12]);           // Viral
    const double tauDamageAmount = damageTypes[13];           // Tau


    double totalMiscDamage      = (impactDamageAmount + punctureDamageAmount) + (slashDamageAmount + tauDamageAmount);  // Impact | Puncture | Slash | Tau
    double totalBasicDamage     = (heatDamageAmount + coldDamageAmount) + (electricDamageAmount + toxinDamageAmount);    // Heat | Cold | Electric | Toxin
    double totalCombinedDamage  = (blastDamageAmount + corrosiveDamageAmount) + (gasDamageAmount + magneticDamageAmount) + (radiationDamageAmount + viralDamageAmount);   // Blast | Corrosive | Gas | Magnetic | Radiation | Viral
    totalDamage     =   totalMiscDamage + totalBasicDamage + totalCombinedDamage;


    damageTypes[0]  = impactDamageAmount;
    damageTypes[1]  = punctureDamageAmount;
    damageTypes[2]  = slashDamageAmount;
    damageTypes[3]  = heatDamageAmount;
    damageTypes[4]  = coldDamageAmount;
    damageTypes[5]  = electricDamageAmount;
    damageTypes[6]  = toxinDamageAmount;
    damageTypes[7]  = blastDamageAmount;
    damageTypes[8]  = corrosiveDamageAmount;
    damageTypes[9]  = gasDamageAmount;
    damageTypes[10] = magneticDamageAmount;
    damageTypes[11] = radiationDamageAmount;
    damageTypes[12] = viralDamageAmount;


    // std::cout<< "The total damage is: " << totalDamage << std::endl;

    // std::cout << "I summed total damage and got multishot value!\n";


    double fireRateInverse = 1 / moddedFireRate;
    double moddedChargeTime = (currAttack.chargeTime * fireRateInverse);
    double effectiveFireRate = (1 / (moddedChargeTime + moddedFireRate));
    

    // Calculate status amounts on enemy
    std::array<double, 14>* currStatusCounts = currEnemy.getStatusCounts();
    uint16_t damageTypesMask = currAttack.damageTypesMask ^ currModConfig.getStatusTypeMask();
    
    // setup data so it isn't calculated every loop for optimizing performance
    double totalDamageInverse = 1 / totalDamage;
    double avgStatusCountConstants = totalDamageInverse * currAttack.statusChance * multishotValue * effectiveFireRate * statusDurationModifier * statusChanceModifier; // status duration & chance mods at the end
    
    for (int i = 0; i < 14; i++)
    {
        if ((damageTypesMask & 1) == 1)  // skip if this damage doesn't exist
        {
            // use each types damage as a proportion of totalDamage to get damage distribution
            // multiply by status chance to get amount applied per hit
            // multiply by multishot to get amount applied per shot
            // multiply by effective fire rate to get procs/second
            // divide by time to expire or something to find amount per second on average considering expiration time
            // cap at max amount
            // add to enemy
            // update damage calculations ot take into account the CC, CD, etc. buffs
            // double proportionOfTotalDamage = currAttack.damage[i] / totalDamage;
            //  double statusAppliedPerHit = proportionOfTotalDamage * currAttack.statusChance;
            //  double statusAppliedPerShot = statusAppliedPerHit * multishotValue;
            //  double statusAppliedPerSecond = statusAppliedPerShot * effectiveFireRate;

            double averageStatusCount = damageTypes[i] * avgStatusCountConstants * baseStatusDurations[i];

            // round down if above cap
            double finalStatusCount = (std::min)(averageStatusCount, statusCaps[i]);

            distinctStatusCount += (finalStatusCount >= 1);

            (*currStatusCounts)[i] = finalStatusCount;
        }
        damageTypesMask >>= 1;  // move to the right one to look at the next status type
    }

    char shotTypeFirstChar = (currAttack.shotType.empty()) ? '1' : currAttack.shotType[0];
    double gunCOMultiplier = 0;
    if (shotTypeFirstChar == 'H')
    {
        // normal addative gunCO;
        baseDMGModValue += (gunCOModValue * distinctStatusCount);
    }
    else if (shotTypeFirstChar == 'P')
    {
        if (moddedWeapon.className[0] == 'B')
        {
            // gunCO apples to uncharged shot, so half the bonus (charged shot is 2x uncharged, this should balance it)
            gunCOModValue *= 0.5;
        }
        gunCOMultiplier += (gunCOModValue * distinctStatusCount);
    }


    // combined gunCO and damage mod multiplier
    double damageMultiplier = baseDMGModValue * (1 + (gunCOMultiplier * distinctStatusCount));

    // apply gunCO and base damage mods
    totalDamage = totalDamage * damageMultiplier;
    
    double moddedCritChance = currAttack.critChance * criticalChanceModifier;
    double moddedCritMultiplier = currAttack.critMultiplier * criticalDamageModifier;



    double statusAndModdedCritChance = (moddedCritChance + currEnemy.getAddedCritChance());
    double statusAndModdedCritMultiplier = (moddedCritMultiplier + currEnemy.getAddedCritDamage());

    double averageShot = totalDamage * ((1 + (statusAndModdedCritChance) * (statusAndModdedCritMultiplier)));

    double averageSingleShot = (totalDamage * (1 + (moddedCritChance * (moddedCritMultiplier - 1))));
        // std::cout << "I got here three!\n";
    // std::cout << "totalDamage is: " << totalDamage << " crit chance is: " << currAttack.critChance << " crit damage is: " << currAttack.critMultiplier << " and avg single shot dmg is: " << averageSingleShot << std::endl;

    double baseAvgDot = totalDamage * multishotValue * statusDamageModifier * 6 * statusDurationModifier;
    double avgSlashDot = baseAvgDot * dotModifier[0];
    double avgElectricityDot = baseAvgDot * dotModifier[1] * (1 + currModConfig.statusTypeModifiers[5]);
    double avgHeatDot = baseAvgDot * dotModifier[2] * (1 + currModConfig.statusTypeModifiers[3]);
    double avgToxinDot = baseAvgDot * dotModifier[3] * (1 + currModConfig.statusTypeModifiers[6]);
    double avgGasDot = baseAvgDot * dotModifier[4] * (1 + currModConfig.statusTypeModifiers[9]);

    double totalAvgDot = (avgSlashDot + avgElectricityDot + avgHeatDot + avgToxinDot + avgGasDot) * totalDamageInverse;
    double avgTotalAvgDot = totalAvgDot * currAttack.statusChance * statusChanceModifier * ((1 + (statusAndModdedCritChance) * (statusAndModdedCritMultiplier)));;

    // Starting Values
    double averageBurstDPS = avgTotalAvgDot;
    double numberOfShotsPerMag = 0;
    double averageSustainedDPS = avgTotalAvgDot;
    double ammoCostPerShotInverse = 1;
    double percentOfTimeShooting = 0;

    if (currAttack.ammoCost)  // anything but 0
    {
        ammoCostPerShotInverse = 1 / currAttack.ammoCost;
    }

    // avg burst dps (held but no reloads)
    // apply viral + corrosive to damage now too
    averageBurstDPS += averageShot * effectiveFireRate;

    // used to calculate time reloading
    if (moddedWeapon.magazineCapacity)  // anything but 0
    {
        numberOfShotsPerMag = (moddedWeapon.magazineCapacity * magazineCapacityModifier) * ammoCostPerShotInverse;
        percentOfTimeShooting = numberOfShotsPerMag / ((effectiveFireRate * moddedWeapon.reloadSpeed * reloadSpeedModifier) + numberOfShotsPerMag);
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
    averageSustainedDPS += averageBurstDPS * percentOfTimeShooting;
    return { averageSingleShot, averageBurstDPS, averageSustainedDPS };
}
std::tuple<double, double, double> calculateDPSValuesRangedBurst(Weapon& moddedWeapon, attackData& currAttack, int weaponTypeIndex, Enemy& currEnemy, weaponModConfig& currModConfig)
{
    // ---------------------------------------- CALCULATE DAMAGE ----------------------------------------
    // Initialize variables and get to local vars
    int distinctStatusCount = 0;
    double totalDamage = 0;
    double magazineCapacityModifier     = 1 + currModConfig.weaponModifiers[2];
    double criticalChanceModifier       = 1 + currModConfig.weaponModifiers[3];
    double criticalDamageModifier       = 1 + currModConfig.weaponModifiers[4];
    double baseDMGModValue              = 1 + currModConfig.weaponModifiers[5];
    double statusDurationModifier       = 1 + currModConfig.weaponModifiers[6];
    double statusChanceModifier         = 1 + currModConfig.weaponModifiers[7];
    double statusDamageModifier         = 1 + currModConfig.weaponModifiers[8];
    double reloadSpeedModifier          = 1 + currModConfig.weaponModifiers[9];
    double gunCOModValue                = 1 + currModConfig.weaponModifiers[10];


    double multishotMultiplier = currModConfig.locksMultishot ? 1.0 : (1 + currModConfig.weaponModifiers[1]);
    double multishotValue = currAttack.multishot * multishotMultiplier;
    
    double fireRateModifier = currModConfig.locksFireRate ? 1.0 : (1 + currModConfig.weaponModifiers[0]);
    double moddedFireRate = currAttack.fireRate * (fireRateModifier);

    std::array<double, 14> damageTypes = currAttack.damage;
    double totalBaseDamage             = currAttack.totalBaseDamage;
    const double impactDamageAmount = (currAttack.damage[0] * (1 + currModConfig.statusTypeModifiers[0]));   // Impact
    const double punctureDamageAmount = (currAttack.damage[1] * (1 + currModConfig.statusTypeModifiers[1]));   // Puncture
    const double slashDamageAmount = (currAttack.damage[2] * (1 + currModConfig.statusTypeModifiers[2]));   // Slash
    const double heatDamageAmount = damageTypes[3]    + (totalBaseDamage * currModConfig.statusTypeModifiers[3]);             // Heat
    const double coldDamageAmount = damageTypes[4]    + (totalBaseDamage * currModConfig.statusTypeModifiers[4]);             // Cold
    const double electricDamageAmount = damageTypes[5]    + (totalBaseDamage * currModConfig.statusTypeModifiers[5]);             // Electricity
    const double toxinDamageAmount = damageTypes[6]    + (totalBaseDamage * currModConfig.statusTypeModifiers[6]);             // Toxin
    const double blastDamageAmount = damageTypes[7]    + (totalBaseDamage * currModConfig.statusTypeModifiers[7]);             // Blast
    const double corrosiveDamageAmount = damageTypes[8]    + (totalBaseDamage * currModConfig.statusTypeModifiers[8]);             // Corrosive
    const double gasDamageAmount = damageTypes[9]    + (totalBaseDamage * currModConfig.statusTypeModifiers[9]);             // Gas
    const double magneticDamageAmount = damageTypes[10]   + (totalBaseDamage * currModConfig.statusTypeModifiers[10]);           // Magnetic
    const double radiationDamageAmount = damageTypes[11]   + (totalBaseDamage * currModConfig.statusTypeModifiers[11]);           // Radiation
    const double viralDamageAmount = damageTypes[12]   + (totalBaseDamage * currModConfig.statusTypeModifiers[12]);           // Viral
    const double tauDamageAmount = damageTypes[13];           // Tau


    double totalMiscDamage      = (impactDamageAmount + punctureDamageAmount) + (slashDamageAmount + tauDamageAmount);  // Impact | Puncture | Slash | Tau
    double totalBasicDamage     = (heatDamageAmount + coldDamageAmount) + (electricDamageAmount + toxinDamageAmount);    // Heat | Cold | Electric | Toxin
    double totalCombinedDamage  = (blastDamageAmount + corrosiveDamageAmount) + (gasDamageAmount + magneticDamageAmount) + (radiationDamageAmount + viralDamageAmount);   // Blast | Corrosive | Gas | Magnetic | Radiation | Viral
    totalDamage     =   totalMiscDamage + totalBasicDamage + totalCombinedDamage;


    damageTypes[0]  = impactDamageAmount;
    damageTypes[1]  = punctureDamageAmount;
    damageTypes[2]  = slashDamageAmount;
    damageTypes[3]  = heatDamageAmount;
    damageTypes[4]  = coldDamageAmount;
    damageTypes[5]  = electricDamageAmount;
    damageTypes[6]  = toxinDamageAmount;
    damageTypes[7]  = blastDamageAmount;
    damageTypes[8]  = corrosiveDamageAmount;
    damageTypes[9]  = gasDamageAmount;
    damageTypes[10] = magneticDamageAmount;
    damageTypes[11] = radiationDamageAmount;
    damageTypes[12] = viralDamageAmount;



    // std::cout<< "The total damage is: " << totalDamage << std::endl;

    // std::cout << "I summed total damage and got multishot value!\n";


    double fireRateInverse = 1 / moddedFireRate;
    double effectiveFireRate = ((currAttack.burstCount) / (fireRateInverse + ((currAttack.burstCount - 1) * currAttack.burstDelay)));
    
    
    // std::cout << "I calculated the fire rate!\n";

    // Calculate status amounts on enemy
    std::array<double, 14>* currStatusCounts = currEnemy.getStatusCounts();
    uint16_t damageTypesMask = currAttack.damageTypesMask ^ currModConfig.getStatusTypeMask();
    
    // setup data so it isn't calculated every loop for optimizing performance
    double totalDamageInverse = 1 / totalDamage;
    double avgStatusCountConstants = totalDamageInverse * currAttack.statusChance * multishotValue * effectiveFireRate * statusDurationModifier * statusChanceModifier; // status duration & chance mods at the end
    
    for (int i = 0; i < 14; i++)
    {
        if ((damageTypesMask & 1) == 1)  // skip if this damage doesn't exist
        {
            // use each types damage as a proportion of totalDamage to get damage distribution
            // multiply by status chance to get amount applied per hit
            // multiply by multishot to get amount applied per shot
            // multiply by effective fire rate to get procs/second
            // divide by time to expire or something to find amount per second on average considering expiration time
            // cap at max amount
            // add to enemy
            // update damage calculations ot take into account the CC, CD, etc. buffs
            // double proportionOfTotalDamage = currAttack.damage[i] / totalDamage;
            //  double statusAppliedPerHit = proportionOfTotalDamage * currAttack.statusChance;
            //  double statusAppliedPerShot = statusAppliedPerHit * multishotValue;
            //  double statusAppliedPerSecond = statusAppliedPerShot * effectiveFireRate;

            double averageStatusCount = damageTypes[i] * avgStatusCountConstants * baseStatusDurations[i];

            // round down if above cap
            double finalStatusCount = (std::min)(averageStatusCount, statusCaps[i]);

            distinctStatusCount += (finalStatusCount >= 1);

            (*currStatusCounts)[i] = finalStatusCount;
        }
        damageTypesMask >>= 1;  // move to the right one to look at the next status type
    }

    char shotTypeFirstChar = (currAttack.shotType.empty()) ? '1' : currAttack.shotType[0];
    double gunCOMultiplier = 0;
    if (shotTypeFirstChar == 'H')
    {
        // normal addative gunCO;
        baseDMGModValue += (gunCOModValue * distinctStatusCount);
    }
    else if (shotTypeFirstChar == 'P')
    {
        if (moddedWeapon.className[0] == 'B')
        {
            // gunCO apples to uncharged shot, so half the bonus (charged shot is 2x uncharged, this should balance it)
            gunCOModValue *= 0.5;
        }
        gunCOMultiplier += (gunCOModValue * distinctStatusCount);
    }


    // combined gunCO and damage mod multiplier
    double damageMultiplier = baseDMGModValue * (1 + (gunCOMultiplier * distinctStatusCount));

    // apply gunCO and base damage mods
    totalDamage = totalDamage * damageMultiplier;
    
    double moddedCritChance = currAttack.critChance * criticalChanceModifier;
    double moddedCritMultiplier = currAttack.critMultiplier * criticalDamageModifier;



    double statusAndModdedCritChance = (moddedCritChance + currEnemy.getAddedCritChance());
    double statusAndModdedCritMultiplier = (moddedCritMultiplier + currEnemy.getAddedCritDamage());

    double averageShot = totalDamage * ((1 + (statusAndModdedCritChance) * (statusAndModdedCritMultiplier)));

    double averageSingleShot = (totalDamage * (1 + (moddedCritChance * (moddedCritMultiplier - 1))));
        // std::cout << "I got here three!\n";
    // std::cout << "totalDamage is: " << totalDamage << " crit chance is: " << currAttack.critChance << " crit damage is: " << currAttack.critMultiplier << " and avg single shot dmg is: " << averageSingleShot << std::endl;
    
    double baseAvgDot = totalDamage * multishotValue * statusDamageModifier * 6 * statusDurationModifier;
    double avgSlashDot = baseAvgDot * dotModifier[0];
    double avgElectricityDot = baseAvgDot * dotModifier[1] * (1 + currModConfig.statusTypeModifiers[5]);
    double avgHeatDot = baseAvgDot * dotModifier[2] * (1 + currModConfig.statusTypeModifiers[3]);
    double avgToxinDot = baseAvgDot * dotModifier[3] * (1 + currModConfig.statusTypeModifiers[6]);
    double avgGasDot = baseAvgDot * dotModifier[4] * (1 + currModConfig.statusTypeModifiers[9]);

    double totalAvgDot = (avgSlashDot + avgElectricityDot + avgHeatDot + avgToxinDot + avgGasDot) * totalDamageInverse;
    double avgTotalAvgDot = totalAvgDot * currAttack.statusChance * statusChanceModifier * ((1 + (statusAndModdedCritChance) * (statusAndModdedCritMultiplier)));;


    // Starting Values
    double averageBurstDPS = avgTotalAvgDot;
    double numberOfShotsPerMag = 0;
    double averageSustainedDPS = avgTotalAvgDot;
    double ammoCostPerShotInverse = 1;
    double percentOfTimeShooting = 0;

    if (currAttack.ammoCost)  // anything but 0
    {
        ammoCostPerShotInverse = 1 / currAttack.ammoCost;
    }

    // avg burst dps (held but no reloads)
    // apply viral + corrosive to damage now too
    averageBurstDPS += averageShot * effectiveFireRate;

    // used to calculate time reloading
    if (moddedWeapon.magazineCapacity)  // anything but 0
    {
        numberOfShotsPerMag = moddedWeapon.magazineCapacity * magazineCapacityModifier * ammoCostPerShotInverse;
        percentOfTimeShooting = numberOfShotsPerMag / ((effectiveFireRate * moddedWeapon.reloadSpeed * reloadSpeedModifier) + numberOfShotsPerMag);
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
    averageSustainedDPS += averageBurstDPS * percentOfTimeShooting;
    return { averageSingleShot, averageBurstDPS, averageSustainedDPS };
}
std::tuple<double, double, double> calculateDPSValuesMelee(Weapon& moddedWeapon, attackData& currAttack, int weaponTypeIndex, Enemy& currEnemy, weaponModConfig& currModConfig)
{
    // ---------------------------------------- CALCULATE DAMAGE ----------------------------------------
    
    int distinctStatusCount = 0;
    double totalDamage = 0;
    double magazineCapacityModifier     = 1 + currModConfig.weaponModifiers[2];
    double criticalChanceModifier       = 1 + currModConfig.weaponModifiers[3];
    double criticalDamageModifier       = 1 + currModConfig.weaponModifiers[4];
    double baseDMGModValue              = 1 + currModConfig.weaponModifiers[5];
    double statusDurationModifier       = 1 + currModConfig.weaponModifiers[6];
    double statusChanceModifier         = 1 + currModConfig.weaponModifiers[7];
    double statusDamageModifier         = 1 + currModConfig.weaponModifiers[8];
    double reloadSpeedModifier          = 1 + currModConfig.weaponModifiers[9];
    double gunCOModValue                = 1 + currModConfig.weaponModifiers[10];
    
    double multishotMultiplier = currModConfig.locksMultishot ? 1.0 : (1 + currModConfig.weaponModifiers[1]);
    double multishotValue = currAttack.multishot * multishotMultiplier;
    
    double fireRateModifier = currModConfig.locksFireRate ? 1.0 : (1 + currModConfig.weaponModifiers[0]);
    double moddedFireRate = currAttack.fireRate * (fireRateModifier);

    std::array<double, 14> damageTypes = currAttack.damage;
    double totalBaseDamage             = currAttack.totalBaseDamage;
    const double impactDamageAmount = (currAttack.damage[0] * (1 + currModConfig.statusTypeModifiers[0]));   // Impact
    const double punctureDamageAmount = (currAttack.damage[1] * (1 + currModConfig.statusTypeModifiers[1]));   // Puncture
    const double slashDamageAmount = (currAttack.damage[2] * (1 + currModConfig.statusTypeModifiers[2]));   // Slash
    const double heatDamageAmount = damageTypes[3]    + (totalBaseDamage * currModConfig.statusTypeModifiers[3]);             // Heat
    const double coldDamageAmount = damageTypes[4]    + (totalBaseDamage * currModConfig.statusTypeModifiers[4]);             // Cold
    const double electricDamageAmount = damageTypes[5]    + (totalBaseDamage * currModConfig.statusTypeModifiers[5]);             // Electricity
    const double toxinDamageAmount = damageTypes[6]    + (totalBaseDamage * currModConfig.statusTypeModifiers[6]);             // Toxin
    const double blastDamageAmount = damageTypes[7]    + (totalBaseDamage * currModConfig.statusTypeModifiers[7]);             // Blast
    const double corrosiveDamageAmount = damageTypes[8]    + (totalBaseDamage * currModConfig.statusTypeModifiers[8]);             // Corrosive
    const double gasDamageAmount = damageTypes[9]    + (totalBaseDamage * currModConfig.statusTypeModifiers[9]);             // Gas
    const double magneticDamageAmount = damageTypes[10]   + (totalBaseDamage * currModConfig.statusTypeModifiers[10]);           // Magnetic
    const double radiationDamageAmount = damageTypes[11]   + (totalBaseDamage * currModConfig.statusTypeModifiers[11]);           // Radiation
    const double viralDamageAmount = damageTypes[12]   + (totalBaseDamage * currModConfig.statusTypeModifiers[12]);           // Viral
    const double tauDamageAmount = damageTypes[13];           // Tau


    double totalMiscDamage      = (impactDamageAmount + punctureDamageAmount) + (slashDamageAmount + tauDamageAmount);  // Impact | Puncture | Slash | Tau
    double totalBasicDamage     = (heatDamageAmount + coldDamageAmount) + (electricDamageAmount + toxinDamageAmount);    // Heat | Cold | Electric | Toxin
    double totalCombinedDamage  = (blastDamageAmount + corrosiveDamageAmount) + (gasDamageAmount + magneticDamageAmount) + (radiationDamageAmount + viralDamageAmount);   // Blast | Corrosive | Gas | Magnetic | Radiation | Viral
    totalDamage     =   totalMiscDamage + totalBasicDamage + totalCombinedDamage;


    damageTypes[0]  = impactDamageAmount;
    damageTypes[1]  = punctureDamageAmount;
    damageTypes[2]  = slashDamageAmount;
    damageTypes[3]  = heatDamageAmount;
    damageTypes[4]  = coldDamageAmount;
    damageTypes[5]  = electricDamageAmount;
    damageTypes[6]  = toxinDamageAmount;
    damageTypes[7]  = blastDamageAmount;
    damageTypes[8]  = corrosiveDamageAmount;
    damageTypes[9]  = gasDamageAmount;
    damageTypes[10] = magneticDamageAmount;
    damageTypes[11] = radiationDamageAmount;
    damageTypes[12] = viralDamageAmount;



    // std::cout<< "The total damage is: " << totalDamage << std::endl;

    // std::cout << "I summed total damage and got multishot value!\n";


    double effectiveFireRate = moddedFireRate;    

    // Calculate status amounts on enemy
    std::array<double, 14>* currStatusCounts = currEnemy.getStatusCounts();
    uint16_t damageTypesMask = currAttack.damageTypesMask ^ currModConfig.getStatusTypeMask();
    
    // setup data so it isn't calculated every loop for optimizing performance
    double totalDamageInverse = 1 / totalDamage;
    double avgStatusCountConstants = totalDamageInverse * currAttack.statusChance * multishotValue * effectiveFireRate * statusDurationModifier * statusChanceModifier; // status duration & chance mods at the end
    
    for (int i = 0; i < 14; i++)
    {
        if ((damageTypesMask & 1) == 1)  // skip if this damage doesn't exist
        {
            // use each types damage as a proportion of totalDamage to get damage distribution
            // multiply by status chance to get amount applied per hit
            // multiply by multishot to get amount applied per shot
            // multiply by effective fire rate to get procs/second
            // divide by time to expire or something to find amount per second on average considering expiration time
            // cap at max amount
            // add to enemy
            // update damage calculations ot take into account the CC, CD, etc. buffs
            // double proportionOfTotalDamage = currAttack.damage[i] / totalDamage;
            //  double statusAppliedPerHit = proportionOfTotalDamage * currAttack.statusChance;
            //  double statusAppliedPerShot = statusAppliedPerHit * multishotValue;
            //  double statusAppliedPerSecond = statusAppliedPerShot * effectiveFireRate;

            double averageStatusCount = damageTypes[i] * avgStatusCountConstants * baseStatusDurations[i];

            // round down if above cap
            double finalStatusCount = (std::min)(averageStatusCount, statusCaps[i]);

            distinctStatusCount += (finalStatusCount >= 1);

            (*currStatusCounts)[i] = finalStatusCount;
        }
        damageTypesMask >>= 1;  // move to the right one to look at the next status type
    }

    // combined gunCO and damage mod multiplier
    double damageMultiplier = baseDMGModValue * (1 + (gunCOModValue * distinctStatusCount));

    // apply gunCO and base damage mods
    totalDamage = totalDamage * damageMultiplier;
    
    double moddedCritChance = currAttack.critChance * criticalChanceModifier;
    double moddedCritMultiplier = currAttack.critMultiplier * criticalDamageModifier;



    double statusAndModdedCritChance = (moddedCritChance + currEnemy.getAddedCritChance());
    double statusAndModdedCritMultiplier = (moddedCritMultiplier + currEnemy.getAddedCritDamage());

    double averageShot = totalDamage * ((1 + (statusAndModdedCritChance) * (statusAndModdedCritMultiplier)));

    double averageSingleShot = (totalDamage * (1 + (moddedCritChance * (moddedCritMultiplier - 1))));
        // std::cout << "I got here three!\n";
    // std::cout << "totalDamage is: " << totalDamage << " crit chance is: " << currAttack.critChance << " crit damage is: " << currAttack.critMultiplier << " and avg single shot dmg is: " << averageSingleShot << std::endl;

    double baseAvgDot = totalDamage * multishotValue * statusDamageModifier * 6 * statusDurationModifier;
    double avgSlashDot = baseAvgDot * dotModifier[0];
    double avgElectricityDot = baseAvgDot * dotModifier[1] * (1 + currModConfig.statusTypeModifiers[5]);
    double avgHeatDot = baseAvgDot * dotModifier[2] * (1 + currModConfig.statusTypeModifiers[3]);
    double avgToxinDot = baseAvgDot * dotModifier[3] * (1 + currModConfig.statusTypeModifiers[6]);
    double avgGasDot = baseAvgDot * dotModifier[4] * (1 + currModConfig.statusTypeModifiers[9]);

    double totalAvgDot = (avgSlashDot + avgElectricityDot + avgHeatDot + avgToxinDot + avgGasDot) * totalDamageInverse;
    double avgTotalAvgDot = totalAvgDot * currAttack.statusChance * statusChanceModifier * ((1 + (statusAndModdedCritChance) * (statusAndModdedCritMultiplier)));;

    // Melee DPS
    // Going to have to add a slot for combo mods to choose from for the melee weapons, then i need to update this
    // TODO: update this calculation once combo mod can be chosen
    double averageSustainedDPS = (averageShot * currAttack.fireRate * (1 / moddedWeapon.comboDuration)) + avgTotalAvgDot;
    return { averageSingleShot, 0, averageSustainedDPS };
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
            validModTypes.push_back(weaponTypeTree[weaponTypeID - 1].name);
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


// --------------------------------------------------GUI SETUP--------------------------------------------------
// Data
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

std::tuple<std::vector<std::vector<std::array<int, 8>>>, std::vector<std::vector<double>>, std::vector<std::array<int, 3>>> calculateBestMods(std::string& weaponName, json& wikiModsData, json& wikiArcaneData, json& wikiPrimaryWeaponData, json& wikiSecondaryWeaponData, json& wikiMeleeWeaponData, int weaponGeneralClassIndex)
{
    json selectedWeaponType;
    std::vector<weaponType> weaponTypeTree;
    if (weaponGeneralClassIndex == 0) {
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
    } else if (weaponGeneralClassIndex == 1) {
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
    } else if (weaponGeneralClassIndex == 2) {
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

    if (!selectedWeaponType.contains(weaponName))
    {
        std::cerr << "Couldn't find a weapon called " << weaponName << ", please double check spelling and capitalization and try again" << std::endl;
    }

    nlohmann::json currentChosenWeapon = selectedWeaponType[weaponName];


    // filter to only check valid mod options
    int currentModIndex = 0;
    std::string currentWeaponType = currentChosenWeapon["Class"];

    /// Skip any mods with IsFlawed that is set to true
    /// If any entry in "Incompatible" starts with "Primed" there is a prime variant, so ignore the base    -   done in loading mods, not needed in this file


    int currentValidModsIndex = 0;
    std::vector<weaponMod> validMods = {};
    std::vector<std::string> validModTypes = getValidModTypes(currentWeaponType, weaponTypeTree);
    for (auto& [modName, modData] : wikiModsData["Mods"].items())
    {
        // prune flawed mods here too
        if (modData.value("IsFlawed", false))
        {   //  skip this mod, is flawed
            continue;
        }
        else if (std::find(validModTypes.begin(), validModTypes.end(), modData["Type"]) != validModTypes.end())
        {
            // push entries or default to empty strings/vectors of strings if that field doesn't exist for this mod
            validMods.push_back(weaponMod(modData.value("Name", ""), modData.value("Type", ""), modData.value("Description", ""), modData.value("Set", ""), modData.value("Class", ""), modData.value("IncompatibilityTags", std::vector<std::string>{}), modData.value("Incompatible", std::vector<std::string>{}), modData.value("UpgradeTypes", std::vector<std::string>{}), currentValidModsIndex));
            //  std::cout << "Adding " << modData.value("Name", " ") << modData.contains("Description") << std::endl;
        }
        else
        { // skip this mod, not valid
            continue;
        }
    }

    std::vector<std::string> validModNames = {};

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
            validModNames.push_back(currMod->name);
            currMod++;
        }
    }

    for (auto currMod = validMods.begin(); currMod != validMods.end();)
    {
        if (currMod->incompatibleMods.size() != 0)
        {
            for (auto incompatibleModName : currMod->incompatibleMods)
            {
                auto iteratorForMod = std::find(validModNames.begin(), validModNames.end(), incompatibleModName);
                if (iteratorForMod != validModNames.end())
                {
                    int indexForMod = std::distance(validModNames.begin(), iteratorForMod);
                    currMod->incompatibleModIndices.push_back(indexForMod);
                }
            }
        }
        currMod++;
    }


    // load arcanes
    std::vector<weaponArcane> validArcanes = {};
    for (auto& [arcaneName, arcaneData] : wikiArcaneData["Arcanes"].items())
    {
        if (std::find(validModTypes.begin(), validModTypes.end(), arcaneData["Type"]) != validModTypes.end())
        {
            // push entries or default to empty strings/vectors of strings if that field doesn't exist for this arcane
            validArcanes.push_back(weaponArcane(arcaneData.value("Name", ""), arcaneData.value("Type", ""), arcaneData.value("Description", ""), arcaneData.value("IncompatibilityTags", std::vector<std::string>{})));
            //  std::cout << "Adding " << arcaneData.value("Name", " ") << arcaneData.contains("Description") << std::endl;
        }
        else
        { // skip this arcane, not valid
            continue;
        }
    }

    // prune extra arcanes
    for (auto currArcane = validArcanes.begin(); currArcane != validArcanes.end();)
    {   // iterate from start until end
        if (currArcane->pruneThis == true)
        {   //  if arcane should be pruned, erase it, iterator auto skips to the next entry
            currArcane = validArcanes.erase(currArcane);
            //  std::cout << "Pruning " << currArcane->name << std::endl;
        }
        else
        {   //  if not pruned then iterate to next valid arcane
            currArcane++;
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
                currentAttack.value("ChargeTime", 0),           // chargeTime
                currentAttack.value("Trigger", ""),             //  triggerType
                currentAttack.value("ForcedProcs", std::vector<std::string>{}),    //  forcedProcs
                currentAttack.value("Damage", std::unordered_map<std::string, double>{})    //  damage
            )
        );
    }
    weaponList.push_back(Weapon(
            currentChosenWeapon.value("Name", ""),               //  name
            currentChosenWeapon.value("Class", ""),              //  className
            currentChosenWeapon.value("Family", ""),             //  weaponFamily
            currentChosenWeapon.value("Trigger", "notFound"),    //  triggerType
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
    std::vector<std::vector<std::array<int, 8>>> optimalModChoices = {};
    //  outer layer is for each attack, inner layer is for each type of DPS
    std::vector<std::vector<double>> optimalStats = {};
    for (int i = 0; i < weaponList.at(0).attackList.size(); i++)
    {
        std::array<int, 8> singleShotModIndices = {0};
        std::array<int, 8> burstDPSModIndices = {0};
        std::array<int, 8> sustainedDPSModIndicess = {0};
        // first vector is a list of the best mods for single shot dps, second vector is a list of the best mods for burst dps, third vector is a list of the bestmods for sustained dps
        std::vector<std::array<int, 8>> attacksModLayouts = {singleShotModIndices, burstDPSModIndices, sustainedDPSModIndicess};
        optimalModChoices.push_back(attacksModLayouts);

        // first entry is single shot average damage, second entry is burst dps, and third entry is sustained dps
        std::vector<double> attacksStats = {0, 0, 0};
        optimalStats.push_back(attacksStats);
    }

    std::vector<std::array<int, 3>> optimalArcaneIndices = {{-1}};

    std::array<double, 14> baseStatusCounts = {0};

    unsigned long long totalCalculations = 1;
    for (unsigned long long k = 1; k <= 8; k++)
    {
        totalCalculations = totalCalculations * (validMods.size() - 8 + k) / k;
    }
    totalCalculations = totalCalculations * validArcanes.size();
    totalCalculations = totalCalculations * weaponList[0].attackList.size();
    unsigned long long completedCalculations = 0;

    for (auto& currentWeapon : weaponList)
    {
        // nested for loops, each starting at 1 higher index, each ending 1 index earlier from the end of valid mods vector
        // then also make a loop that tries each permutation of elemental mods to try their configs
        if (weaponGeneralClassIndex != 2) // if not melee
        {
            std::tuple<double, double, double> (*damageCalcFunction)(Weapon& moddedWeapon, attackData& currAttack, int weaponTypeIndex, Enemy& currEnemy, weaponModConfig& currModConfig);

            if (currentWeapon.triggerTypeIndex)
            {
                if (currentWeapon.triggerTypeIndex == 1)
                {
                    damageCalcFunction = calculateDPSValuesRangedCharge;
                }
                else if (currentWeapon.triggerTypeIndex == 2)
                {
                    damageCalcFunction = calculateDPSValuesRangedBurst;
                }
            }
            else
            {
                damageCalcFunction = calculateDPSValuesRanged;
            }
            int attackCount = currentWeapon.attackList.size();
            int arcaneCount = validArcanes.size();

            // ---------------------------------------- FOR EACH ATTACK ----------------------------------------
            #pragma omp parallel for collapse(2) schedule(dynamic, 1) shared(optimalModChoices, optimalStats, optimalArcaneIndices)
            for (int attackIndex = 0 ; attackIndex < attackCount; ++attackIndex)
            {
                for (int arcaneSlotIndex = 0; arcaneSlotIndex < arcaneCount; arcaneSlotIndex++)
                {
                    attackData currentAttack = currentWeapon.attackList[attackIndex];
                    std::vector<std::vector<std::array<int, 8>>> localOptimalModChoices = {};
                    std::vector<std::vector<double>> localOptimalStats = {};
                    int localOptimalSingleShotArcaneIndex = -1;
                    int localOptimalBurstDPSArcaneIndex = -1;
                    int localOptimalSustainedDPSArcaneIndex = -1;
                    
                    // ---------------------------------------- BASE MOD CONFIG ----------------------------------------
                    weaponModConfig currentModConfig = weaponModConfig();
                    // ---------------------------------------- BASE ENEMY INFO ----------------------------------------
                    Enemy currEnemy = Enemy();

                    for (int i = 0; i < weaponList.at(0).attackList.size(); i++)
                    {
                        std::array<int, 8> singleShotModIndices = {};
                        std::array<int, 8> burstDPSModIndices = {};
                        std::array<int, 8> sustainedDPSModIndicess = {};
                        // first vector is a list of the best mods for single shot dps, second vector is a list of the best mods for burst dps, third vector is a list of the bestmods for sustained dps
                        std::vector<std::array<int, 8>> attacksModLayouts = {singleShotModIndices, burstDPSModIndices, sustainedDPSModIndicess};
                        localOptimalModChoices.push_back(attacksModLayouts);

                        // first entry is single shot average damage, second entry is burst dps, and third entry is sustained dps
                        std::vector<double> attacksStats = {0, 0, 0};
                        localOptimalStats.push_back(attacksStats);
                    }


                    currentModConfig.addArcane(validArcanes[arcaneSlotIndex]);
                    for (int modSlotOneIndex = 0; modSlotOneIndex < validMods.size() - 7; modSlotOneIndex++)
                    {
                        // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
                        bool skipModSlotOne = false;
                        for (int incompatibleMod : validMods[modSlotOneIndex].incompatibleModIndices)
                        {
                            if (std::find(currentModConfig.currentModIndices.begin(), currentModConfig.currentModIndices.end(), incompatibleMod) != currentModConfig.currentModIndices.end())
                            {   // if there is an incompatible mod already in the current mod config
                                skipModSlotOne = true;
                            }
                        }
                        if (skipModSlotOne)
                        {
                            continue;   //  skip to next mod in this slot
                        }

                        currentModConfig.addMod(validMods[modSlotOneIndex], modSlotOneIndex, 0);
                        // Start lower level loop
                        for (int modSlotTwoIndex = modSlotOneIndex + 1; modSlotTwoIndex < validMods.size() - 6; modSlotTwoIndex++)
                        {
                            // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
                            bool skipModSlotTwo = false;
                            for (int incompatibleMod : validMods[modSlotTwoIndex].incompatibleModIndices)
                            {
                                if (std::find(currentModConfig.currentModIndices.begin(), currentModConfig.currentModIndices.end(), incompatibleMod) != currentModConfig.currentModIndices.end())
                                {   // if there is an incompatible mod already in the current mod config
                                    skipModSlotTwo = true;
                                }
                            }
                            if (skipModSlotTwo)
                            {
                                continue;   //  skip to next mod in this slot
                            }

                            currentModConfig.addMod(validMods[modSlotTwoIndex], modSlotTwoIndex, 1);
                            // Start lower level loop
                            for (int modSlotThreeIndex = modSlotTwoIndex + 1; modSlotThreeIndex < validMods.size() - 5; modSlotThreeIndex++)
                            {
                                // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
                                bool skipModSlotThree = false;
                                for (int incompatibleMod : validMods[modSlotThreeIndex].incompatibleModIndices)
                                {
                                    if (std::find(currentModConfig.currentModIndices.begin(), currentModConfig.currentModIndices.end(), incompatibleMod) != currentModConfig.currentModIndices.end())
                                    {   // if there is an incompatible mod already in the current mod config
                                        skipModSlotThree = true;
                                    }
                                }
                                if (skipModSlotThree)
                                {
                                    continue;   //  skip to next mod in this slot
                                }

                                currentModConfig.addMod(validMods[modSlotThreeIndex], modSlotThreeIndex, 2);
                                // Start lower level loop
                                for (int modSlotFourIndex = modSlotThreeIndex + 1; modSlotFourIndex < validMods.size() - 4; modSlotFourIndex++)
                                {
                                    // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
                                    bool skipModSlotFour = false;
                                    for (int incompatibleMod : validMods[modSlotFourIndex].incompatibleModIndices)
                                    {
                                        if (std::find(currentModConfig.currentModIndices.begin(), currentModConfig.currentModIndices.end(), incompatibleMod) != currentModConfig.currentModIndices.end())
                                        {   // if there is an incompatible mod already in the current mod config
                                            skipModSlotFour = true;
                                        }
                                    }
                                    if (skipModSlotFour)
                                    {
                                        continue;   //  skip to next mod in this slot
                                    }

                                    currentModConfig.addMod(validMods[modSlotFourIndex], modSlotFourIndex, 3);
                                    // Start lower level loop
                                    for (int modSlotFiveIndex = modSlotFourIndex + 1; modSlotFiveIndex < validMods.size() - 3; modSlotFiveIndex++)
                                    {
                                        // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
                                        bool skipModSlotFive = false;
                                        for (int incompatibleMod : validMods[modSlotFiveIndex].incompatibleModIndices)
                                        {
                                            if (std::find(currentModConfig.currentModIndices.begin(), currentModConfig.currentModIndices.end(), incompatibleMod) != currentModConfig.currentModIndices.end())
                                            {   // if there is an incompatible mod already in the current mod config
                                                skipModSlotFive = true;
                                            }
                                        }
                                        if (skipModSlotFive)
                                        {
                                            continue;   //  skip to next mod in this slot
                                        }

                                        currentModConfig.addMod(validMods[modSlotFiveIndex], modSlotFiveIndex, 4);
                                        // Start lower level loop
                                        for (int modSlotSixIndex = modSlotFiveIndex + 1; modSlotSixIndex < validMods.size() - 2; modSlotSixIndex++)
                                        {
                                            // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
                                            bool skipModSlotSix = false;
                                            for (int incompatibleMod : validMods[modSlotSixIndex].incompatibleModIndices)
                                            {
                                                if (std::find(currentModConfig.currentModIndices.begin(), currentModConfig.currentModIndices.end(), incompatibleMod) != currentModConfig.currentModIndices.end())
                                                {   // if there is an incompatible mod already in the current mod config
                                                    skipModSlotSix = true;
                                                }
                                            }
                                            if (skipModSlotSix)
                                            {
                                                continue;   //  skip to next mod in this slot
                                            }

                                            currentModConfig.addMod(validMods[modSlotSixIndex], modSlotSixIndex, 5);
                                            // Start lower level loop
                                            for (int modSlotSevenIndex = modSlotSixIndex + 1; modSlotSevenIndex < validMods.size() - 1; modSlotSevenIndex++)
                                            {
                                                // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
                                                bool skipModSlotSeven = false;
                                                for (int incompatibleMod : validMods[modSlotSevenIndex].incompatibleModIndices)
                                                {
                                                    if (std::find(currentModConfig.currentModIndices.begin(), currentModConfig.currentModIndices.end(), incompatibleMod) != currentModConfig.currentModIndices.end())
                                                    {   // if there is an incompatible mod already in the current mod config
                                                        skipModSlotSeven = true;
                                                    }
                                                }
                                                if (skipModSlotSeven)
                                                {
                                                    continue;   //  skip to next mod in this slot
                                                }

                                                currentModConfig.addMod(validMods[modSlotSevenIndex], modSlotSevenIndex, 6);
                                                // Start lower level loop
                                                for (int modSlotEightIndex = modSlotSevenIndex + 1; modSlotEightIndex < validMods.size(); modSlotEightIndex++)
                                                {
                                                    // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
                                                    bool skipModSlotEight = false;
                                                    for (int incompatibleMod : validMods[modSlotEightIndex].incompatibleModIndices)
                                                    {
                                                        if (std::find(currentModConfig.currentModIndices.begin(), currentModConfig.currentModIndices.end(), incompatibleMod) != currentModConfig.currentModIndices.end())
                                                        {   // if there is an incompatible mod already in the current mod config
                                                            skipModSlotEight = true;
                                                        }
                                                    }
                                                    if (skipModSlotEight)
                                                    {
                                                        continue;   //  skip to next mod in this slot
                                                    }
                                    
                                                    currentModConfig.addMod(validMods[modSlotEightIndex], modSlotEightIndex, 7);

                                                    // auto weaponBeforeMod = currentWeapon;
                                                                   
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
                                                    auto [tempAverageShot, tempAverageBurstDPS, tempAverageSustainedDPS] = damageCalcFunction(currentWeapon, currentAttack, weaponGeneralClassIndex, currEnemy, currentModConfig);
                                                            // std::cout << "I calculated the DPS!\n";
                                                    if (tempAverageShot > localOptimalStats.at(currentAttack.attackIndex - 1).at(0))
                                                    {
                                                        localOptimalStats[currentAttack.attackIndex - 1][0] = tempAverageShot;
                                                        localOptimalModChoices[currentAttack.attackIndex - 1][0] = currentModConfig.currentModIndices;
                                                        localOptimalSingleShotArcaneIndex = arcaneSlotIndex;
                                                    }
                                                    if (tempAverageBurstDPS > localOptimalStats.at(currentAttack.attackIndex - 1).at(1))
                                                    {
                                                        localOptimalStats[currentAttack.attackIndex - 1][1] = tempAverageBurstDPS;
                                                        localOptimalModChoices[currentAttack.attackIndex - 1][1] = currentModConfig.currentModIndices;
                                                        localOptimalBurstDPSArcaneIndex = arcaneSlotIndex;
                                                    }
                                                    if (tempAverageSustainedDPS > localOptimalStats.at(currentAttack.attackIndex - 1).at(2))
                                                    {
                                                        localOptimalStats[currentAttack.attackIndex - 1][2] = tempAverageSustainedDPS;
                                                        localOptimalModChoices[currentAttack.attackIndex - 1][2] = currentModConfig.currentModIndices;
                                                        localOptimalSustainedDPSArcaneIndex = arcaneSlotIndex;
                                                    }

                                                    // if ((++completedCalculations % 10000000) == 0)
                                                    // {   //  print remaining calculation number every 10M completed calcs
                                                    //     std::cout << "Remaining: " << (totalCalculations - completedCalculations) << '\n';
                                                    // }

                                                    currentModConfig.removeMod(validMods[modSlotEightIndex], modSlotEightIndex, 7);    //  Last entry should always be this mod as it is about to move to a lower level
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
                                                    if (currentModConfig.currentModIndices.size() != 1)
                                                    {
                                                        std::cout << "Applying " << currentModConfig.currentModIndices.size() << " mods" << std::endl;
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
                                                currentModConfig.removeMod(validMods[modSlotSevenIndex], modSlotSevenIndex, 6);    //  Last entry should always be this mod as it is about to move to a lower level
                                                // Move up a loop
                                            }
                                            currentModConfig.removeMod(validMods[modSlotSixIndex], modSlotSixIndex, 5);    //  Last entry should always be this mod as it is about to move to a lower level
                                            // Move up a loop
                                        }
                                        currentModConfig.removeMod(validMods[modSlotFiveIndex], modSlotFiveIndex, 4);    //  Last entry should always be this mod as it is about to move to a lower level
                                        // Move up a loop
                                    }
                                    currentModConfig.removeMod(validMods[modSlotFourIndex], modSlotFourIndex, 3);    //  Last entry should always be this mod as it is about to move to a lower level
                                    // Move up a loop
                                }
                                currentModConfig.removeMod(validMods[modSlotThreeIndex], modSlotThreeIndex, 2);    //  Last entry should always be this mod as it is about to move to a lower level
                                // Move up a loop
                            }
                            currentModConfig.removeMod(validMods[modSlotTwoIndex], modSlotTwoIndex, 1);    //  Last entry should always be this mod as it is about to move to a lower level
                            // Move up a loop
                        }
                        currentModConfig.removeMod(validMods[modSlotOneIndex], modSlotOneIndex, 0);    //  Last entry should always be this mod as it is about to move to a lower level
                        // Move up a loop
                    }
                    currentModConfig.removeArcane(validArcanes[arcaneSlotIndex]);

                    if (localOptimalStats.at(currentAttack.attackIndex - 1).at(0) > optimalStats.at(currentAttack.attackIndex - 1).at(0))
                    {
                        #pragma omp critical(globalBestUpdate)
                        {
                            if (localOptimalStats.at(currentAttack.attackIndex - 1).at(0) > optimalStats.at(currentAttack.attackIndex - 1).at(0))
                            {
                                optimalStats[currentAttack.attackIndex - 1][0] = localOptimalStats[currentAttack.attackIndex - 1][0];
                                optimalModChoices[currentAttack.attackIndex - 1][0] = localOptimalModChoices[currentAttack.attackIndex - 1][0];
                                optimalArcaneIndices[currentAttack.attackIndex - 1][0] = localOptimalSingleShotArcaneIndex;
                            }
                        }
                    }
                    if (localOptimalStats.at(currentAttack.attackIndex - 1).at(1) > optimalStats.at(currentAttack.attackIndex - 1).at(1))
                    {
                        #pragma omp critical(globalBestUpdate)
                        {
                            if (localOptimalStats.at(currentAttack.attackIndex - 1).at(1) > optimalStats.at(currentAttack.attackIndex - 1).at(1))
                            {
                                optimalStats[currentAttack.attackIndex - 1][1] = localOptimalStats[currentAttack.attackIndex - 1][1];
                                optimalModChoices[currentAttack.attackIndex - 1][1] = localOptimalModChoices[currentAttack.attackIndex - 1][1];
                                optimalArcaneIndices[currentAttack.attackIndex - 1][1] = localOptimalBurstDPSArcaneIndex;
                            }
                        }
                    }
                    if (localOptimalStats.at(currentAttack.attackIndex - 1).at(2) > optimalStats.at(currentAttack.attackIndex - 1).at(2))
                    {
                        #pragma omp critical(globalBestUpdate)
                        {
                            if (localOptimalStats.at(currentAttack.attackIndex - 1).at(2) > optimalStats.at(currentAttack.attackIndex - 1).at(2))
                            {
                                optimalStats[currentAttack.attackIndex - 1][2] = localOptimalStats[currentAttack.attackIndex - 1][2];
                                optimalModChoices[currentAttack.attackIndex - 1][2] = localOptimalModChoices[currentAttack.attackIndex - 1][2];
                                optimalArcaneIndices[currentAttack.attackIndex - 1][2] = localOptimalSustainedDPSArcaneIndex;
                            }
                        }
                    }
                }
            }
            for (int i = 0; i < currentWeapon.attackList.size(); i++)
            {
                std::cout << "For the: " << currentWeapon.name << "'s " << currentWeapon.attackList[i].attackName << " attack, the calculated best stats are as follows: Average shot: " << optimalStats.at(i).at(0) << ", Average burst DPS: " << optimalStats.at(i).at(1) << ", Average Sustained DPS: " << optimalStats.at(i).at(2) << std::endl;
                std::cout << "Using the following arcane and mods for single shot: \n";
                std::cout << validArcanes.at(optimalArcaneIndices[i][0]).name << "\n";
                for (int j = 0; j < 8; j++)
                {
                    std::cout << validModNames.at(optimalModChoices.at(i).at(0).at(j)) << "\n";
                }
                std::cout << "Using the following arcane and mods for burst DPS: \n";
                std::cout << validArcanes.at(optimalArcaneIndices[i][1]).name << "\n";
                for (int j = 0; j < 8; j++)
                {
                    std::cout << validModNames.at(optimalModChoices.at(i).at(1).at(j)) << "\n";
                }
                std::cout << "Using the following arcane and mods for sustained DPS: \n";
                std::cout << validArcanes.at(optimalArcaneIndices[i][2]).name << "\n";
                for (int j = 0; j < 8; j++)
                {
                    std::cout << validModNames.at(optimalModChoices.at(i).at(2).at(j)) << "\n";
                }
            }
        }
        else    // for melee weapons
        {
            int attackCount = currentWeapon.attackList.size();
            int arcaneCount = validArcanes.size();

            // ---------------------------------------- FOR EACH ATTACK ----------------------------------------
            #pragma omp parallel for collapse(2) schedule(dynamic, 1) shared(optimalModChoices, optimalStats, optimalArcaneIndices)
            for (int attackIndex = 0 ; attackIndex < attackCount; ++attackIndex)
            {
                for (int arcaneSlotIndex = 0; arcaneSlotIndex < arcaneCount; arcaneSlotIndex++)
                {
                    attackData currentAttack = currentWeapon.attackList[attackIndex];
                    std::vector<std::vector<std::array<int, 8>>> localOptimalModChoices = {};
                    std::vector<std::vector<double>> localOptimalStats = {};
                    int localOptimalSingleShotArcaneIndex = -1;
                    int localOptimalBurstDPSArcaneIndex = -1;
                    int localOptimalSustainedDPSArcaneIndex = -1;
                    
                    // ---------------------------------------- BASE MOD CONFIG ----------------------------------------
                    weaponModConfig currentModConfig = weaponModConfig();
                    // ---------------------------------------- BASE ENEMY INFO ----------------------------------------
                    Enemy currEnemy = Enemy();

                    for (int i = 0; i < weaponList.at(0).attackList.size(); i++)
                    {
                        std::array<int, 8> singleShotModIndices = {};
                        std::array<int, 8> burstDPSModIndices = {};
                        std::array<int, 8> sustainedDPSModIndicess = {};
                        // first vector is a list of the best mods for single shot dps, second vector is a list of the best mods for burst dps, third vector is a list of the bestmods for sustained dps
                        std::vector<std::array<int, 8>> attacksModLayouts = {singleShotModIndices, burstDPSModIndices, sustainedDPSModIndicess};
                        localOptimalModChoices.push_back(attacksModLayouts);

                        // first entry is single shot average damage, second entry is burst dps, and third entry is sustained dps
                        std::vector<double> attacksStats = {0, 0, 0};
                        localOptimalStats.push_back(attacksStats);
                    }


                    currentModConfig.addArcane(validArcanes[arcaneSlotIndex]);
                    for (int modSlotOneIndex = 0; modSlotOneIndex < validMods.size() - 7; modSlotOneIndex++)
                    {
                        // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
                        bool skipModSlotOne = false;
                        for (int incompatibleMod : validMods[modSlotOneIndex].incompatibleModIndices)
                        {
                            if (std::find(currentModConfig.currentModIndices.begin(), currentModConfig.currentModIndices.end(), incompatibleMod) != currentModConfig.currentModIndices.end())
                            {   // if there is an incompatible mod already in the current mod config
                                skipModSlotOne = true;
                            }
                        }
                        if (skipModSlotOne)
                        {
                            continue;   //  skip to next mod in this slot
                        }

                        currentModConfig.addMod(validMods[modSlotOneIndex], modSlotOneIndex, 0);
                        // Start lower level loop
                        for (int modSlotTwoIndex = modSlotOneIndex + 1; modSlotTwoIndex < validMods.size() - 6; modSlotTwoIndex++)
                        {
                            // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
                            bool skipModSlotTwo = false;
                            for (int incompatibleMod : validMods[modSlotTwoIndex].incompatibleModIndices)
                            {
                                if (std::find(currentModConfig.currentModIndices.begin(), currentModConfig.currentModIndices.end(), incompatibleMod) != currentModConfig.currentModIndices.end())
                                {   // if there is an incompatible mod already in the current mod config
                                    skipModSlotTwo = true;
                                }
                            }
                            if (skipModSlotTwo)
                            {
                                continue;   //  skip to next mod in this slot
                            }

                            currentModConfig.addMod(validMods[modSlotTwoIndex], modSlotTwoIndex, 1);
                            // Start lower level loop
                            for (int modSlotThreeIndex = modSlotTwoIndex + 1; modSlotThreeIndex < validMods.size() - 5; modSlotThreeIndex++)
                            {
                                // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
                                bool skipModSlotThree = false;
                                for (int incompatibleMod : validMods[modSlotThreeIndex].incompatibleModIndices)
                                {
                                    if (std::find(currentModConfig.currentModIndices.begin(), currentModConfig.currentModIndices.end(), incompatibleMod) != currentModConfig.currentModIndices.end())
                                    {   // if there is an incompatible mod already in the current mod config
                                        skipModSlotThree = true;
                                    }
                                }
                                if (skipModSlotThree)
                                {
                                    continue;   //  skip to next mod in this slot
                                }

                                currentModConfig.addMod(validMods[modSlotThreeIndex], modSlotThreeIndex, 2);
                                // Start lower level loop
                                for (int modSlotFourIndex = modSlotThreeIndex + 1; modSlotFourIndex < validMods.size() - 4; modSlotFourIndex++)
                                {
                                    // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
                                    bool skipModSlotFour = false;
                                    for (int incompatibleMod : validMods[modSlotFourIndex].incompatibleModIndices)
                                    {
                                        if (std::find(currentModConfig.currentModIndices.begin(), currentModConfig.currentModIndices.end(), incompatibleMod) != currentModConfig.currentModIndices.end())
                                        {   // if there is an incompatible mod already in the current mod config
                                            skipModSlotFour = true;
                                        }
                                    }
                                    if (skipModSlotFour)
                                    {
                                        continue;   //  skip to next mod in this slot
                                    }

                                    currentModConfig.addMod(validMods[modSlotFourIndex], modSlotFourIndex, 3);
                                    // Start lower level loop
                                    for (int modSlotFiveIndex = modSlotFourIndex + 1; modSlotFiveIndex < validMods.size() - 3; modSlotFiveIndex++)
                                    {
                                        // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
                                        bool skipModSlotFive = false;
                                        for (int incompatibleMod : validMods[modSlotFiveIndex].incompatibleModIndices)
                                        {
                                            if (std::find(currentModConfig.currentModIndices.begin(), currentModConfig.currentModIndices.end(), incompatibleMod) != currentModConfig.currentModIndices.end())
                                            {   // if there is an incompatible mod already in the current mod config
                                                skipModSlotFive = true;
                                            }
                                        }
                                        if (skipModSlotFive)
                                        {
                                            continue;   //  skip to next mod in this slot
                                        }

                                        currentModConfig.addMod(validMods[modSlotFiveIndex], modSlotFiveIndex, 4);
                                        // Start lower level loop
                                        for (int modSlotSixIndex = modSlotFiveIndex + 1; modSlotSixIndex < validMods.size() - 2; modSlotSixIndex++)
                                        {
                                            // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
                                            bool skipModSlotSix = false;
                                            for (int incompatibleMod : validMods[modSlotSixIndex].incompatibleModIndices)
                                            {
                                                if (std::find(currentModConfig.currentModIndices.begin(), currentModConfig.currentModIndices.end(), incompatibleMod) != currentModConfig.currentModIndices.end())
                                                {   // if there is an incompatible mod already in the current mod config
                                                    skipModSlotSix = true;
                                                }
                                            }
                                            if (skipModSlotSix)
                                            {
                                                continue;   //  skip to next mod in this slot
                                            }

                                            currentModConfig.addMod(validMods[modSlotSixIndex], modSlotSixIndex, 5);
                                            // Start lower level loop
                                            for (int modSlotSevenIndex = modSlotSixIndex + 1; modSlotSevenIndex < validMods.size() - 1; modSlotSevenIndex++)
                                            {
                                                // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
                                                bool skipModSlotSeven = false;
                                                for (int incompatibleMod : validMods[modSlotSevenIndex].incompatibleModIndices)
                                                {
                                                    if (std::find(currentModConfig.currentModIndices.begin(), currentModConfig.currentModIndices.end(), incompatibleMod) != currentModConfig.currentModIndices.end())
                                                    {   // if there is an incompatible mod already in the current mod config
                                                        skipModSlotSeven = true;
                                                    }
                                                }
                                                if (skipModSlotSeven)
                                                {
                                                    continue;   //  skip to next mod in this slot
                                                }

                                                currentModConfig.addMod(validMods[modSlotSevenIndex], modSlotSevenIndex, 6);
                                                // Start lower level loop
                                                for (int modSlotEightIndex = modSlotSevenIndex + 1; modSlotEightIndex < validMods.size(); modSlotEightIndex++)
                                                {
                                                    // this section skips to the next possible mod in this slot if any mods that are already selected are incompatible with it
                                                    bool skipModSlotEight = false;
                                                    for (int incompatibleMod : validMods[modSlotEightIndex].incompatibleModIndices)
                                                    {
                                                        if (std::find(currentModConfig.currentModIndices.begin(), currentModConfig.currentModIndices.end(), incompatibleMod) != currentModConfig.currentModIndices.end())
                                                        {   // if there is an incompatible mod already in the current mod config
                                                            skipModSlotEight = true;
                                                        }
                                                    }
                                                    if (skipModSlotEight)
                                                    {
                                                        continue;   //  skip to next mod in this slot
                                                    }
                                    
                                                    currentModConfig.addMod(validMods[modSlotEightIndex], modSlotEightIndex, 7);

                                                    // auto weaponBeforeMod = currentWeapon;
                                                                   
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
                                                    auto [tempAverageShot, tempAverageBurstDPS, tempAverageSustainedDPS] = calculateDPSValuesMelee(currentWeapon, currentAttack, weaponGeneralClassIndex, currEnemy, currentModConfig);
                                                            // std::cout << "I calculated the DPS!\n";
                                                    if (tempAverageShot > localOptimalStats.at(currentAttack.attackIndex - 1).at(0))
                                                    {
                                                        localOptimalStats[currentAttack.attackIndex - 1][0] = tempAverageShot;
                                                        localOptimalModChoices[currentAttack.attackIndex - 1][0] = currentModConfig.currentModIndices;
                                                        localOptimalSingleShotArcaneIndex = arcaneSlotIndex;
                                                    }
                                                    if (tempAverageSustainedDPS > localOptimalStats.at(currentAttack.attackIndex - 1).at(2))
                                                    {
                                                        localOptimalStats[currentAttack.attackIndex - 1][2] = tempAverageSustainedDPS;
                                                        localOptimalModChoices[currentAttack.attackIndex - 1][2] = currentModConfig.currentModIndices;
                                                        localOptimalSustainedDPSArcaneIndex = arcaneSlotIndex;
                                                    }

                                                    // if ((++completedCalculations % 10000000) == 0)
                                                    // {   //  print remaining calculation number every 10M completed calcs
                                                    //     std::cout << "Remaining: " << (totalCalculations - completedCalculations) << '\n';
                                                    // }

                                                    currentModConfig.removeMod(validMods[modSlotEightIndex], modSlotEightIndex, 7);    //  Last entry should always be this mod as it is about to move to a lower level
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
                                                    if (currentModConfig.currentModIndices.size() != 1)
                                                    {
                                                        std::cout << "Applying " << currentModConfig.currentModIndices.size() << " mods" << std::endl;
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
                                                currentModConfig.removeMod(validMods[modSlotSevenIndex], modSlotSevenIndex, 6);    //  Last entry should always be this mod as it is about to move to a lower level
                                                // Move up a loop
                                            }
                                            currentModConfig.removeMod(validMods[modSlotSixIndex], modSlotSixIndex, 5);    //  Last entry should always be this mod as it is about to move to a lower level
                                            // Move up a loop
                                        }
                                        currentModConfig.removeMod(validMods[modSlotFiveIndex], modSlotFiveIndex, 4);    //  Last entry should always be this mod as it is about to move to a lower level
                                        // Move up a loop
                                    }
                                    currentModConfig.removeMod(validMods[modSlotFourIndex], modSlotFourIndex, 3);    //  Last entry should always be this mod as it is about to move to a lower level
                                    // Move up a loop
                                }
                                currentModConfig.removeMod(validMods[modSlotThreeIndex], modSlotThreeIndex, 2);    //  Last entry should always be this mod as it is about to move to a lower level
                                // Move up a loop
                            }
                            currentModConfig.removeMod(validMods[modSlotTwoIndex], modSlotTwoIndex, 1);    //  Last entry should always be this mod as it is about to move to a lower level
                            // Move up a loop
                        }
                        currentModConfig.removeMod(validMods[modSlotOneIndex], modSlotOneIndex, 0);    //  Last entry should always be this mod as it is about to move to a lower level
                        // Move up a loop
                    }
                    currentModConfig.removeArcane(validArcanes[arcaneSlotIndex]);

                    if (localOptimalStats.at(currentAttack.attackIndex - 1).at(0) > optimalStats.at(currentAttack.attackIndex - 1).at(0))
                    {
                        #pragma omp critical(globalBestUpdate)
                        {
                            if (localOptimalStats.at(currentAttack.attackIndex - 1).at(0) > optimalStats.at(currentAttack.attackIndex - 1).at(0))
                            {
                                optimalStats[currentAttack.attackIndex - 1][0] = localOptimalStats[currentAttack.attackIndex - 1][0];
                                optimalModChoices[currentAttack.attackIndex - 1][0] = localOptimalModChoices[currentAttack.attackIndex - 1][0];
                                optimalArcaneIndices[currentAttack.attackIndex - 1][0] = localOptimalSingleShotArcaneIndex;
                            }
                        }
                    }
                    if (localOptimalStats.at(currentAttack.attackIndex - 1).at(2) > optimalStats.at(currentAttack.attackIndex - 1).at(2))
                    {
                        #pragma omp critical(globalBestUpdate)
                        {
                            if (localOptimalStats.at(currentAttack.attackIndex - 1).at(2) > optimalStats.at(currentAttack.attackIndex - 1).at(2))
                            {
                                optimalStats[currentAttack.attackIndex - 1][2] = localOptimalStats[currentAttack.attackIndex - 1][2];
                                optimalModChoices[currentAttack.attackIndex - 1][2] = localOptimalModChoices[currentAttack.attackIndex - 1][2];
                                optimalArcaneIndices[currentAttack.attackIndex - 1][2] = localOptimalSustainedDPSArcaneIndex;
                            }
                        }
                    }
                }
            }
            for (int i = 0; i < currentWeapon.attackList.size(); i++)
            {
                std::cout << "For the: " << currentWeapon.name << "'s " << currentWeapon.attackList[i].attackName << " attack, the calculated best stats are as follows: Average shot: " << optimalStats.at(i).at(0) << ", Average burst DPS: " << optimalStats.at(i).at(1) << ", Average Sustained DPS: " << optimalStats.at(i).at(2) << std::endl;
                std::cout << "Using the following arcane and mods for single shot: \n";
                std::cout << validArcanes.at(optimalArcaneIndices[i][0]).name << "\n";
                for (int j = 0; j < 8; j++)
                {
                    std::cout << validModNames.at(optimalModChoices.at(i).at(0).at(j)) << "\n";
                }
                std::cout << "Using the following arcane and mods for sustained DPS: \n";
                std::cout << validArcanes.at(optimalArcaneIndices[i][2]).name << "\n";
                for (int j = 0; j < 8; j++)
                {
                    std::cout << validModNames.at(optimalModChoices.at(i).at(2).at(j)) << "\n";
                }
            }
        }
    }
    std::tuple<std::vector<std::vector<std::array<int, 8>>>, std::vector<std::vector<double>>, std::vector<std::array<int, 3>>> returnTuple = {optimalModChoices, optimalStats, optimalArcaneIndices};
    return returnTuple;
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


    std::tuple<std::vector<std::vector<std::array<int, 8>>>, std::vector<std::vector<double>>, std::vector<std::array<int, 3>>> resultTuple = {{{{0}}}, {{0.0}}, {{0}}};
    double calculationTime = 0;

    // ---------------------------------------- START USER GUI ----------------------------------------
    // Make process DPI aware and obtain main monitor scale
    ImGui_ImplWin32_EnableDpiAwareness();
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

    // Create application window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_OVERLAPPEDWINDOW, 100, 100, (int)(1280 * main_scale), (int)(800 * main_scale), nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    bool ok = ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
    printf("ImGui_ImplDX11_Init returned %d\n", ok);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    bool closeThisWindow = false;
    while (!closeThisWindow)
    {
        MSG msg;
        while (::PeekMessageW(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
            {
                closeThisWindow = true;
            }
        }
        if (closeThisWindow)
        {
            break;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();

        ImGui::NewFrame();

        if (ImGui::Begin("Damage Optimizer"))
        {
            ImGui::Text("Weapon slot:");
            
            ImVec2 buttonSize = ImVec2(100, 100);
            
            static int selectedWeaponType = 0;
            if (ImGui::Selectable("Primary", selectedWeaponType == 0, 0, buttonSize))   selectedWeaponType = 0;
            ImGui::SameLine();
            if (ImGui::Selectable("Secondary", selectedWeaponType == 1, 0, buttonSize))   selectedWeaponType = 1;
            ImGui::SameLine();
            if (ImGui::Selectable("Melee", selectedWeaponType == 2, 0, buttonSize))   selectedWeaponType = 2;

            ImGui::Text("Weapon name:");
            static std::string weaponName = "";
            ImGui::InputText(" ", &weaponName);

            if (ImGui::Button("Calculate"))
            {
                auto calculationStartTime = std::chrono::steady_clock::now();
                resultTuple = calculateBestMods(weaponName, wikiModsData, wikiArcaneData, wikiPrimaryWeaponData, wikiSecondaryWeaponData, wikiMeleeWeaponData, selectedWeaponType);
                auto calculationEndTime = std::chrono::steady_clock::now();
                std::chrono::duration<double, std::milli> elapsedTime = calculationEndTime - calculationStartTime;
                calculationTime = elapsedTime.count();
                calculationTime = calculationTime * 0.001;
            }

            if (calculationTime != 0)
            {
                ImGui::Text("Calculation Time: ");
                ImGui::SameLine();
                ImGui::Text(std::to_string(calculationTime).c_str());
                ImGui::SameLine();
                ImGui::Text(" seconds");
            }

        }
        ImGui::End();

        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Present
        HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync
        //HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);


    // ---------------------------------------- USER SETTINGS ----------------------------------------
    bool heatArmorStrip = false;
    int maxDrain = 999;
    // ---------------------------------------- USER SETTINGS ----------------------------------------
    std::string weaponName = "";
    int weaponGeneralClassIndex = 0;

    return 0;
}






// ---------------------------------------------------------- IMGUI HELPERS ----------------------------------------------------------
// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    // This is a basic setup. Optimally could use e.g. DXGI_SWAP_EFFECT_FLIP_DISCARD and handle fullscreen mode differently. See #8979 for suggestions.
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}