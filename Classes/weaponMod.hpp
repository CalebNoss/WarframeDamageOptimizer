#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <array>


class weaponMod
{
    public:
        std::string name = "";                              // name of mod                      (always)
        std::string type = "";                              // type of mod                      (always)
        std::vector<std::string> incompatibilityTags = {};  // tags for incompatible weapons    (if they exist)
        std::vector<std::string> upgradeTypes = {};         // values affected                  (always)
        std::string description = "";                       // description of mod               (always)
        std::string setName = "";                           // set name                         (if it exists)
        std::vector<std::string> incompatibleMods = {};     // incompatible mods                (if they exist)
        std::vector<int> incompatibleModIndices = {};       // incompatible mod indices         (if they exist)
        std::string className = "";                         // class name                       (if it exists)
        int indexInValidMods = -1;

        bool pruneThis = false;                             // whether the mod should be pruned (ie: better version exists so don't count in damage calcs, or no affect on damage outcome (for example, ammo max mods))
        bool modifiesWeapon = false;
        bool modifiesElements = false;
        bool isElementalMod = false;

        // list of the index for each entry that is worth something
        unsigned int weaponModifierMask = 0;

        // list of the index for each entry that is worth something
        unsigned int modifiedElementIndicesMask = 0;


        alignas(64) std::array<double, 16> weaponModifiers = {0};
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
        
        // "Fire Rate cannot be modified"
        bool locksFireRate = false;
        // "Multishot cannot be modified"
        bool locksMultishot = false;

        alignas(64) std::array<double, 16> statusTypeModifiers = {0};
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
        //Entry 12: {"Viral", 0}                // "<DT_VIRAL_COLOR>Viral"


        // Having these be in a separate implementation file was causing weird errors, so they are defined here instead.

        std::vector<std::string> valuableUpgradeTypes = {
            "WEAPON_DAMAGE_AMOUNT",
            "WEAPON_MELEE_DAMAGE",
            "WEAPON_MELEE_SLAM_DAMAGE_BONUS",
            "WEAPON_CRIT_CHANCE",
            "WEAPON_CRIT_DAMAGE",
            "WEAPON_FIRE_RATE",
            "WEAPON_PERCENT_BASE_DAMAGE_ADDED",
            "WEAPON_FIRE_ITERATIONS",
            "WEAPON_CLIP_MAX",
            "WEAPON_RELOAD_SPEED",
            "WEAPON_PROC_TIME",
            "WEAPON_PROC_CHANCE",
            "WEAPON_PROC_DAMAGE",
            "WEAPON_DAMAGE_IF_VICTIM_PROC_ACTIVE"
        };



