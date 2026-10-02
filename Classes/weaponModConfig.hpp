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
        

        double weaponModifiers[13] = {0};
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

        double statusTypeModifiers[13] = {0};
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


        // create a base instance of this then add and remove mods as the loop goes, and apply the 'config' setup
        // this also lets me save a copy of the vector of mods at the time it outperforms the current best mods

        void addMod(weaponMod& currentMod) // make this constructor create a mod based on passed in data. I need to decide how to pass in the data from the json though
        {
            for (int i = 0; i < 13; i++)
            {
                this->weaponModifiers[i] += currentMod.weaponModifiers[i];
            }
            for (int i = 0; i < 13; i++)
            {
                this->statusTypeModifiers[i] += currentMod.statusTypeModifiers[i];
            }
            currentMods.push_back(currentMod.name);

            bool isElementalMod = false;
            for (int i = 0; i < 13; i++)
            {
                if (currentMod.statusTypeModifiers[i] != 0)
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
            for (int i = 0; i < 13; i++)
            {
                this->weaponModifiers[i] -= currentMod.weaponModifiers[i];
            }
            for (int i = 0; i < 13; i++)
            {
                this->statusTypeModifiers[i] -= currentMod.statusTypeModifiers[i];
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
