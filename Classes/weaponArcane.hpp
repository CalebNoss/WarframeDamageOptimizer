#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <array>

class weaponArcane
{
    public:
        std::string name = "";                              // name of mod                      (always)
        std::string type = "";                              // type of mod                      (always)
        std::vector<std::string> incompatibilityTags = {};  // tags for incompatible weapons    (if they exist)
        std::string description = "";                       // description of mod               (always)

        bool modifiesWeapon = false;
        bool arcaneModifier = false;

        unsigned int weaponModifierMask = 0;
        int arcaneModifierIndex = -1;

        bool pruneThis = false;                             // whether the mod should be pruned (ie: better version exists so don't count in damage calcs, or no affect on damage outcome (for example, ammo max mods))
        
        std::array<double, 13> weaponModifiers = {0};
        // the comment is the string to regex for when parsing a mod description, and which value in weaponModifiers to affect for it
        //Entry 0: "Fire Rate" or "Attack Speed"
        //Entry 1: "Multishot"
        //Entry 2: "Magazine Capacity"
        //Entry 3: "Critical Chance"
        //Entry 4: "Critical Damage"
        //Entry 5: "Damage" or "Melee Damage"
        //Entry 6: "Status Duration"
        //Entry 7: "Status Chance"
        //Entry 8: "Status Damage"
        //Entry 9: "Reload Speed"
        //Entry 10: "Direct Damage per Status Type affecting the target" or "Melee Damage per Status Type affecting the target"
        //Entry 11: "Fire Rate cannot be modified"
        //Entry 12: "Multishot cannot be modified"

        std::array<bool, 15> arcaneBuffs = {false};
        // the comment is the string to regex for when parsing a mod description
        //Entry 0:      //  Secondary Cryogenic :   Secondary    :   "On <DT_PUNCTURE_COLOR>Puncture: Apply 3 <DT_FREEZE_COLOR>Cold stacks on targets within 15m.",
        //Entry 1:      //  Secondary Enervate  :   Secondary    :   "On Hit: Increase Critical Chance by 10%. Resets after 6 Big Critical Hit.",
        //Entry 2:      //  Secondary Encumber  :   Secondary    :   "On Status Effect: +24% chance to trigger a second random Status Effect.",
        //Entry 3:      //  Cascadia Empowered  :   Secondary    :   "On Status Effect:\r\nDeals +750 Damage matching the Damage Type of the Status Effect",
        //Entry 4:      //  Cascadia Accuracy   :   Secondary    :   "On Roll:\r\n+300% Critical Chance on Weakpoint Hits for 4s",
        //Entry 5:      //  Cascadia Flare      :   Secondary    :   "On <DT_FIRE_COLOR>Heat Status Effect:\r\n+12% Damage for 10s. Stacks up to 480%.",
        //Entry 6:      //  Conjunction Voltage :   Secondary    :   "On <DT_ELECTRICITY_COLOR>Electricity Status Effect:\r\n+1.5% Reload Speed and +3% Multishot for 12s. Stacks up to 40x.",
        //Entry 7:      //  Primary Blight      :   Primary      :   "On <DT_POISON_COLOR>Toxin Status Effect:\r\n+3.6% Critical Damage and +1.8% Multishot for 12s. Stacks up to 40x.",
        //Entry 8:      //  Primary Frostbite   :   Primary      :   "On <DT_FREEZE_COLOR>Cold Status Effect:\r\n+3% Critical Damage and +2.25% Multishot for 12s. Stacks up to 40x.",
        //Entry 9:      //  Primary Plated Round:   Primary      :   "On Reload: Deal increased damage per round loaded based on max magazine size. Lasts for 10s.",
        //Entry 10:     //  Melee Duplicate     :   Melee        :   "On Base Critical Hits: 100% chance for your attack to strike a second time",
        //Entry 11:     //  Melee Exposure      :   Melee        :   "On Ability Cast: Gain 60% Corrosive Damage on Melee strikes for 25s. Stacks up to 240%.",
        //Entry 12:     //  Secondary Shiver    :   Secondary    :   "Enemies take +45% damage per <DT_FREEZE_COLOR>Cold Status"
        //Entry 13:     //  Melee Careen        :   Melee        :   "x2.50 Melee Damage against Frozen enemies. On Roll: <DT_FREEZE_COLOR> Freeze enemies in a 5.5m radius with a 2s Cooldown."
        //Entry 14:     //  Melee Doughty       :   Melee        :   "Gain 1.0x Critical Damage for every 10% <DT_PUNCTURE_COLOR>Puncture Status chance on your Melee Weapon."


