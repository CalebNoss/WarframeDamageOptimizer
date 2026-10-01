#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include "weaponMod.hpp"

class weaponModConfig
{
    // make this so I can apply a mod using a function passing in a weaponMod class to modify this
    // mods will always be valid


    public:
        std::vector<std::string> currentMods = {};          // tags for incompatible weapons    (if they exist)
        std::vector<std::string> elementalMods = {};        // values affected                  (always)
        std::vector<std::string> elementalModsOrder = {};   // incompatible mods                (if they exist)
        
        std::unordered_map<std::string, double> weaponModifiers = {        // the comment is the string to regex for when parsing a mod description
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
            {"isFireRateLocked", 0},    // "Fire Rate cannot be modified"
            {"isMultishotLocked", 0}    // "Multishot cannot be modified"
        };

        std::unordered_map<std::string, double> statusTypeModifiers = {        // the comment is the string to regex for when parsing a mod description
            {"Impact", 0},              // "<DT_IMPACT_COLOR>Impact"
            {"Puncture", 0},            // "<DT_PUNCTURE_COLOR>Puncture"
            {"Slash", 0},               // "<DT_SLASH_COLOR>Slash"
            {"Heat", 0},                // "<DT_FIRE_COLOR>Heat"
            {"Cold", 0},                // "<DT_FREEZE_COLOR>Cold"
            {"Electricity", 0},            // "<DT_ELECTRICITY_COLOR>Electricity"
            {"Toxin", 0},               // "<DT_POISON_COLOR>Toxin"
            {"Blast", 0},               // "<DT_EXPLOSION_COLOR>Blast"
            {"Corrosive", 0},           // "<DT_CORROSIVE_COLOR>Corrosive"
            {"Gas", 0},                 // "<DT_GAS_COLOR>Gas"
            {"Magnetic", 0},            // "<DT_MAGNETIC_COLOR>Magnetic"
            {"Radiation", 0},           // "<DT_RADIATION_COLOR>Radiation"
            {"Viral", 0}                // "<DT_VIRAL_COLOR>Viral"
        };


        // create a base instance of this then add and remove mods as the loop goes, and apply the 'config' setup
        // this also lets me save a copy of the vector of mods at the time it outperforms the current best mods

        void addMod(weaponMod& currentMod) // make this constructor create a mod based on passed in data. I need to decide how to pass in the data from the json though
        {
            for (const auto& [modifierName, modifierAmount] : currentMod.weaponModifiers)
            {
                this->weaponModifiers[modifierName] += modifierAmount;
            }
            for (const auto& [modifierName, modifierAmount] : currentMod.statusTypeModifiers)
            {
                this->statusTypeModifiers[modifierName] += modifierAmount;
            }
            currentMods.push_back(currentMod.name);

            bool isElementalMod = false;
            for (const auto& [modifierName, modifierAmount] : currentMod.statusTypeModifiers)
            {
                if (modifierAmount != 0)
                {
                    isElementalMod = true;
                }
            }
            if (isElementalMod)
            {
                elementalMods.push_back(currentMod.name);
                elementalModsOrder.push_back(currentMod.name);
            }
        }
        void removeMod(weaponMod& currentMod) // make this constructor create a mod based on passed in data. I need to decide how to pass in the data from the json though
        {
            for (const auto& [modifierName, modifierAmount] : currentMod.weaponModifiers)
            {
                this->weaponModifiers[modifierName] -= modifierAmount;
            }
            for (const auto& [modifierName, modifierAmount] : currentMod.statusTypeModifiers)
            {
                this->statusTypeModifiers[modifierName] -= modifierAmount;
            }

            // remove from current list of mods
            auto modToBeRemovedIterator = std::find(currentMods.begin(), currentMods.end(), currentMod.name);
            if (modToBeRemovedIterator != currentMods.end())
            {
                currentMods.erase(modToBeRemovedIterator);
            }            

            // if it is in the list of elemental mods, remove it
            modToBeRemovedIterator = std::find(elementalMods.begin(), elementalMods.end(), currentMod.name);
            if (modToBeRemovedIterator != elementalMods.end())
            {
                elementalMods.erase(modToBeRemovedIterator);
            }            
            // if it is in the other list of elemental mods, remove it
            modToBeRemovedIterator = std::find(elementalModsOrder.begin(), elementalModsOrder.end(), currentMod.name);
            if (modToBeRemovedIterator != elementalModsOrder.end())
            {
                elementalModsOrder.erase(modToBeRemovedIterator);
            }            
        }
};


//  INSTEAD OF DOING THIS I SHOULD BE ABLE TO COMBINE ALL MODS INTO ONE WITH ANOTHER CONSTRUCTOR THAT TAKES A VECTOR OF MODS
//  I CAN APPLY THE MOD IN THE DPS DAMAGE CALCULATION FUNCTION, MAYBE PERMUTATE THE ELEMENTAL OPTIONS IN THE CONSTRUCTOR AND RETURN A VECTOR OF COMBINED MOD COMBOS
//  THIS CAN BE A NEW CLASS THAT IS LIKE A MOD WITH AN EXTRA VECTOR OF STRINGS FOR THE ORDER OF MODS, FIGURE OUT HOW TO TRY ELEMENTAL COMBOS LATER