        weaponMod(std::string newName, std::string newType, std::string newDescription, std::string newSetName, std::string newClassName, std::vector<std::string> newIncompatibilityTags, std::vector<std::string> newIncompatibleMods, std::vector<std::string> newUpgradeTypes, int& currentValidModsIndex) // make this constructor create a mod based on passed in data. I need to decide how to pass in the data from the json though
        {
            this->name = newName;
            this->type = newType;
            this->description = newDescription;
            this->setName = newSetName;
            this->incompatibilityTags = newIncompatibilityTags;
            this->incompatibleMods = newIncompatibleMods;
            this->upgradeTypes = newUpgradeTypes;
            this->className = newClassName;

            bool isUsefulMod = false;

            for (auto& upgradeType : newUpgradeTypes)
            {
                // if a mod does not affect a notable stat, move to prune it for less mods to consider/compute, lowers time & space complexity
                auto valuableUpgradeTypeThatExists = std::find(valuableUpgradeTypes.begin(), valuableUpgradeTypes.end(), upgradeType);
                if (valuableUpgradeTypeThatExists != valuableUpgradeTypes.end())
                {
                    isUsefulMod = true;
                }
            }
            for (std::string& incompatibleMod : incompatibleMods)
            {
                if (incompatibleMod.find("Primed ") == 0)
                {   //  is incompatible with a primed version of that mod, which will always be better, so just skip this
                    isUsefulMod = false;
                }
            }


            if ((isUsefulMod == false) ||   // currently working with no augment mods, if that is true, no useful mod does not start with either a '+', '-', or 'O'
            (this->description[0] != '+' && // not +, -, O(for 'Only compatible with Semi-Auto Trigger', 'On Melee Kill', etc.), 
            this->description[0] != '-' &&  // define mods that are zoom only or stuff, anything that doesn't affect tracked info
            this->description[0] != 'O'))   // this should prune most mods that are not useful.
            {
                this->pruneThis = true;
            }
            else
            {
                // Parse 'Only' and 'On' mods first
                if (this->description[0] == 'O')
                {
                    this->modifiesWeapon = true;
                    if (this->description.find("Only") == 0) // finds if the first match is at index 0
                    {   // because no augment mods the only ones starting with this are cannonade mods (no need to worry about Efficient Beams)
                        if (this->type == "Pistol")
                        {   // pistol has higher stats than others, so seperate that
                            this->weaponModifiers[5] = 300;
                        } else
                        {
                            this->weaponModifiers[5] = 240;
                        }
                        this->locksFireRate = true;
                    } else if (this->description.find("On ") == 0) // finds if the first match is at 0
                    {
                        /// List of mods to parse
                        /// Berserker Fury: "On Melee Kill:\r\n+35% Attack Speed for 10s. Stacks up to 2x."
                        if (this->description.find("On M") == 0) // Either Combo Fury or Berserker Fury
                        {
                            if (this->type != "Thrown Melee")
                            {   // Berserker Fury
                                this->weaponModifiers[0] = 70;
                            } else
                            {   // Combo Fury
                                this->pruneThis = true;
                            }
                        }
                        /// Nano-Applicator: "On Ability Cast:\r\n+90% Status Chance when Aiming for 9s"
                        /// Embedded Catalyzer: "On Ability Cast:\r\n+90% Status Chance when Aiming for 9s"
                        /// Catalyzer Link: "On Ability Cast:\r\n+60% Status Chance when Aiming for 9s
                        else if (this->description.find("On A") == 0) // No Auras so it isn't Brief Respite
                        {
                            if (this->type == "Rifle")
                            {   // Catalyzer Link
                                this->weaponModifiers[7] = 60;
                            } else
                            {   // Nano-Applicator or Embedded Catalyzer
                                this->weaponModifiers[7] = 90;
                            }
                        }
                        /// Hydraulic Crosshairs: "On Headshot:\r\n+135% Critical Chance when Aiming for 9s"
                        /// Argon Scope: "On Headshot:\r\n+135% Critical Chance when Aiming for 9s"
                        /// Laser Sight: "On Headshot:\r\n+120% Critical Chance when Aiming for 9s"
                        /// Galvanized Scope: "On Headshot:\r\n+120% Critical Chance when Aiming for 12s\r\nOn Headshot Kill:\r\n+40% Critical Chance when Aiming for 12s. Stacks up to 5x."
                        /// Galvanized Crosshairs: "On Headshot:\r\n+120% Critical Chance when Aiming for 12s\r\nOn Headshot Kill:\r\n+40% Critical Chance when Aiming for 12s. Stacks up to 5x."
                        else if (this->description.find("On Head") == 0)
                        {
                            // all on headshots, too specific
                            this->pruneThis = true;
                            // if (this->className == "Galvanized")
                            // {   // Galvanized Scope and/or Galvanized Crosshairs
                            //     this->weaponModifiers[3] = 320;
                            // }
                            // else if (this->type == "Shotgun")
                            // {   // Laser Sight
                            //     this->weaponModifiers[3] = 120;
                            // } else
                            // {   // Hydraulic Crosshairs or Argon Scope
                            //     this->weaponModifiers[3] = 135;
                            // }
                        }
                        /// Split Flights: "On Hit:\r\n+100% Multishot\r\n-180% Accuracy for 2s. Stacks up to 4x.\r\n(Non-AOE Bows)"
                        else if (this->description.find("On Hit") == 0)
                        {   // Narrow Barrel, Targeting Subsystem, Guided Ordnance, Night Stalker, Bounty Hunter, and Apex Predator get removed above | Double Tap and Velox Conclusion are augments so they don't get here either
                            if (this->name == "Split Flights")
                            {   // Split Flights
                                this->weaponModifiers[1] = 400;
                            } else
                            {   // Plan B
                                this->pruneThis = true;
                            }
                        }
                        /// Repeater Clip: "On Reload:\r\n+105% Fire Rate when Aiming for 9s"
                        /// Spring-Loaded Chamber: "On Reload:\r\n+75% Fire Rate when Aiming for 9s"
                        /// Pressurized Magazine: "On Reload:\r\n+90% Fire Rate when Aiming for 9s"
                        else if (this->description.find("On Rel") == 0)
                        {   // No Archgun so not Deadly Efficiency/Primed Deadly Efficiency | No Augments so not Clip Delegation
                            if (this->type == "Assault Rifle")
                            {   // Spring-Loaded Chamber
                                this->weaponModifiers[0] = 75;
                            } else if (this->type == "Shotgun")
                            {   // Repeater Clip
                                this->weaponModifiers[0] = 105;
                            } else
                            {   // Pressurized Magazine
                                this->weaponModifiers[0] = 90;
                            }
                        }
                        /// Sharpened Bullets: "On Kill:\r\n+75% Critical Damage when Aiming for 9s"
                        /// Shrapnel Shot: "On Kill:\r\n+99% Critical Damage when Aiming for 9s"
                        /// Bladed Rounds: "On Kill:\r\n+120% Critical Damage when Aiming for 9s"
                        /// Secondary Wind: "On Kill:\r\n+50% Reload Speed for 4s"
                        /// Emergent Aftermath: "On Kill:\r\n+50% Reload Speed for 3s"
                        /// Kill Switch: "On Kill:\r\n+50% Reload Speed for 3s"
                        else if (this->description.find("On K") == 0) // No Augments so not Gorgon Frenzy | Relentless Assault, Momentary Pause, Recover, Recuperate, Vanquished Prey, Prize Kill, and Calculated Victory all get pruned before hitting this
                        {
                            if (this->upgradeTypes[0] == "WEAPON_CRIT_DAMAGE")
                            { // Sharpened Bullets, Shrapnel Shot and Bladed Rounds
                                if (this->type == "Rifle")
                                { // Bladed Rounds
                                    this->weaponModifiers[4] = 120;
                                } else if (this->type == "Shotgun")
                                { // Shrapnel Shot
                                    this->weaponModifiers[4] = 99;
                                } else
                                { // Sharpened Bullets
                                    this->weaponModifiers[4] = 75;
                                }
                            } else
                            { // Secondary Wind, Emergent Aftermath and Kill Switch         |           conclave exclusives
                                this->pruneThis = true;
                            }
                        }
                        else
                        {
                            this->pruneThis = true;
                        }
                    }
                }
                // parse '+' mods
                else if (this->description[0] == '+')
                {
                    // always start with a number following the '+', then a '%' following the number (except Power Throw, so parse that first)
                    // if there is a second stat it is on the second line, after an \r\n

                    // Parse power throw as one of two mods that don't have a '%' after the first number, then handle the ones that have %
                    if (this->description.find("+2 ") == 0)
                    { /// Power Throw: "+2 Punch Through\r\nOn Consecutive throw (Max stacks 3):\r\n+100% Throw Damage"
                        this->weaponModifiers[5] = 300;
                        this->modifiesWeapon = true;
                    } 
                    
                    // Parse Drifting Contact which also doesn't have a % after the first number
                    else if (this->description.find("+10s ") == 0)
                    { 
                        this->weaponModifiers[7] = 40;
                        this->modifiesWeapon = true;
                    }

                    // parse the other mods
                    else
                    {
                        if (this->description[1] == '1')
                        {
                            if (this->description[2] == '0')
                            {   // mods with an initial value of "+100%"
                                this->modifiesWeapon = true;
                                if (this->description[6] == 'M')
                                {   // Spoiled Strike
                                    this->weaponModifiers[5] = 100;
                                    this->weaponModifiers[0] = -20;
                                }
                                else if (this->description[6] == 'R')
                                {   // Primed Tactical Pump
                                    this->weaponModifiers[9] = 100;
                                }
                                else if (this->description[13] == 'D')
                                {   // Continuous Misery
                                    this->weaponModifiers[6] = 100;
                                }
                                else if (this->description[6] == 'C')
                                {   // Enduring Affliction      -       Only on lifted enemies, so skip this
                                    this->pruneThis = true;
                                }
                            }
                            else if (this->description[2] == '1')
                            {   // mods with first value of "+110%"
                                this->modifiesWeapon = true;
                                if (this->description[6] == 'M')
                                {
                                    if (this->description[7] == 'a')
                                    {   // Primed Ammo Stock
                                        this->weaponModifiers[2] = 110;
                                    }
                                    else if (this->description[7] == 'e')
                                    {   // Sacrificial Pressure     -       For now base values not counting set        -       TODO: work with set mods to scale strength up
                                        this->weaponModifiers[5] = 110;
                                    }
                                    else if (this->description[7] == 'u')
                                    {
                                        if (this->className == "Amalgam")
                                        {   // Amalgam Barrel Diffusion     -       Always worse than normal Barrel Diffusion
                                            this->pruneThis = true;
                                        }
                                        else if (this->className == "Galvanized")
                                        {   // Galvanized Diffusion or Galvanized Hell 
                                            this->weaponModifiers[1] = 230;
                                        }
                                    }
                                }
                                else if (this->description[6] == 'C')
                                {
                                    if (this->description[15] == 'C')
                                    {   // Galvanized Steel
                                        this->weaponModifiers[3] = 110;
                                        this->weaponModifiers[4] = 120;
                                    }
                                    else if (this->description[15] == 'D')
                                    {   // Primed Ravage or Primed Target Cracker
                                        this->weaponModifiers[4] = 110;
                                    }
                                }
                                else if (this->description[6] == 'S')
                                {   //  Lasting Sting
                                    this->weaponModifiers[6] = 110;
                                }
                            }
                            else if (this->description[2] == '2')
                            {   // Mods with starting value of "+120%"
                                if (this->description[10] == 'I')
                                {   // impact mod
                                    this->statusTypeModifiers[0] = 120;
                                    this->modifiesElements = true;
                                }
                                else if (this->description[10] == 'P')
                                {   // puncture mod
                                    this->statusTypeModifiers[1] = 120;
                                    this->modifiesElements = true;
                                }
                                else if (this->description[10] == 'S')
                                {   // slash mod
                                    this->statusTypeModifiers[2] = 120;
                                    this->modifiesElements = true;
                                }
                                else if (this->description[15] == 'C')
                                {   // Critical Chance
                                    this->weaponModifiers[3] = 120;
                                    this->modifiesWeapon = true;
                                }
                                else if (this->description[15] == 'D')
                                {   // Critical Damage
                                    this->weaponModifiers[4] = 120;
                                    this->modifiesWeapon = true;
                                }
                                else if (this->description[7] == 'u')
                                {   // Multishot
                                    this->weaponModifiers[1] = 120;
                                    this->modifiesWeapon = true;
                                }
                                else if (this->description[7] == 'e')
                                {   // Either Pressure Point, which PPP always beats, or Killing Blow which is only heavy attacks, so pruned for now        -       TODO: add killing blow and stuff
                                    this->pruneThis = true;
                                }
                            }
                            else if (this->description[2] == '5')
                            {
                                this->modifiesWeapon = true;
                                if (this->description[3] == '5')
                                {   // Amalgam Serration, never better than Serration so drop it
                                    this->pruneThis = true;
                                }
                                else if (this->description[3] == '%')
                                {   // Seeking Fury
                                    this->weaponModifiers[9] = 15;
                                }
                                else if (this->type == "Rifle")
                                {   // Point Strike
                                    this->weaponModifiers[3] = 150;
                                }
                                else if (this->type == "Melee")
                                {   //  Maiming Strike      -       Only for slide attacks, so skip it
                                    this->pruneThis = true;
                                }
                            }
                            else if (this->description[2] == '6')
                            {
                                if (this->description[6] == '<')
                                {   //  Primed Elemental Mod
                                    this->modifiesElements = true;
                                    if (this->description[10] == 'E')
                                    {   //  Primed Convulsion
                                        this->statusTypeModifiers[5] = 165;
                                        this->isElementalMod = true;
                                    }
                                    else if (this->description[10] == 'I')
                                    {   //  Primed Heavy Trauma
                                        this->statusTypeModifiers[0] = 165;
                                    }
                                    else if (this->description[10] == 'P')
                                    {   //  Primed Fever Strike
                                        this->statusTypeModifiers[6] = 165;
                                        this->isElementalMod = true;
                                    }
                                    else if (this->description[11] == 'I')
                                    {   //  Primed Heated Charge
                                        this->statusTypeModifiers[3] = 165;
                                        this->isElementalMod = true;
                                    }
                                    else if (this->description[11] == 'R')
                                    {   //  Primed Cryo Rounds  or  Primed Chilling Grasp
                                        this->statusTypeModifiers[4] = 165;
                                        this->isElementalMod = true;
                                    }
                                }
                                else
                                {   //  Magnum Force, Primed Pressure Point, Serration, Primed Point Blank, or Heavy Caliber
                                    this->weaponModifiers[5] = 165;
                                    this->modifiesWeapon = true;
                                }
                            }
                            else if (this->description[2] == '8')
                            {   // Primed Pistol Gambit
                                this->weaponModifiers[3] = 187;
                                this->modifiesWeapon = true;
                            }
                        }
                        else if (this->description[1] == '2')
                        {
                            this->modifiesWeapon = true;
                            if (this->description[2] == '2')
                            {
                                if (this->name == "Hornet Strike")
                                {   // Hornet Strike
                                    this->weaponModifiers[5] = 220;
                                }
                                else
                                {   // Sacrificial Steel
                                    this->weaponModifiers[3] = 220;
                                }
                            }
                            else if (this->description[3] == '%')
                            {   // Mods that start with "+20%"
                                if (this->description[5] == 'M')
                                {   //  Wildfire
                                    this->weaponModifiers[2] = 20;
                                    this->statusTypeModifiers[3] = 60;
                                    this->isElementalMod = true;
                                    this->modifiesElements = true;
                                }
                                else
                                {   // Martial Fury     |       Lie in Wait     -       Conclave Exclusive
                                    this->pruneThis = true;
                                }
                            }
                            else
                            {   //  Critical Delay, Critical Deceleration, Creeping Bullseye
                                this->weaponModifiers[3] = 200;
                                this->weaponModifiers[0] = -20;
                            }
                        }
                        else if (this->description[1] == '3')
                        {
                            this->modifiesWeapon = true;
                            if (this->description[3] != '%')
                            {   // Spectral Serration, or Acuity mod, too specific to use for now   TODO: let user decide this
                                this->pruneThis = true;
                            }
                            else if (this->description[5] == 'A')
                            {   //  Gladiator Vice or Fury
                                this->weaponModifiers[0] = 30;
                            }
                            else if (this->description[5] == 'M')
                            {   // Slip Magazine, Magazine Warp, 
                                this->weaponModifiers[2] = 30;
                                if (this->description.length() > 23)
                                {   //  Full Capacity, Loaded Capacity, or Maximum Capacity     |       conclave exclusive
                                    this->pruneThis = true;
                                }
                            }
                            else
                            {   //  Fast hands, loose chamber, loose hatch, or Impenetrable offense           |       primed fast hands always better than normal, and others are conclave exclusive
                                this->pruneThis = true;
                            }
                        }
                        else if (this->description[1] == '4')
                        {
                            this->modifiesWeapon = true;
                            if (this->description[2] == '0')
                            {
                                if (this->description[5] == 'A')
                                {   //  Quickening
                                    this->weaponModifiers[0] = 40;
                                }
                                else if (this->description[5] == 'H')
                                {   //  Focus Energy
                                    this->statusTypeModifiers[5] = 60;
                                    this->isElementalMod = true;
                                    this->modifiesElements = true;
                                    this->modifiesWeapon = false;
                                }
                                else if (this->description[5] == 'M')
                                {   //  Ice Storm
                                    this->weaponModifiers[2] = 40;
                                    this->statusTypeModifiers[4] = 40;
                                    this->isElementalMod = true;
                                    this->modifiesElements = true;
                                }
                                else if (this->description[5] == 'R')
                                {   //  Stunning Speed
                                    this->weaponModifiers[9] = 40;
                                    this->weaponModifiers[7] = 30;
                                }
                                else
                                {   //  Weeping Wounds  |   Blood Rush      TODO: implement combo count and stop pruning these
                                    this->pruneThis = true;
                                }
                            }
                            else if (this->description[2] == '5')
                            {
                                if (this->type == "Primary")
                                {   //  Vigilante Fervor
                                    this->weaponModifiers[0] = 45;
                                }
                                else if (this->type == "Pistol")
                                {   //  Merciless Gunfight
                                    this->weaponModifiers[4] = 45;
                                }
                            }
                            else if (this->description[2] == '8')
                            {   //  Quickdraw
                                this->weaponModifiers[9] = 48;
                            }
                        }
                        else if (this->description[1] == '5')
                        {
                            this->modifiesWeapon = true;
                            if (this->description[2] == '0')
                            {
                                if (this->description[5] == 'R')
                                {   // Loose Magazine
                                    this->weaponModifiers[9] = 50;
                                }
                                else
                                {   //  Soaring Strike      |       Galvanized Reflex   TODO: add combo into account for melee, stop pruning galvanized reflex
                                    this->pruneThis = true;
                                }
                            }
                            else
                            {
                                if (this->description[5] == 'R')
                                {   //  Primed Fast Hands
                                    this->weaponModifiers[9] = 55;
                                }
                                else if (this->description[5] == 'M')
                                {   //  Primed Magazine Warp,   Primed Slip Magazine
                                    this->weaponModifiers[2] = 55;
                                }
                                else
                                {   //  Primed Shred, Primed Fury
                                    this->weaponModifiers[0] = 55;
                                }
                            }
                        }
                        else if (this->description[1] == '6')
                        {
                            if (this->description[5] == '<')
                            {   //  Elemental 60/60 mods
                                this->modifiesElements = true;
                                if (this->description[9] == 'E')
                                {   //  Electric 60/60 mods
                                    this->statusTypeModifiers[5] = 60;
                                    this->weaponModifiers[7] = 60;
                                    this->isElementalMod = true;
                                    this->modifiesWeapon = true;
                                }
                                else if (this->description[9] == 'M')
                                {   //  Magnetic 60/60 mods
                                    this->statusTypeModifiers[10] = 60;
                                    this->isElementalMod = true;
                                    this->modifiesWeapon = true;
                                    if (this->type == "Rifle")
                                    {   //  Magnetic Capacity
                                        this->weaponModifiers[2] = 40;
                                    }
                                    else if (this->type == "Pistol")
                                    {   //  Magnetic Might
                                        this->weaponModifiers[4] = 40;
                                    }
                                    else if (this->type == "Melee")
                                    {   //  Magnetic Rush
                                        this->weaponModifiers[0] = 20;
                                    }
                                    else if(this->type == "Shotgun")
                                    {   //  Magnetic Strafe
                                        this->weaponModifiers[0] = 40;
                                    }
                                }
                                else if (this->description[9] == 'P')
                                {   //  Toxin 60/60 mods
                                    this->statusTypeModifiers[6] = 60;
                                    this->weaponModifiers[7] = 60;
                                    this->isElementalMod = true;
                                    this->modifiesWeapon = true;
                                }
                                else if (this->description[9] == 'R')
                                {   //  Radiation 60/60 mods
                                    this->statusTypeModifiers[11] = 60;
                                    this->isElementalMod = true;
                                    this->modifiesWeapon = true;
                                    if (this->type == "Rifle")
                                    {   //  Radiated Reload
                                        this->weaponModifiers[9] = 40;
                                    }
                                    else if (this->type == "Pistol")
                                    {   //  Accelerated Isotope
                                        this->weaponModifiers[0] = 40;
                                    }
                                    else if(this->type == "Shotgun")
                                    {   //  Atomic Fallout
                                        this->weaponModifiers[2] = 40;
                                    }
                                    //  Focus Radon has heavy attack efficiency which isn't tracked
                                }
                                else if (this->description[9] == 'S')
                                {   //  Rending Strike 60/60 mods
                                    this->statusTypeModifiers[2] = 60;
                                    this->statusTypeModifiers[1] = 80;
                                }
                                else if (this->description[10] == 'I')
                                {   //  Heat 60/60 mods
                                    this->statusTypeModifiers[3] = 60;
                                    this->weaponModifiers[7] = 60;
                                    this->isElementalMod = true;
                                    this->modifiesWeapon = true;
                                }
                                else if (this->description[10] == 'R')
                                {   // Cold 60/60 mods
                                    this->statusTypeModifiers[4] = 60;
                                    this->isElementalMod = true;
                                    this->modifiesWeapon = true;
                                    if (this->name == "Chilling Reload")
                                    {   //  Chilling Reload
                                        this->weaponModifiers[9] = 40;
                                    }
                                    else
                                    {   //  Vicious Frost, Rime Rounds, Frigid Blast, Frostbite
                                        this->weaponModifiers[7] = 60;
                                    }
                                }
                            }
                            else if (this->description[5] == 'C')
                            {   //  Critical Damage mods    |   Ravage, Target Cracker, Gladiator Might
                                this->weaponModifiers[4] = 60;
                                this->modifiesWeapon = true;
                                if (this->type == "Rifle")
                                {   //  Hammer Shot
                                    this->weaponModifiers[7] = 80;
                                }
                                else if (this->name == "Hollow Point")
                                {   //  Hollow Point
                                    this->weaponModifiers[5] = -15;
                                }
                            }
                            else if (this->description[5] == 'D')
                            {   //  Blaze
                                this->weaponModifiers[5] = 60;
                                this->statusTypeModifiers[3] = 60;
                                this->isElementalMod = true;
                                this->modifiesElements = true;
                                this->modifiesWeapon = true;
                            }
                            else if (this->description[5] == 'F')
                            {   //  Fire rate mods
                                this->weaponModifiers[0] = 60;
                                this->modifiesWeapon = true;
                                if (this->type == "Pistol")
                                {   //  Lethal Torrent
                                    this->weaponModifiers[1] = 60;
                                }
                                else if (this->type == "Shotgun")
                                {   //  Accelerated Blast
                                    this->statusTypeModifiers[1] = 60;
                                    this->modifiesElements = true;
                                }
                            }
                            else if (this->description[6] == 'a')
                            {   //  Magazine Capacity mods
                                this->modifiesWeapon = true;
                                if (this->description[2] == '6')
                                {   //  Tainted Mag
                                    this->weaponModifiers[2] = 66;
                                    this->weaponModifiers[9] = -33;
                                }
                                else if (this->type == "Shotgun")
                                {   //  Ammo Stock
                                    this->weaponModifiers[2] = 60;
                                    if (this->description.length() > 25)
                                    {   //  Burdened Magazine
                                        this->weaponModifiers[9] = -18;
                                    }
                                }
                                else
                                {   //  Tainted Clip
                                    this->weaponModifiers[2] = 60;
                                    this->weaponModifiers[9] = -30;
                                }
                            }
                            else if (this->description[6] == 'u')
                            {   //  Vigilante Armaments
                                this->weaponModifiers[1] = 60;
                                this->modifiesWeapon = true;
                            }
                            else if (this->description[5] == 'R')
                            {   //  Tactical Pump
                                this->weaponModifiers[9] = 60;
                                this->modifiesWeapon = true;
                            }
                            
                        }
                        else if (this->description[1] == '7')
                        {
                            this->modifiesWeapon = true;
                            if (this->description[2] == '2')
                            {   // Gunslinger
                                this->weaponModifiers[0] = 72;
                            }
                            else if (this->description[2] == '7')
                            {   // Tainted shell        -       No benefit that is tracked, drop it
                                this->pruneThis = true;
                            }
                        }
                        else if (this->description[1] == '8')
                        {
                            this->modifiesWeapon = true;
                            if (this->description[2] == '0')
                            {
                                if (this->description[6] == 'u')
                                {   //  Galvanized Chamber
                                    this->weaponModifiers[1] = 230;
                                }
                                else if (this->description[6] == 'e')
                                {   //  Condition Overload
                                    this->weaponModifiers[10] = 80;
                                }
                                else if (this->type == "Melee")
                                {   //  Galvanized Elementalist
                                    this->weaponModifiers[8] = 80;
                                    this->weaponModifiers[7] = 120;
                                }
                                else if (this->type == "Pistol")
                                {   //  Galvanized Shot
                                    this->weaponModifiers[7] = 80;
                                    this->weaponModifiers[10] = 120;
                                }
                                else
                                {   //  Galvanized Aptitude and Galvanized Savvy
                                    this->weaponModifiers[7] = 80;
                                    this->weaponModifiers[10] = 80;
                                }
                            }
                            else if (this->description[2] == '5')
                            {   //  Amalgam Shotgun Barrage and Amalgam Organ Shatter   -   always worse than their other counterparts (for DPS like this)
                                this->pruneThis = true;
                            }
                            else if (this->description[2] == '8')
                            {   //  Primed Quickdraw
                                this->weaponModifiers[9] = 88;
                            }                            
                        }
                        else if (this->description[1] == '9')
                        {
                            if (this->description[5] == '<')
                            {   //  Elemental mods
                                this->modifiesElements = true;
                                if (this->description[9] == 'E')
                                {   //  Electric
                                    this->statusTypeModifiers[5] = 90;
                                    this->isElementalMod = true;
                                }
                                else if (this->description[10] == 'I')
                                {   //  Heat
                                    this->statusTypeModifiers[3] = 90;
                                    this->isElementalMod = true;
                                }
                                else if (this->description[10] == 'R')
                                {   //  Cold
                                    this->statusTypeModifiers[4] = 90;
                                    this->isElementalMod = true;
                                }
                                else if (this->description[10] == 'O')
                                {   //  Toxin
                                    this->statusTypeModifiers[6] = 90;
                                    this->isElementalMod = true;
                                }
                                else
                                {
                                    if (this->description[9] == 'I')
                                    {   //  Impact
                                        this->statusTypeModifiers[0] = 90;
                                    }
                                    else if (this->description[9] == 'P')
                                    {   //  Puncture
                                        this->statusTypeModifiers[1] = 90;
                                    }
                                    else if (this->description[9] == 'S')
                                    {   //  Slash
                                        this->statusTypeModifiers[2] = 90;
                                    }
                                    if (this->setName != "")
                                    {   //  Saxum, Jugulus, or Carnis set
                                        this->modifiesWeapon = true;
                                        this->weaponModifiers[7] = 60;
                                    }
                                }
                            }
                            else if (this->description[5] == 'D')
                            {   //  Point Blank, Augur Pact, Vicious Spread
                                this->weaponModifiers[5] = 90;
                                this->modifiesWeapon = true;
                            }
                            else if (this->description[5] == 'F')
                            {   //  Shotgun Barrage
                                this->weaponModifiers[0] = 90;
                                this->modifiesWeapon = true;
                                if (this->name != "Shotgun Barrage")
                                {   //  Frail Momentum, Vile Acceleration, Anemic Agility
                                    this->weaponModifiers[5] = 15;
                                }
                            }
                            else if (this->description[6] == 'u')
                            {   //  Split Chamber
                                this->weaponModifiers[1] = 90;
                                this->modifiesWeapon = true;
                            }
                            else if (this->description[5] == 'S')
                            {   //  Status Chance, Status Damage, and Status Duration mods
                                this->modifiesWeapon = true;
                                if (this->description[12] == 'C')
                                {   //  Rifle Aptitude, Shotgun Savvy, Sure Shot, Melee Prowess
                                    this->weaponModifiers[7] = 90;
                                }
                                else if (this->description[13] == 'a')
                                {   //  Melee Elementalist, Rifle Elementalist      -       Second stat is not a tracked one
                                    this->weaponModifiers[8] = 90;
                                    if (this->type == "Pistol")
                                    {   //  Pistol Elementalist
                                        this->weaponModifiers[9] = 60;
                                    }
                                    else if (this->type == "Shotgun")
                                    {   //  Shotgun Elementalist
                                        this->weaponModifiers[2] = 60;
                                    }
                                }
                                else if (this->description[13] == 'u')
                                {   //  Augur Seeker, Perpetual Agony, Lingering torment, Hunter Track
                                    this->weaponModifiers[6] = 90;
                                }
                            }
                            else if (this->description[5] == 'C')
                            {   //  Crit mods
                                this->modifiesWeapon = true;
                                if (this->description[14] == 'C')
                                {   //  Blunderbuss
                                    this->weaponModifiers[3] = 90;
                                }
                                else if (this->description[14] == 'D')
                                {
                                    this->weaponModifiers[4] = 90;
                                }
                            }
                            else if (this->description[5] == 'M')
                            {
                                this->modifiesWeapon = true;
                                if (this->type == "Dual Daggers")
                                {   //  Amar's Contempt
                                    this->weaponModifiers[5] = 90;
                                    this->statusTypeModifiers[2] = 30;
                                    this->modifiesElements = true;
                                }
                                else if (this->type == "Polearms")
                                {   //  Boreal's Contempt
                                    this->weaponModifiers[5] = 90;
                                    this->weaponModifiers[8] = 60;
                                }
                            }
                        }
                    }
                }
                // parse '-' mods
                else if (this->description[0] == '-')
                {
                    /// Depleted Reload: "-60% Magazine Capacity\r\n+48% Reload Speed"
                    if (this->description == "-60% Magazine Capacity\r\n+48% Reload Speed")
                    {
                        this->weaponModifiers[2] = -60;
                        this->weaponModifiers[9] = 48;
                        this->modifiesWeapon = true;
                    }
                    /// Hydraulic Gauge: "-60% Weapon Recoil\r\n-10% Magazine Capacity" | PRUNE THIS - no benefit that is tracked
                    /// Hydraulic Chamber: "-60% Weapon Recoil\r\n-10% Magazine Capacity" | PRUNE THIS - no benefit that is tracked
                    /// Hydraulic Barrel: "-40% Weapon Recoil\r\n-20% Magazine Capacity" | PRUNE THIS - no benefit that is tracked
                    /// Vile Precision: "-90% Weapon Recoil\r\n-36% Fire Rate (x2 for Bows)" | PRUNE THIS - no benefit that is tracked
                    else
                    {
                        this->pruneThis = true;
                    }
                }
            }
            if (this->pruneThis == false)
            {
                this->indexInValidMods = currentValidModsIndex;
                currentValidModsIndex++;
            }

            for (int weaponUpgradeIndex = 0; weaponUpgradeIndex < 11; weaponUpgradeIndex++)
            {
                if (this->weaponModifiers.at(weaponUpgradeIndex) != 0)
                {
                    this->weaponModifierMask |= (1 << weaponUpgradeIndex);
                    this->weaponModifiers[weaponUpgradeIndex] = this->weaponModifiers[weaponUpgradeIndex] * 0.01;
                }
            }
            for (int elementalUpgradeIndex = 0; elementalUpgradeIndex < 13; elementalUpgradeIndex++)
            {
                if (this->statusTypeModifiers.at(elementalUpgradeIndex) != 0)
                {
                    this->modifiedElementIndicesMask |= (1 << elementalUpgradeIndex);
                    this->statusTypeModifiers[elementalUpgradeIndex] = this->statusTypeModifiers[elementalUpgradeIndex] * 0.01;
                }
            }
        }
};





// set to be pruned bool (for mods that are useless for dps, or lower variants of other mods)       -       then have the main pass over all entries in validMods vector and drop any that have the pruneThis bool set to true
