#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

class attackData
{
    public:
        double attackIndex = 0;
        std::string attackName = "Normal Attack";

        std::string shotType = "";

        double critChance = 0;
        double critMultiplier = 1;

        double statusChance = 0;

        double fireRate = 0;
        double ammoCost = 0;

        double multishot = 0;

        double burstCount = 0;
        double burstDelay = 0;

        double totalBaseDamage = 0;

        double totalIPSDamage = 0;
        double impactPercent = 0;
        double puncturePercent = 0;
        double slashPercent = 0;

        std::string triggerType = "";

        std::vector<std::string> forcedProcs = {};

        std::unordered_map<std::string, double> damage = {
            {"Impact", 0},
            {"Puncture", 0},
            {"Slash", 0},
            {"Heat", 0},
            {"Cold", 0},
            {"Electricity", 0},
            {"Toxin", 0},
            {"Blast", 0},
            {"Corrosive", 0},
            {"Gas", 0},
            {"Magnetic", 0},
            {"Radiation", 0},
            {"Viral", 0},
            {"Tau", 0}

        };

        attackData(
            double newAttackIndex,
            std::string newAttackName,
            std::string newShotType,
            double newCritChance,
            double newCritMultiplier,
            double newStatusChance,
            double newFireRate,
            double newAmmoCost,
            double newMultishot,
            double newBurstCount,
            double newBurstDelay,
            std::string newTriggerType,
            std::vector<std::string> newForcedProcs,
            std::unordered_map<std::string, double> newDamage
        )
        {
            this->attackIndex       =   newAttackIndex;
            this->attackName        =   newAttackName;
            this->shotType          =   newShotType;
            this->critChance        =   newCritChance;
            this->critMultiplier    =   newCritMultiplier;
            this->statusChance      =   newStatusChance;
            this->fireRate          =   newFireRate;
            this->ammoCost          =   newAmmoCost;
            this->multishot         =   newMultishot;
            this->burstCount        =   newBurstCount;
            this->burstDelay        =   newBurstDelay;
            this->triggerType       =   newTriggerType;
            this->forcedProcs       =   newForcedProcs;
            for (const auto& damageType : this->damage)
            {
                if (newDamage.find(damageType.first) != newDamage.end())
                {
                    this->damage[damageType.first] = newDamage[damageType.first];
                }
                else
                {
                    this->damage[damageType.first] = 0;
                }
            }

            for (const auto& damageType : newDamage)
            {
                this->totalBaseDamage += damageType.second;
            }


            this->totalIPSDamage = (this->damage["Impact"] + this->damage["Puncture"] + this->damage["Slash"]);
            this->impactPercent = (this->damage["Impact"] / this->totalIPSDamage);
            this->puncturePercent = (this->damage["Puncture"] / this->totalIPSDamage);
            this->slashPercent = (this->damage["Slash"] / this->totalIPSDamage);
        }

        attackData()
        {}
    };



class Weapon
{
    public:
        std::string name = "";                              // name of weapon
        std::string className = "";                         // type of weapon

        std::vector<std::string> compatibilityTags = {};    // tags for compatibility

        std::string weaponFamily = "";                      // weapon family
        std::string triggerType = "";                       // trigger type

        double magazineCapacity = 0;                            // magazine capacity
        double reloadSpeed = 0;                                 // reload speed

        double rivenDispo = 0;  // UNUSED!!! --- Riven Disposition

        double spoolSpeed = 0;                                  // spool rate    (if it exists)

        double reloadDelay = 0;                                 // reload delay  (if it exists) --  charge weapons!
        double reloadRate = 0;                                  // reload speed  (if it exists) --  charge weapons!        

        double comboDuration = 0;                               // melee combo duration (if it exists)
        double heavyAttackDamage = 0;                           // heavy attack damage  (if it exists)

        double statusDamage = 0;                                // status damage (from mods)
        double statusDuration = 0;                              // status duration (from mods)
        double gunCOModifier = 0;                               // Condition Overload modifier type (used in damage calc so number of status on enemy can be used)
        double baseDamageModifier = 0;                          // base damage modifier (not applied here so it can be addative vs multiplicitive with gunCO later on)

        std::vector<std::string> innateUpgrades = {};    // built in effects on the weapon  --  // UNUSED!!! for now, too complex for my MVP

        std::vector<attackData> attackList = {};

