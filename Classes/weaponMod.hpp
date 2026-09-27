#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

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
        std::string className = "";                         // class name                       (if it exists)

        bool locksFireRate = false;                         // whether the mod should lock the fire rate of the weapon otherwise

        bool pruneThis = false;                             // whether the mod should be pruned (ie: better version exists so don't count in damage calcs, or no affect on damage outcome (for example, ammo max mods))
        
        std::unordered_map<std::string, int> weaponModifiers = {        // the comment is the string to regex for when parsing a mod description
            {"Fire Rate", 0},           // "Fire Rate" or "Attack Speed"
            {"Multishot", 0},           // "Multishot"
            {"Magazine Capacity", 0},   // "Magazine Capacity"
            {"Critical Chance", 0},     // "Critical Chance"
            {"Critical Damage", 0},     // "Critical Damage"
            {"Damage", 0},              // "Damage" or "Melee Damage"
            {"Status Duration", 0},     // "Status Duration"
            {"Status Chance", 0},       // "Status Chance"
            {"Status Damage", 0},       // "Status Damage"
            {"Reload Speed", 0},        // "Reload Speed"
            {"GunCODamage", 0},         // "Direct Damage per Status Type affecting the target" or "Melee Damage per Status Type affecting the target"
            {"isFireRateLocked", 0}     // "Fire Rate cannot be modified"
        };

        std::unordered_map<std::string, int> statusTypeModifiers = {        // the comment is the string to regex for when parsing a mod description
            {"Impact", 0},              // "<DT_IMPACT_COLOR>Impact"
            {"Puncture", 0},            // "<DT_PUNCTURE_COLOR>Puncture"
            {"Slash", 0},               // "<DT_SLASH_COLOR>Slash"
            {"Heat", 0},                // "<DT_FIRE_COLOR>Heat"
            {"Cold", 0},                // "<DT_FREEZE_COLOR>Cold"
            {"Electric", 0},            // "<DT_ELECTRICITY_COLOR>Electricity"
            {"Toxin", 0},               // "<DT_POISON_COLOR>Toxin"
            {"Blast", 0},               // "<DT_EXPLOSION_COLOR>Blast"
            {"Corrosive", 0},           // "<DT_CORROSIVE_COLOR>Corrosive"
            {"Gas", 0},                 // "<DT_GAS_COLOR>Gas"
            {"Magnetic", 0},            // "<DT_MAGNETIC_COLOR>Magnetic"
            {"Radiation", 0},           // "<DT_RADIATION_COLOR>Radiation"
            {"Viral", 0}                // "<DT_VIRAL_COLOR>Viral"
        };

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


        // general getters
        double getPruneThis() const { return pruneThis; }
        std::unordered_map<std::string, int> getWeaponModifiers() const { return weaponModifiers; }
        std::unordered_map<std::string, int> getStatusTypeModifiers() const { return statusTypeModifiers; }

        weaponMod(std::string newName, std::string newType, std::string newDescription, std::string newSetName, std::string newClassName, std::vector<std::string> newIncompatibilityTags, std::vector<std::string> newIncompatibleMods, std::vector<std::string> newUpgradeTypes) // make this constructor create a mod based on passed in data. I need to decide how to pass in the data from the json though
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
                // TODO: list all useful tags in upgradeTypes field for a mod, so I can set a mod to be pruned if it doesn't have at least one of them
                // Use if (a OR b OR c OR d)
                // do nothing
                // else set to prune
                // that way the or checks will break and go right away, slightly optimized performance
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
                    if (this->description.find("Only") == 0) // finds if the first match is at index 0
                    {   // because no augment mods the only ones starting with this are cannonade mods (no need to worry about Efficient Beams)
                        if (this->type == "Pistol")
                        {   // pistol has higher stats than others, so seperate that
                            this->weaponModifiers["Damage"] = 300;
                        } else
                        {
                            this->weaponModifiers["Damage"] = 240;
                        }
                        // Find a way to lock fire rate
                        this->locksFireRate = true;
                    } else if (this->description.find("On ") == 0) // finds if the first match is at 0
                    {
                        /// List of mods to parse
                        /// Berserker Fury: "On Melee Kill:\r\n+35% Attack Speed for 10s. Stacks up to 2x."
                        if (this->description.find("On M") == 0) // Either Combo Fury or Berserker Fury
                        {
                            if (this->type != "Thrown Melee")
                            {   // Berserker Fury
                                this->weaponModifiers["Fire Rate"] = 70;
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
                                this->weaponModifiers["Status Chance"] = 60;
                            } else
                            {   // Nano-Applicator or Embedded Catalyzer
                                this->weaponModifiers["Status Chance"] = 90;
                            }
                        }
                        /// Hydraulic Crosshairs: "On Headshot:\r\n+135% Critical Chance when Aiming for 9s"
                        /// Argon Scope: "On Headshot:\r\n+135% Critical Chance when Aiming for 9s"
                        /// Laser Sight: "On Headshot:\r\n+120% Critical Chance when Aiming for 9s"
                        /// Galvanized Scope: "On Headshot:\r\n+120% Critical Chance when Aiming for 12s\r\nOn Headshot Kill:\r\n+40% Critical Chance when Aiming for 12s. Stacks up to 5x."
                        /// Galvanized Crosshairs: "On Headshot:\r\n+120% Critical Chance when Aiming for 12s\r\nOn Headshot Kill:\r\n+40% Critical Chance when Aiming for 12s. Stacks up to 5x."
                        else if (this->description.find("On Head") == 0)
                        {
                            if (this->className == "Galvanized")
                            {   // Galvanized Scope and/or Crosshairs
                                this->weaponModifiers["Critical Chance"] = 320;
                            }
                            else if (this->type == "Shotgun")
                            {   // Laser Sight
                                this->weaponModifiers["Critical Chance"] = 120;
                            } else
                            {   // Hydraulic Crosshairs or Argon Scope
                                this->weaponModifiers["Critical Chance"] = 135;
                            }
                        }
                        /// Split Flights: "On Hit:\r\n+100% Multishot\r\n-180% Accuracy for 2s. Stacks up to 4x.\r\n(Non-AOE Bows)"
                        else if (this->description.find("On Hit") == 0)
                        {   // Narrow Barrel, Targeting Subsystem, Guided Ordnance, Night Stalker, Bounty Hunter, and Apex Predator get removed above | Double Tap and Velox Conclusion are augments so they don't get here either
                            if (this->name == "Split Flights")
                            {   // Split Flights
                                this->weaponModifiers["Multishot"] = 400;
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
                                this->weaponModifiers["Fire Rate"] = 75;
                            } else if (this->type == "Shotgun")
                            {   // Repeater Clip
                                this->weaponModifiers["Fire Rate"] = 105;
                            } else
                            {   // Pressurized Magazine
                                this->weaponModifiers["Fire Rate"] = 90;
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
                                    this->weaponModifiers["Critical Damage"] = 120;
                                } else if (this->type == "Shotgun")
                                { // Shrapnel Shot
                                    this->weaponModifiers["Critical Damage"] = 99;
                                } else
                                { // Sharpened Bullets
                                    this->weaponModifiers["Critical Damage"] = 75;
                                }
                            } else
                            { // Secondary Wind, Emergent Aftermath and Kill Switch
                                this->weaponModifiers["Reload Speed"] = 50;
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
                        this->weaponModifiers["Damage"] = 300;
                    } 
                    
                    // Parse Drifting Contact which also doesn't have a % after the first number
                    else if (this->description.find("+10s ") == 0)
                    { 
                        this->weaponModifiers["Status Chance"] = 40;
                    }

                    // parse the other mods
                    else
                    {
                        if (this->description[1] == '1')
                        {
                            if (this->description[2] == '0')
                            {   // mods with an initial value of "+100%"
                                if (this->description[6] == 'M')
                                {   // Spoiled Strike
                                    this->weaponModifiers["Damage"] = 100;
                                    this->weaponModifiers["Fire Rate"] = -20;
                                }
                                else if (this->description[6] == 'R')
                                {   // Primed Tactical Pump
                                    this->weaponModifiers["Reload Speed"] = 100;
                                }
                                else if (this->description[13] == 'D')
                                {   // Continuous Misery
                                    this->weaponModifiers["Status Duration"] = 100;
                                }
                                else if (this->description[6] == 'C')
                                {   // Enduring Affliction      -       Only on lifted enemies, so skip this
                                    this->pruneThis = true;
                                }
                            }
                            else if (this->description[2] == '1')
                            {   // mods with first value of "+110%"
                                if (this->description[6] == 'M')
                                {
                                    if (this->description[7] == 'a')
                                    {   // Primed Ammo Stock
                                        this->weaponModifiers["Magazine Capacity"] = 110;
                                    }
                                    else if (this->description[7] == 'e')
                                    {   // Sacrificial Pressure     -       For now base values not counting set        -       TODO: work with set mods to scale strength up
                                        this->weaponModifiers["Damage"] = 110;
                                    }
                                    else if (this->description[7] == 'u')
                                    {
                                        if (this->className == "Amalgam")
                                        {   // Amalgam Barrel Diffusion     -       Always worse than normal Barrel Diffusion
                                            this->pruneThis = true;
                                        }
                                        else if (this->className == "Galvanized")
                                        {   // Galvanized Diffusion or Galvanized Hell 
                                            this->weaponModifiers["Multishot"] = 230;
                                        }
                                    }
                                }
                                else if (this->description[6] == 'C')
                                {
                                    if (this->description[15] == 'C')
                                    {   // Galvanized Steel
                                        this->weaponModifiers["Critical Chance"] = 110;
                                        this->weaponModifiers["Critical Damage"] = 120;
                                    }
                                    else if (this->description[15] == 'D')
                                    {   // Primed Ravage or Primed Target Cracker
                                        this->weaponModifiers["Critical Damage"] = 110;
                                    }
                                }
                                else if (this->description[6] == 'S')
                                {   //  Lasting Sting
                                    this->weaponModifiers["Status Duration"] = 110;
                                }
                            }
                            else if (this->description[2] == '2')
                            {   // Mods with starting value of "+120%"
                                if (this->description[10] == 'I')
                                {   // impact mod
                                    this->statusTypeModifiers["Impact"] = 120;
                                }
                                else if (this->description[10] == 'P')
                                {   // puncture mod
                                    this->statusTypeModifiers["Puncture"] = 120;
                                }
                                else if (this->description[10] == 'S')
                                {   // slash mod
                                    this->statusTypeModifiers["Slash"] = 120;
                                }
                                else if (this->description[15] == 'C')
                                {   // Critical Chance
                                    this->weaponModifiers["Critical Chance"] = 120;
                                }
                                else if (this->description[15] == 'D')
                                {   // Critical Damage
                                    this->weaponModifiers["Critical Damage"] = 120;
                                }
                                else if (this->description[7] == 'u')
                                {   // Multishot
                                    this->weaponModifiers["Multishot"] = 120;
                                }
                                else if (this->description[7] == 'e')
                                {   // Either Pressure Point, which PPP always beats, or Killing Blow which is only heavy attacks, so pruned for now        -       TODO: add killing blow and stuff
                                    this->pruneThis = true;
                                }
                            }
                            else if (this->description[2] == '5')
                            {
                                if (this->description[3] == '5')
                                {   // Amalgam Serration, never better than Serration so drop it
                                    this->pruneThis = true;
                                }
                                else if (this->description[3] == '%')
                                {   // Seeking Fury
                                    this->weaponModifiers["Reload Speed"] = 15;
                                }
                                else if (this->type == "Rifle")
                                {   // Point Strike
                                    this->weaponModifiers["Critical Chance"] = 150;
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
                                    if (this->description[10] == 'E')
                                    {   //  Primed Convulsion
                                        this->statusTypeModifiers["Electric"] = 165;
                                    }
                                    else if (this->description[10] == 'I')
                                    {   //  Primed Heavy Trauma
                                        this->statusTypeModifiers["Impact"] = 165;
                                    }
                                    else if (this->description[10] == 'P')
                                    {   //  Primed Fever Strike
                                        this->statusTypeModifiers["Toxin"] = 165;
                                    }
                                    else if (this->description[11] == 'I')
                                    {   //  Primed Heated Charge
                                        this->statusTypeModifiers["Heat"] = 165;
                                    }
                                    else if (this->description[11] == 'R')
                                    {   //  Primed Cryo Rounds  or  Primed Chilling Grasp
                                        this->statusTypeModifiers["Cold"] = 165;
                                    }
                                }
                                else
                                {   //  Magnum Force, Primed Pressure Point, Serration, Primed Point Blank, or Heavy Caliber
                                    this->weaponModifiers["Damage"] = 165;
                                }
                            }
                            else if (this->description[2] == '8')
                            {   // Primed Pistol Gambit
                                this->weaponModifiers["Critical Chance"] = 187;
                            }
                        }
                        else if (this->description[1] == '2')
                        {
                            if (this->description[2] == '2')
                            {
                                if (this->name == "Hornet Strike")
                                {   // Hornet Strike
                                    this->weaponModifiers["Damage"] = 220;
                                }
                                else
                                {   // Sacrificial Steel
                                    this->weaponModifiers["Critical Chance"] = 220;
                                }
                            }
                            else if (this->description[3] == '%')
                            {   // Mods that start with "+20%"
                                if (this->description[5] == 'M')
                                {   //  Wildfire
                                    this->weaponModifiers["Magazine Capacity"] = 20;
                                    this->statusTypeModifiers["Heat"] = 60;
                                }
                                else
                                {   // Martial Fury     |       Lie in Wait     -       Conclave Exclusive
                                    this->pruneThis = true;
                                }
                            }
                            else
                            {   //  Critical Delay, Critical Deceleration, Creeping Bullseye
                                this->weaponModifiers["Critical Chance"] = 200;
                                this->weaponModifiers["Fire Rate"] = -20;
                            }
                        }
                        else if (this->description[1] == '3')
                        {
                            if (this->description[3] != '%')
                            {   // Spectral Serration, or Acuity mod, too specific to use for now   TODO: let user decide this
                                this->pruneThis;
                            }
                            else if (this->description[5] == 'A')
                            {   //  Gladiator Vice or Fury
                                this->weaponModifiers["Fire Rate"] = 30;
                            }
                            else if (this->description[5] == 'M')
                            {   // Slip Magazine, Magazine Warp, 
                                this->weaponModifiers["Magazine Capacity"] = 30;
                                if (this->description.length() > 23)
                                {   //  Full Capacity, Loaded Capacity, or Maximum Capacity
                                    this->weaponModifiers["Reload Speed"] = -15;
                                }
                            }
                            else if (this->description[5] == 'R')
                            {   //  Fast hands, loose chamber, or loose hatch
                                this->weaponModifiers["Reload Speed"] = 30;
                            }
                            else
                            {   //  Impenetrable Offense
                                this->pruneThis;
                            }
                        }
                        else if (this->description[1] == '4')
                        {
                            if (this->description[2] == '0')
                            {
                                if (this->description[5] == 'A')
                                {   //  Quickening
                                    this->weaponModifiers["Fire Rate"] = 40;
                                }
                                else if (this->description[5] == 'H')
                                {   //  Focus Energy
                                    this->statusTypeModifiers["Electric"] = 60;
                                }
                                else if (this->description[5] == 'M')
                                {   //  Ice Storm
                                    this->weaponModifiers["Magazine Capacity"] = 40;
                                    this->statusTypeModifiers["Cold"] = 40;
                                }
                                else if (this->description[5] == 'R')
                                {   //  Stunning Speed
                                    this->weaponModifiers["Reload Speed"] = 40;
                                    this->weaponModifiers["Status Chance"] = 30;
                                }
                                else
                                {   //  Weeping Wounds  |   Blood Rush      TODO: implement combo count and stop pruning these
                                    this->pruneThis;
                                }
                            }
                            else if (this->description[2] == '5')
                            {
                                if (this->type == "Primary")
                                {   //  Vigilante Fervor
                                    this->weaponModifiers["Fire Rate"] = 45;
                                }
                                else if (this->type == "Pistol")
                                {   //  Merciless Gunfight
                                    this->weaponModifiers["Critical Damage"] = 45;
                                }
                            }
                            else if (this->description[2] == '8')
                            {   //  Quickdraw
                                this->weaponModifiers["Reload Speed"] = 48;
                            }
                        }
                        else if (this->description[1] == '5')
                        {
                            if (this->description[2] == '0')
                            {
                                if (this->description[5] == 'R')
                                {   // Loose Magazine
                                    this->weaponModifiers["Reload Speed"] = 50;
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
                                    this->weaponModifiers["Reload Speed"] = 55;
                                }
                                else if (this->description[5] == 'M')
                                {   //  Primed Magazine Warp,   Primed Slip Magazine
                                    this->weaponModifiers["Magazine Capacity"] = 55;
                                }
                                else
                                {   //  Primed Shred, Primed Fury
                                    this->weaponModifiers["Fire Rate"] = 55;
                                }
                            }
                        }
                        else if (this->description[1] == '6')
                        {
                            if (this->description[5] == '<')
                            {   //  Elemental 60/60 mods
                                if (this->description[9] == 'E')
                                {   //  Electric 60/60 mods
                                    this->statusTypeModifiers["Electric"] = 60;
                                    this->weaponModifiers["Status Chance"] = 60;
                                }
                                else if (this->description[9] == 'M')
                                {   //  Magnetic 60/60 mods
                                    this->statusTypeModifiers["Magnetic"] = 60;
                                    if (this->type == "Rifle")
                                    {   //  Magnetic Capacity
                                        this->weaponModifiers["Magazine Capacity"] = 40;
                                    }
                                    else if (this->type == "Pistol")
                                    {   //  Magnetic Might
                                        this->weaponModifiers["Critical Damage"] = 40;
                                    }
                                    else if (this->type == "Melee")
                                    {   //  Magnetic Rush
                                        this->weaponModifiers["Fire Rate"] = 20;
                                    }
                                    else if(this->type == "Shotgun")
                                    {   //  Magnetic Strafe
                                        this->weaponModifiers["Fire Rate"] = 40;
                                    }
                                }
                                else if (this->description[9] == 'P')
                                {   //  Toxin 60/60 mods
                                    this->statusTypeModifiers["Toxin"] = 60;
                                    this->weaponModifiers["Status Chance"] = 60;
                                }
                                else if (this->description[9] == 'R')
                                {   //  Radiation 60/60 mods
                                    this->statusTypeModifiers["Radiation"] = 60;
                                    if (this->type == "Rifle")
                                    {   //  Radiated Reload
                                        this->weaponModifiers["Reload Speed"] = 40;
                                    }
                                    else if (this->type == "Pistol")
                                    {   //  Accelerated Isotope
                                        this->weaponModifiers["Fire Rate"] = 40;
                                    }
                                    else if(this->type == "Shotgun")
                                    {   //  Atomic Fallout
                                        this->weaponModifiers["Magazine Capacity"] = 40;
                                    }
                                    //  Focus Radon has heavy attack efficiency which isn't tracked
                                }
                                else if (this->description[9] == 'S')
                                {   //  Rending Strike 60/60 mods
                                    this->statusTypeModifiers["Slash"] = 60;
                                    this->statusTypeModifiers["Puncture"] = 80;
                                }
                                else if (this->description[10] == 'I')
                                {   //  Heat 60/60 mods
                                    this->statusTypeModifiers["Heat"] = 60;
                                    this->weaponModifiers["Status Chance"] = 60;
                                }
                                else if (this->description[10] == 'R')
                                {   // Cold 60/60 mods
                                    this->statusTypeModifiers["Cold"] = 60;
                                    if (this->name == "Chilling Reload")
                                    {   //  Chilling Reload
                                        this->weaponModifiers["Reload Speed"] = 40;
                                    }
                                    else
                                    {   //  Vicious Frost, Rime Rounds, Frigid Blast, Frostbite
                                        this->weaponModifiers["Status Chance"] = 60;
                                    }
                                }
                            }
                            else if (this->description[5] == 'C')
                            {   //  Critical Damage mods    |   Ravage, Target Cracker, Gladiator Might
                                this->weaponModifiers["Critical Damage"] = 60;
                                if (this->type == "Rifle")
                                {   //  Hammer Shot
                                    this->weaponModifiers["Status Chance"] = 80;
                                }
                                else if (this->name == "Hollow Point")
                                {   //  Hollow Point
                                    this->weaponModifiers["Damage"] = -15;
                                }
                            }
                            else if (this->description[5] == 'D')
                            {   //  Blaze
                                this->weaponModifiers["Damage"] = 60;
                                this->statusTypeModifiers["Heat"] = 60;
                            }
                            else if (this->description[5] == 'F')
                            {   //  Fire rate mods
                                this->weaponModifiers["Fire Rate"] = 60;
                                if (this->type == "Pistol")
                                {   //  Lethal Torrent
                                    this->weaponModifiers["Multishot"] = 60;
                                }
                                else if (this->type == "Shotgun")
                                {   //  Accelerated Blast
                                    this->statusTypeModifiers["Puncture"] = 60;
                                }
                            }
                            else if (this->description[6] == 'a')
                            {   //  Magazine Capacity mods
                                if (this->description[2] == '6')
                                {   //  Tainted Mag
                                    this->weaponModifiers["Magazine Capcity"] = 66;
                                    this->weaponModifiers["Reload Speed"] = -33;
                                }
                                else if (this->type == "Shotgun")
                                {   //  Ammo Stock
                                    this->weaponModifiers["Magazine Capcity"] = 60;
                                    if (this->description.length() > 25)
                                    {   //  Burdened Magazine
                                        this->weaponModifiers["Reload Speed"] = -18;
                                    }
                                }
                                else
                                {   //  Tainted Clip
                                    this->weaponModifiers["Magazine Capcity"] = 60;
                                    this->weaponModifiers["Reload Speed"] = -30;
                                }
                            }
                            else if (this->description[6] == 'u')
                            {   //  Vigilante Armaments
                                this->weaponModifiers["Multishot"] = 60;
                            }
                            else if (this->description[5] == 'R')
                            {   //  Tactical Pump
                                this->weaponModifiers["Reload Speed"] = 60;
                            }
                            
                        }
                        else if (this->description[1] == '7')
                        {
                            if (this->description[2] == '2')
                            {   // Gunslinger
                                this->weaponModifiers["Fire Rate"] = 72;
                            }
                            else if (this->description[2] == '7')
                            {   // Tainted shell        -       No benefit that is tracked, drop it
                                this->pruneThis = true;
                            }
                        }
                        else if (this->description[1] == '8')
                        {
                            if (this->description[2] == '0')
                            {
                                if (this->description[6] == 'u')
                                {   //  Galvanized Chamber
                                    this->weaponModifiers["Multishot"] = 230;
                                }
                                else if (this->description[6] == 'e')
                                {   //  Condition Overload
                                    this->weaponModifiers["GunCODamage"] = 80;
                                }
                                else if (this->type == "Melee")
                                {   //  Galvanized Elementalist
                                    this->weaponModifiers["Status Damage"] = 80;
                                    this->weaponModifiers["Status Chance"] = 120;
                                }
                                else if (this->type == "Pistol")
                                {   //  Galvanized Shot
                                    this->weaponModifiers["Status Chance"] = 80;
                                    this->weaponModifiers["GunCODamage"] = 120;
                                }
                                else
                                {   //  Galvanized Aptitude and Galvanized Savvy
                                    this->weaponModifiers["Status Chance"] = 80;
                                    this->weaponModifiers["GunCODamage"] = 80;
                                }
                            }
                            else if (this->description[2] == '5')
                            {   //  Amalgam Shotgun Barrage and Amalgam Organ Shatter   -   always worse than their other counterparts (for DPS like this)
                                this->pruneThis = true;
                            }
                            else if (this->description[2] == '8')
                            {   //  Primed Quickdraw
                                this->weaponModifiers["Reload Speed"] = 88;
                            }                            
                        }
                        else if (this->description[1] == '9')
                        {
                            if (this->description[5] == '<')
                            {   //  Elemental mods
                                if (this->description[9] == 'E')
                                {   //  Electric
                                    this->statusTypeModifiers["Electric"] = 90;
                                }
                                else if (this->description[10] == 'I')
                                {   //  Heat
                                    this->statusTypeModifiers["Heat"] = 90;
                                }
                                else if (this->description[10] == 'R')
                                {   //  Cold
                                    this->statusTypeModifiers["Cold"] = 90;
                                }
                                else if (this->description[10] == 'O')
                                {   //  Toxin
                                    this->statusTypeModifiers["Toxin"] = 90;
                                }
                                else
                                {
                                    if (this->description[9] == 'I')
                                    {   //  Impact
                                        this->statusTypeModifiers["Impact"] = 90;
                                    }
                                    else if (this->description[9] == 'P')
                                    {   //  Puncture
                                        this->statusTypeModifiers["Puncture"] = 90;
                                    }
                                    else if (this->description[9] == 'S')
                                    {   //  Slash
                                        this->statusTypeModifiers["Slash"] = 90;
                                    }
                                    if (this->setName != "")
                                    {   //  Saxum, Jugulus, or Carnis set
                                        this->weaponModifiers["Status Chance"] = 60;
                                    }
                                }
                            }
                            else if (this->description[5] == 'D')
                            {   //  Point Blank, Augur Pact, Vicious Spread
                                this->weaponModifiers["Damage"] = 90;
                            }
                            else if (this->description[5] == 'F')
                            {   //  Shotgun Barrage
                                this->weaponModifiers["Fire Rate"] = 90;
                                if (this->name != "Shotgun Barrage")
                                {   //  Frail Momentum, Vile Acceleration, Anemic Agility
                                    this->weaponModifiers["Damage"] = 15;
                                }
                            }
                            else if (this->description[6] == 'u')
                            {   //  Split Chamber
                                this->weaponModifiers["Multishot"] = 90;
                            }
                            else if (this->description[5] == 'S')
                            {   //  Status Chance, Status Damage, and Status Duration mods
                                if (this->description[12] == 'C')
                                {   //  Rifle Aptitude, Shotgun Savvy, Sure Shot, Melee Prowess
                                    this->weaponModifiers["Status Chance"] = 90;
                                }
                                else if (this->description[13] == 'a')
                                {   //  Melee Elementalist, Rifle Elementalist      -       Second stat is not a tracked one
                                    this->weaponModifiers["Status Damage"] = 90;
                                    if (this->type == "Pistol")
                                    {   //  Pistol Elementalist
                                        this->weaponModifiers["Reload Speed"] = 60;
                                    }
                                    else if (this->type == "Shotgun")
                                    {   //  Shotgun Elementalist
                                        this->weaponModifiers["Magazine Capacity"] = 60;
                                    }
                                }
                                else if (this->description[13] == 'u')
                                {   //  Augur Seeker, Perpetual Agony, Lingering torment, Hunter Track
                                    this->weaponModifiers["Status Duration"] = 90;
                                }
                            }
                            else if (this->description[5] == 'C')
                            {   //  Crit mods
                                if (this->description[14] == 'C')
                                {   //  Blunderbuss
                                    this->weaponModifiers["Critical Chance"] = 90;
                                }
                                else if (this->description[14] == 'D')
                                {
                                    this->weaponModifiers["Critical Damage"] = 90;
                                }
                            }
                            else if (this->description[5] == 'M')
                            {
                                if (this->type == "Dual Daggers")
                                {   //  Amar's Contempt
                                    this->weaponModifiers["Damage"] = 90;
                                    this->statusTypeModifiers["Slash"] = 30;
                                }
                                else if (this->type == "Polearms")
                                {   //  Boreal's Contempt
                                    this->weaponModifiers["Damage"] = 90;
                                    this->weaponModifiers["Status Damage"] = 60;
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
                        this->weaponModifiers["Magazine Capacity"] = -60;
                        this->weaponModifiers["Reload Speed"] = 48;
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
        }
};





// set to be pruned bool (for mods that are useless for dps, or lower variants of other mods)       -       then have the main pass over all entries in validMods vector and drop any that have the pruneThis bool set to true
