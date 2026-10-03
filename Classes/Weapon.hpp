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

        std::unordered_map<std::string, int> damageTypeNames = {
            {"Impact", 0},
            {"Puncture", 1},
            {"Slash", 2},
            {"Heat", 3},
            {"Cold", 4},
            {"Electricity", 5},
            {"Toxin", 6},
            {"Blast", 7},
            {"Corrosive", 8},
            {"Gas", 9},           
            {"Magnetic", 10},
            {"Radiation", 11},
            {"Viral", 12},
            {"Tau", 13}
        };


        std::array<double, 14> damage = {0};
        // the comment is the string to regex for when parsing a mod description
        //Entry 0: {"Impact", 0},              // "<DT_IMPACT_COLOR>Impact"
        //Entry 1: {"Puncture", 0},            // "<DT_PUNCTURE_COLOR>Puncture"
        //Entry 2: {"Slash", 0},               // "<DT_SLASH_COLOR>Slash"
        //Entry 3: {"Heat", 0},                // "<DT_FIRE_COLOR>Heat"
        //Entry 4: {"Cold", 0},                // "<DT_FREEZE_COLOR>Cold"
        //Entry 5: {"Electricity", 0},         // "<DT_ELECTRICITY_COLOR>Electricity"
        //Entry 6: {"Toxin", 0},               // "<DT_POISON_COLOR>Toxin"
        //Entry 7: {"Blast", 0},               // "<DT_EXPLOSION_COLOR>Blast"
        //Entry 8: {"Corrosive", 0},           // "<DT_CORROSIVE_COLOR>Corrosive"
        //Entry 9: {"Gas", 0},                 // "<DT_GAS_COLOR>Gas"
        //Entry 10: {"Magnetic", 0},            // "<DT_MAGNETIC_COLOR>Magnetic"
        //Entry 11: {"Radiation", 0},           // "<DT_RADIATION_COLOR>Radiation"
        //Entry 12: {"Viral", 0},                // "<DT_VIRAL_COLOR>Viral"
        //Entry 13: {"Tau", 0}                  // Something about tau, IDK right now

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
            for (const auto& damageType : newDamage)
            {
                int damageTypeIndex = damageTypeNames[damageType.first];
                this->damage[damageTypeIndex] = damageType.second;
            }

            for (const auto& damageType : newDamage)
            {
                this->totalBaseDamage += damageType.second;
            }


            this->totalIPSDamage = (this->damage[0] + this->damage[1] + this->damage[2]);
            this->impactPercent = (this->damage[0] / this->totalIPSDamage);
            this->puncturePercent = (this->damage[1] / this->totalIPSDamage);
            this->slashPercent = (this->damage[2] / this->totalIPSDamage);
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

        
        [[msvc::noinline]] void applyModConfig(weaponModConfig& currentModConfig)
        {
            this->magazineCapacity = (this->magazineCapacity * (1 + (currentModConfig.weaponModifiers[2] * 0.01)));
            this->statusDuration = currentModConfig.weaponModifiers[6] * 0.01;
            this->statusDamage = currentModConfig.weaponModifiers[8] * 0.01;
            this->gunCOModifier = currentModConfig.weaponModifiers[10] * 0.01;
            this->reloadSpeed = (this->reloadSpeed * (1 + (currentModConfig.weaponModifiers[9] * 0.01)));
            this->baseDamageModifier = currentModConfig.weaponModifiers[5] * 0.01;
            for (auto& currentAttack : this->attackList)
            {
                if (currentModConfig.weaponModifiers[11] == 0)
                {   // if fire rate is not locked, apply mods
                    currentAttack.fireRate = (currentAttack.fireRate * (1 + (currentModConfig.weaponModifiers[0] * 0.01)));
                }
                if (currentModConfig.weaponModifiers[12] == 0)
                {   // if multishot is not locked, apply mods
                    currentAttack.multishot = (currentAttack.multishot * (1 + (currentModConfig.weaponModifiers[1] * 0.01)));
                }
                currentAttack.critChance = (currentAttack.critChance * (1 + (currentModConfig.weaponModifiers[3] * 0.01)));
                currentAttack.critMultiplier = (currentAttack.critMultiplier * (1 + (currentModConfig.weaponModifiers[4] * 0.01)));
                currentAttack.statusChance = (currentAttack.statusChance * (1 + (currentModConfig.weaponModifiers[7] * 0.01)));
                double totalBaseDamage = currentAttack.totalBaseDamage;
                for (int i = 0; i < 3; i++)
                {   // for IPS damage types
                    currentAttack.damage[i] = (currentAttack.damage[i] * (1 + (currentModConfig.statusTypeModifiers[i] * 0.01)));
                }
                for (int i = 3; i < 13; i++)
                {   // for non IPS types (other than Tau, no mod for that exists (yet))
                    currentAttack.damage[i] = (currentAttack.damage[i] + (totalBaseDamage * ((currentModConfig.statusTypeModifiers[i] * 0.01))));
                }
            }
        }
        [[msvc::noinline]] void removeModConfig(weaponModConfig& currentModConfig)
        {
            // make it remove mod effects   -   if still adding instead of removing it is extra indented
            this->magazineCapacity = (this->magazineCapacity * (1 / (1 + (currentModConfig.weaponModifiers[2] * 0.01))));
            this->statusDuration = 0;
            this->statusDamage = 0;
            this->gunCOModifier = 0;
            this->reloadSpeed = (this->reloadSpeed * (1 / (1 + (currentModConfig.weaponModifiers[9] * 0.01))));
            this->baseDamageModifier = 0;
            for (auto& currentAttack : this->attackList)
            {
                if (currentModConfig.weaponModifiers[11] == 0)
                {   // if fire rate is not locked, apply mods
                    currentAttack.fireRate = (currentAttack.fireRate * (1 / (1 + (currentModConfig.weaponModifiers[0] * 0.01))));
                }
                if (currentModConfig.weaponModifiers[12] == 0)
                {   // if multishot is not locked, apply mods
                    currentAttack.multishot = (currentAttack.multishot * (1 / (1 + (currentModConfig.weaponModifiers[1] * 0.01))));
                }
                currentAttack.critChance = (currentAttack.critChance * (1 / (1 + (currentModConfig.weaponModifiers[3] * 0.01))));
                currentAttack.critMultiplier = (currentAttack.critMultiplier * (1 / (1 + (currentModConfig.weaponModifiers[4] * 0.01))));
                currentAttack.statusChance = (currentAttack.statusChance * (1 / (1 + (currentModConfig.weaponModifiers[7] * 0.01))));
                double totalBaseDamage = currentAttack.totalBaseDamage;
                for (int i = 0; i < 3; i++)
                {   // for IPS damage types
                    currentAttack.damage[i] = (currentAttack.damage[i] * (1 / (1 + (currentModConfig.statusTypeModifiers[i] * 0.01))));
                }
                for (int i = 3; i < 13; i++)
                {   // for non IPS types (other than Tau, no mod for that exists (yet))
                    currentAttack.damage[i] = (currentAttack.damage[i] - (totalBaseDamage * ((currentModConfig.statusTypeModifiers[i] * 0.01))));
                }
            }
        }
};





// set to be pruned bool (for mods that are useless for dps, or lower variants of other mods)       -       then have the main pass over all entries in validMods vector and drop any that have the pruneThis bool set to true