        // Having these be in a separate implementation file was causing weird errors, so they are defined here instead.



        // arcane mod constructor
        weaponArcane(std::string newName, std::string newType, std::string newDescription, std::vector<std::string> newIncompatibilityTags)
        {
            this->name = newName;
            this->type = newType;
            this->description = newDescription;
            this->incompatibilityTags = newIncompatibilityTags;

            if (this->description[0] == 'O')
            {
                if (this->type[0] == 'S')
                {
                    if (this->type[1] == 'e')
                    {
                        if (this->name[0] == 'S')
                        {
                            if (this->name[10] == 'M')
                            {
                                //  Secondary Merciless :   Secondary    :   "On Kill:\r\n+30% Damage for 4s. Stacks up to 12x.\r\n+30% Reload Speed",
                                this->weaponModifiers[5] = 360; // Assume max buff
                                this->weaponModifiers[9] = 30;
                                this->modifiesWeapon = true;
                            }
                            else if (this->description[3] == 'P')
                            {
                                //  Secondary Deadhead  :   Secondary    :   "On Precision Headshot Kill:\r\n+120% Damage for 24s. Stacks up to 3x.\r\n+30% to Headshot Multiplier\r\n-50% Weapon Recoil",
                                this->pruneThis = true;
                                // will always be matched in damage by merciless, other than headshot bonus but that isn't really tracked... so it gets prunes
                            }
                            else if (this->description[3] == '<')
                            {
                                //  Secondary Cryogenic :   Secondary    :   "On <DT_PUNCTURE_COLOR>Puncture: Apply 3 <DT_FREEZE_COLOR>Cold stacks on targets within 15m.",
                                this->arcaneBuffs[0] = true;
                                this->arcaneModifier = true;
                            }
                            else if (this->description[3] == 'H')
                            {
                                //  Secondary Enervate  :   Secondary    :   "On Hit: Increase Critical Chance by 10%. Resets after 6 Big Critical Hit.",
                                this->arcaneBuffs[1] = true;
                                this->arcaneModifier = true;
                            }
                            else if (this->description[3] == 'S')
                            {
                                //  Secondary Encumber  :   Secondary    :   "On Status Effect: +24% chance to trigger a second random Status Effect.",
                                this->arcaneBuffs[2] = true;
                                this->arcaneModifier = true;
                            }
                            else
                            {
                                //  Secondary Surge     :   Secondary    :   "On Ability Cast: Next shot gains a Damage Multiplier for every 200 current Energy, up to x8.",
                                //  Secondary Irradiate :   Secondary    :   "On hitting enemies afflicted by 10 stacks of <DT_RADIATION_COLOR>Radiation: Deal 180% of the hit damage to enemies within 7m.",
                                //  Secondary Dexterity :   Secondary    :   "On Melee Kill:\r\n+60% Damage for 20s. Stacks up to 6x.\r\n+7.5s Combo Duration\r\n",
                                //  Secondary Outburst  :   Secondary    :   "On swapping to Secondary Weapon, consume all Combo Multipliers to increase Secondary Weapon Critical Chance and Critical Damage by 20% per Combo consumed for 30s.",
                                this->pruneThis = true;     // ^ basically can't track these so...
                            }
                        }
                        else
                        {
                            if (this->name[9] == 'E')
                            {
                                //  Cascadia Empowered  :   Secondary    :   "On Status Effect:\r\nDeals +750 Damage matching the Damage Type of the Status Effect",
                                this->arcaneBuffs[3] = true;
                                this->arcaneModifier = true;
                            }
                            else if (this->name[9] == 'A')
                            {
                                //  Cascadia Accuracy   :   Secondary    :   "On Roll:\r\n+300% Critical Chance on Weakpoint Hits for 4s",
                                this->arcaneBuffs[4] = true;
                                this->arcaneModifier = true;
                            }
                            else if (this->name[9] == 'F')
                            {
                                //  Cascadia Flare      :   Secondary    :   "On <DT_FIRE_COLOR>Heat Status Effect:\r\n+12% Damage for 10s. Stacks up to 480%.",
                                this->arcaneBuffs[5] = true;
                                this->arcaneModifier = true;
                            }
                            else
                            {
                                //  Conjunction Voltage :   Secondary    :   "On <DT_ELECTRICITY_COLOR>Electricity Status Effect:\r\n+1.5% Reload Speed and +3% Multishot for 12s. Stacks up to 40x.",
                                this->arcaneBuffs[6] = true;
                                this->arcaneModifier = true;
                            }
                        }
                    }
                    else
                    {
                        //  Shotgun Vendetta    :   Shotgun    :   "On shotgun kill within 5m of target:\r\n+180% Multishot and +75% Reload Speed for 15s.",
                        this->weaponModifiers[1] = 180;
                        this->weaponModifiers[9] = 75;
                        this->modifiesWeapon = true;
                    }
                }
                else if (this->type[0] == 'P')
                {
                    if (this->name[8] == 'B')
                    {
                        //  Primary Blight      :   Primary    :   "On <DT_POISON_COLOR>Toxin Status Effect:\r\n+3.6% Critical Damage and +1.8% Multishot for 12s. Stacks up to 40x.",
                        this->arcaneBuffs[7] = true;
                        this->arcaneModifier = true;
                    }
                    else if (this->name[8] == 'F')
                    {
                        //  Primary Frostbite   :   Primary    :   "On <DT_FREEZE_COLOR>Cold Status Effect:\r\n+3% Critical Damage and +2.25% Multishot for 12s. Stacks up to 40x.",
                        this->arcaneBuffs[8] = true;
                        this->arcaneModifier = true;
                    }
                    else if (this->name[8] == 'M')
                    {
                        //  Primary Merciless   :   Primary    :   "On Kill:\r\n+30% Damage for 4s. Stacks up to 12x.\r\n+30% Reload Speed",
                        this->weaponModifiers[5] = 360;
                        this->weaponModifiers[9] = 30;
                        this->modifiesWeapon = true;
                    }
                    else if (this->name[8] == 'P')
                    {
                        //  Primary Plated Round:   Primary    :   "On Reload: Deal increased damage per round loaded based on max magazine size. Lasts for 10s.",
                        this->arcaneBuffs[9] = true;
                        this->arcaneModifier = true;
                    }
                    else if (this->name[2] == 'a')
                    {
                        //  Fractalized Reset   :   Primary    :   "On Ability Cast:\r\n+240% Reload Speed for 5s",
                        this->weaponModifiers[9] = 240; //  Assume max buff
                        this->modifiesWeapon = true;
                    }
                    else
                    {
                        //  Primary Compression :   Primary    :   "On aim: x0.2 explosion radius, +100% damage and +5.5% ammo efficiency for every 1m radius lost.",
                        // ^ I don't even know how I would implement this for dps calcs
                        //  Primary Crux        :   Primary    :   "On Weak Point Hit: Gain +30% Status Chance and +6% Ammo Efficiency for 10s. Stacks up to 10x.",
                        // ^ too niche TODO: make weak points optional and let user choose
                        //  Primary Deadhead    :   Primary    :   "On Headshot Kill:\r\n+120% Damage for 24s. Stacks up to 3x.\r\n+30% to Headshot Multiplier\r\n-50% Weapon Recoil",
                        // ^ outclassed by merciless except for headshot multiplier, which isn't really tracked TODO: let user choose headshot build or not
                        //  Primary Dexterity   :   Primary    :   "On Melee Kill:\r\n+60% Damage for 20s. Stacks up to 6x.\r\n+7.5s Combo Duration",
                        // ^ melee requirement, too niche, can't properly track
                        //  Primary Exhilarate  :   Primary    :   "On Weapon <DT_IMPACT_COLOR>Impact Status Effect:\r\n+1.2 Energy Regen\/s for 10s. Stacks up to 3x.",                        this->arcaneBuffs[] = true;
                        // ^ QOL for your warframe, no bonus to DPS
                        //  Primary Obstruct    :   Primary    :   "On Weapon <DT_MAGNETIC_COLOR>Magnetic Status Effect: Enemy weapons jam within 15m of target. Cooldown 10s.",
                        // ^ QOL for gameplay, no bonus to DPS
                        this->pruneThis = true;
                    }
                }
                else if (this->type[0] == 'M')
                {
                    if (this->name[6] == 'D')
                    {
                        //  Melee Duplicate     :   Melee    :   "On Base Critical Hits: 100% chance for your attack to strike a second time",
                        this->arcaneBuffs[10] = true;
                        this->arcaneModifier = true;
                    }
                    else if (this->name[6] == 'E')
                    {
                        //  Melee Exposure      :   Melee    :   "On Ability Cast: Gain 60% Corrosive Damage on Melee strikes for 25s. Stacks up to 240%.",
                        this->arcaneBuffs[11] = true;
                        this->arcaneModifier = true;
                    }
                    else
                    {
                        //  Melee Fortification :   Melee    :   "On Melee Kill: +210 Armor for 10s",
                        // ^ Warframe QOL, no buff to DPS
                        //  Melee Crescendo     :   Melee    :   "On Finisher Kill: Gain 6 Initial Combo for the rest of your mission",
                        // ^ How would I even track this, max stacks because infinite missions, or none because of capture missions... too variable
                        //  Melee Assimilation  :   Melee    :   "On Shield Break: +150% Melee Damage on Heavy Attack and Heavy Attack kills restore +30% of max Shields for 20s.",
                        // ^ Would you assume constant shield gating or never breaking shields? what about inaros? too niche
                        //  Melee Animosity     :   Melee    :   "On Melee Hit: Gain 42% Critical Chance on your next Heavy Attack, up to 420%",
                        // ^ Just the thought of tracking crit chance for heavy attacks specifically, and how many normal hits you need to do first seems like pure torture. Like, farm for the invinicible title again, or that xenorhat honoria again torture

                        //  Melee Influence     :   Melee    :   "On Melee <DT_ELECTRICITY_COLOR>Electricity Status: 20% chance for elemental Melee Status Effects to apply to enemies within 20m for 18s. Cannot refresh while active.",
                        // ^ And here is the downside of optimizing for single target, this can't really be tracked.. would multiply calculation complexity a crazy amount. Also, do you go for steel path density, or normal? E Prime, or an omni fissure on Lua?
                        this->pruneThis = true;
                    }
                }
                else
                {
                    //  Longbow Sharpshot   :   Bow    :   "On Headshot: Gain +300% damage on your next shot.",
                    // too niche for now TODO: make user choice and implement ^
                    this->pruneThis = true;
                }
            }
            else
            {   
                if (this->type[0] == 'P')
                {
                    if (this->description[0] == 'W')
                    {
                        //  Primary Overcharge      :   Primary     :   "While at or above 90% Energy: Gain 35% of Max Energy as Multishot, capped at 350%."
                        this->weaponModifiers[1] = 350; //  Assume max buff
                        this->modifiesWeapon = true;
                    }
                    else if (this->description[0] == 'G')
                    {
                        //  Primary Bulwark         :   Primary     :   "Gain +1% damage for each unit of armor past 1,000, up to a max of +500%."
                        this->weaponModifiers[5] = 500; //  Assume max buff
                        this->modifiesWeapon = true;
                    }
                    else
                    {
                        //  Primary Debilitate      :   Primary     :   "If an enemy has 10 stacks of a combined Status Effect, inflicting the same Status Effect again has a 100% chance to inflict one of the base Status Effects it is composed of."
                        this->pruneThis = true;
                        // too niche to calc for properly
                    }
                
                }
                else if (this->type[0] == 'S')
                {
                    if (this->name[1] == 'a')
                    {
                        //  Cascadia Overcharge     :   Secondary   :   "While Overshields Active: +300% Critical Chance"
                        this->weaponModifiers[3] = 300; //  Assume max buff
                        this->modifiesWeapon = true;
                    }
                    else if (this->name[10] == 'S')
                    {
                        //  Secondary Shiver        :   Secondary   :   "Enemies take +45% damage per <DT_FREEZE_COLOR>Cold Status"
                        this->arcaneBuffs[12] = true;
                        this->arcaneModifier = true;
                    }
                    else
                    {
                        //  Secondary Fortifier     :   Secondary   :   "Gain 1 Overguard for every 100 Damage dealt to an enemy's Overguard.\r\nDeals x8 Extra Damage to Overguard."
                        // ^ not building for overguard, just normal enemies
                        //  Secondary Kinship       :   Secondary   :   "While Buffing Ally Warframes: +20% Critical Chance per buff"
                        // ^ no way to track
                        //  Akimbo Slip Shot        :   Secondary   :   "While sliding or aim gliding: Gain 65% ammo efficiency with Dual Pistols."
                        // ^ too niche to track
                        this->pruneThis = true;
                    }
                }
                else if (this->type[0] == 'M')
                {
                    if (this->name[6] == 'R')
                    {
                        //  Melee Retaliation       :   Melee       :   "Gain 30% Melee Damage for every 200 current Shields, up to 420%. Bonus halved for Overshields."
                        this->weaponModifiers[5] = 420; //  Assume max buff
                        this->modifiesWeapon = true;
                    }
                    else
                    {
                        if (this->name[6] == 'C')
                        {
                            //  Melee Careen            :   Melee       :   "x2.50 Melee Damage against Frozen enemies. On Roll: <DT_FREEZE_COLOR> Freeze enemies in a 5.5m radius with a 2s Cooldown."
                            this->arcaneBuffs[13] = true;
                            this->arcaneModifier = true;
                        }
                        else if (this->name[6] == 'D')
                        {
                            //  Melee Doughty           :   Melee       :   "Gain 1.0x Critical Damage for every 10% <DT_PUNCTURE_COLOR>Puncture Status chance on your Melee Weapon."
                            this->arcaneBuffs[14] = true;
                            this->arcaneModifier = true;
                        }
                        else
                        {
                            //  Melee Afflictions       :   Melee       :   "Enemies affected by Status Effects gain 6 additional stacks when they're knocked down or flung by melee attacks."
                            // ^ No real way to track how often you slam
                            //  Melee Vortex            :   Melee       :   "Kill an enemy affected by <DT_MAGNETIC_COLOR>Magnetic Status for a 45% chance to pull in enemies within 18m radius"
                            // ^ Not real way to track how useful this is
                            this->pruneThis = true;
                        }
                    }
                }
            }

            if (modifiesWeapon)
            {
                for (int weaponUpgradeIndex = 0; weaponUpgradeIndex < 13; weaponUpgradeIndex++)
                {
                    if (this->weaponModifiers.at(weaponUpgradeIndex) != 0)
                    {
                        this->weaponModifierMask |= (1 << weaponUpgradeIndex);
                        this->weaponModifiers[weaponUpgradeIndex] = this->weaponModifiers[weaponUpgradeIndex] * 0.01;
                    }
                }
            }
            else
            {
                for (int arcaneUpgradeIndex = 0; arcaneUpgradeIndex < 15; arcaneUpgradeIndex++)
                {
                    if (this->arcaneBuffs.at(arcaneUpgradeIndex))
                    {
                        this->arcaneModifierIndex = arcaneUpgradeIndex;
                    }
                }
            }
        }
};





// set to be pruned bool (for mods that are useless for dps, or lower variants of other mods)       -       then have the main pass over all entries in validMods vector and drop any that have the pruneThis bool set to true