        Weapon(std::string newName,
            std::string newClassName,
            std::string newWeaponFamily,
            std::string newTriggerType,
            double newMagazineCapacity,
            double newReloadSpeed,
            double newRivenDisposition,
            double newSpoolSpeed,
            double newReloadDelay,
            double newReloadRate,
            double newComboDuration,
            double newHeavyAttackDamage,
            std::vector<std::string> newCompatibilityTags,
            std::vector<std::string> newInnateUpgrades,
            std::vector<attackData>& newAttackList)
        {
            this->name              =   newName;
            this->className         =   newClassName;
            this->weaponFamily      =   newWeaponFamily;
            this->triggerType       =   newTriggerType;
            this->magazineCapacity  =   newMagazineCapacity;
            this->reloadSpeed       =   newReloadSpeed;
            this->rivenDispo        =   newRivenDisposition;
            this->spoolSpeed        =   newSpoolSpeed;
            this->reloadDelay       =   newReloadDelay;
            this->reloadRate        =   newReloadRate;
            this->comboDuration     =   newComboDuration;
            this->heavyAttackDamage =   newHeavyAttackDamage;
            this->compatibilityTags =   newCompatibilityTags;
            this->innateUpgrades    =   newInnateUpgrades;
            this->attackList        =   newAttackList;

            // fill in data for attack list from weapon

            // fill in data for weapon from attack list

        }

        
        void applyModConfig(weaponModConfig& currentModConfig)
        {
            this->magazineCapacity = (this->magazineCapacity * (1 + (currentModConfig.weaponModifiers["Magazine Capacity"] / 100)));
            this->statusDuration = currentModConfig.weaponModifiers["Status Duration"] / 100;
            this->statusDamage = currentModConfig.weaponModifiers["Status Damage"] / 100;
            this->gunCOModifier = currentModConfig.weaponModifiers["GunCODamage"] / 100;
            this->reloadSpeed = (this->reloadSpeed * (1 + (currentModConfig.weaponModifiers["Reload Speed"] / 100)));
            this->baseDamageModifier = currentModConfig.weaponModifiers["Damage"] / 100;
            for (auto& currentAttack : this->attackList)
            {
                if (currentModConfig.weaponModifiers["isFireRateLocked"] == 0)
                {   // if fire rate is not locked, apply mods
                    currentAttack.fireRate = (currentAttack.fireRate * (1 + (currentModConfig.weaponModifiers["Fire Rate"] / 100)));
                }
                if (currentModConfig.weaponModifiers["isMultishotLocked"] == 0)
                {   // if multishot is not locked, apply mods
                    currentAttack.multishot = (currentAttack.multishot * (1 + (currentModConfig.weaponModifiers["Multishot"] / 100)));
                }
                currentAttack.critChance = (currentAttack.critChance * (1 + (currentModConfig.weaponModifiers["Critical Chance"] / 100)));
                currentAttack.critMultiplier = (currentAttack.critMultiplier * (1 + (currentModConfig.weaponModifiers["Critical Damage"] / 100)));
                currentAttack.statusChance = (currentAttack.statusChance * (1 + (currentModConfig.weaponModifiers["Status Chance"] / 100)));
                for (auto& [damageType, damageAmount] : currentAttack.damage)
                {
                    if (damageType != "Impact" && damageType != "Puncture" && damageType != "Slash" && damageType != "Tau")
                    {
                        currentAttack.damage[damageType] = (currentAttack.damage[damageType] + (currentAttack.totalBaseDamage * ((currentModConfig.statusTypeModifiers[damageType] / 100))));
                    }
                    else if (damageType != "Tau")
                    {
                        currentAttack.damage[damageType] = (currentAttack.damage[damageType] * (1 + (currentModConfig.statusTypeModifiers[damageType] / 100)));
                    }
                    // no tau mods, so it is not considered here, that damage is always base and not affected by elemental mods
                }
            }
        }
        void removeModConfig(weaponModConfig& currentModConfig)
        {
            // make it remove mod effects   -   if still adding instead of removing it is extra indented
            this->magazineCapacity = (this->magazineCapacity / (1 + (currentModConfig.weaponModifiers["Magazine Capacity"] / 100)));
            this->statusDuration = 0;
            this->statusDamage = 0;
            this->gunCOModifier = 0;
            this->reloadSpeed = (this->reloadSpeed / (1 + (currentModConfig.weaponModifiers["Reload Speed"] / 100)));
            this->baseDamageModifier = 0;
            for (auto& currentAttack : this->attackList)
            {
                if (currentModConfig.weaponModifiers["isFireRateLocked"] == 0)
                {   // if fire rate is not locked, apply mods
                    currentAttack.fireRate = (currentAttack.fireRate / (1 + (currentModConfig.weaponModifiers["Fire Rate"] / 100)));
                }
                if (currentModConfig.weaponModifiers["isMultishotLocked"] == 0)
                {   // if multishot is not locked, apply mods
                    currentAttack.multishot = (currentAttack.multishot / (1 + (currentModConfig.weaponModifiers["Multishot"] / 100)));
                }
                currentAttack.critChance = (currentAttack.critChance / (1 + (currentModConfig.weaponModifiers["Critical Chance"] / 100)));
                currentAttack.critMultiplier = (currentAttack.critMultiplier / (1 + (currentModConfig.weaponModifiers["Critical Damage"] / 100)));
                currentAttack.statusChance = (currentAttack.statusChance / (1 + (currentModConfig.weaponModifiers["Status Chance"] / 100)));
                for (auto& [damageType, damageAmount] : currentAttack.damage)
                {
                    if (damageType != "Impact" && damageType != "Puncture" && damageType != "Slash" && damageType != "Tau")
                    {
                        currentAttack.damage[damageType] = (currentAttack.damage[damageType] - (currentAttack.totalBaseDamage * ((currentModConfig.statusTypeModifiers[damageType] / 100))));
                    }
                    else if (damageType != "Tau")
                    {
                        currentAttack.damage[damageType] = (currentAttack.damage[damageType] / (1 + (currentModConfig.statusTypeModifiers[damageType] / 100)));
                    }
                    // no tau mods, so it is not considered here, that damage is always base and not affected by elemental mods
                }
            }
        }
};





// set to be pruned bool (for mods that are useless for dps, or lower variants of other mods)       -       then have the main pass over all entries in validMods vector and drop any that have the pruneThis bool set to true
