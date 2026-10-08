#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>


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

        double multishot = 1;

        double burstCount = 0;
        double burstDelay = 0;

        double chargeTime = 0;

        double totalBaseDamage = 0;

        double totalIPSDamage = 0;
        double impactPercent = 0;
        double puncturePercent = 0;
        double slashPercent = 0;

        int triggerTypeIndex = 0;

        std::vector<std::string> forcedProcs = {};

        alignas(64) std::array<double, 16> damage = {0};
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

        // list of indices of which types actually have a damage value
        unsigned int damageTypesMask = 0;
        unsigned int baseDamageTypesMask = 0;

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
            double newChargeTime,
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
            this->chargeTime        =   newChargeTime;
            this->forcedProcs       =   newForcedProcs;
            for (const auto& damageType : newDamage)
            {
                int damageTypeIndex = damageTypeNames[damageType.first];
                this->damage[damageTypeIndex] = damageType.second;
                this->baseDamageTypesMask |= (1 << damageTypeIndex);
            }

            for (const auto& damageType : newDamage)
            {
                this->totalBaseDamage += damageType.second;
            }

            if (!newTriggerType.empty())
            {
                if (newTriggerType.at(0) == 'C')
                {   // Charge
                    this->triggerTypeIndex = 1;
                }
                else if (newTriggerType.size() == 5 ||  // Burst or Auto-Burst
                (newTriggerType.size() == 10 && newTriggerType.at(5) == 'B'))
                {
                    this->triggerTypeIndex = 2;
                }
            }


            this->totalIPSDamage    = (this->damage[0] + this->damage[1] + this->damage[2]);
            this->impactPercent     = (this->damage[0] / this->totalIPSDamage);
            this->puncturePercent   = (this->damage[1] / this->totalIPSDamage);
            this->slashPercent      = (this->damage[2] / this->totalIPSDamage);
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
        int triggerTypeIndex        = 0;                    // trigger type index (1 == charge) (2 == burst/auto-burst) (0 == anything else)

        double magazineCapacity     = 0;                          // magazine capacity
        double reloadSpeed  = 0;                                  // reload speed

        double rivenDispo   = 0;  // UNUSED!!! --- Riven Disposition

        double spoolSpeed   = 0;                                  // spool rate    (if it exists)

        double reloadDelay  = 0;                                  // reload delay  (if it exists) --  charge weapons!
        double reloadRate   = 0;                                  // reload speed  (if it exists) --  charge weapons!        

        double comboDuration        = 0;                          // melee combo duration (if it exists)
        double heavyAttackDamage    = 0;                          // heavy attack damage  (if it exists)

        std::vector<std::string> innateUpgrades = {};    // built in effects on the weapon  --  // UNUSED!!! for now, too complex for my MVP

        std::vector<attackData> attackList = {};

        alignas(64) std::array<double, 3> savedStats = {0};
        std::vector<std::array<double, 5>> savedAttackStats     = {{0}};
        std::vector<std::array<double, 16>> savedAttackDamages  = {{0}};

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

            if (!newTriggerType.empty())
            {
                if (newTriggerType.at(0) == 'C')
                {   // Charge
                    this->triggerTypeIndex = 1;
                }
                else if (newTriggerType.size() == 5 ||  // Burst or Auto-Burst
                (newTriggerType.size() == 10 && newTriggerType.at(5) == 'B'))
                {
                    this->triggerTypeIndex = 2;
                }
            }


            // fill in data for attack list from weapon

            // fill in data for weapon from attack list

            for (auto& currentAttack : this->attackList)
            {
                if (this->triggerTypeIndex && (!currentAttack.triggerTypeIndex))
                {   // If charge/burst weapon but not for attack type, carry trigger type down
                    currentAttack.triggerTypeIndex = this->triggerTypeIndex;
                }
            }
        }
};





// set to be pruned bool (for mods that are useless for dps, or lower variants of other mods)       -       then have the main pass over all entries in validMods vector and drop any that have the pruneThis bool set to true
