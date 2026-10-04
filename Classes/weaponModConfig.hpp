#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <bit>
#include "weaponMod.hpp"

class weaponModConfig
{
    // make this so I can apply a mod using a function passing in a weaponMod class to modify this
    // mods will always be valid


    public:
        std::vector<int> currentModIndices = {};          // indices of current mods (index in reference to validMods)
        std::vector<int> elementalMods = {};        // indices of current elemental mods (index in reference to validMods)
        

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

        std::array<double, 13> statusTypeModifiers = {0};
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

        uint16_t modifiedStatusTypeIndicesMask = 0;
        std::array<uint16_t, 8> modStatusTypeMasks = {};

        // create a base instance of this then add and remove mods as the loop goes, and apply the 'config' setup
        // this also lets me save a copy of the vector of mods at the time it outperforms the current best mods

        [[msvc::noinline]] uint16_t getStatusTypeMask() // make this constructor create a mod based on passed in data. I need to decide how to pass in the data from the json though
        {
            return (modStatusTypeMasks[0] ^
            modStatusTypeMasks[1] ^
            modStatusTypeMasks[2] ^
            modStatusTypeMasks[3] ^
            modStatusTypeMasks[4] ^
            modStatusTypeMasks[5] ^
            modStatusTypeMasks[6] ^
            modStatusTypeMasks[7])
            ;
        }

        [[msvc::noinline]] void addMod(weaponMod& currentMod, int modIndex) // make this constructor create a mod based on passed in data. I need to decide how to pass in the data from the json though
        {
            uint16_t indicesToCheck = currentMod.weaponModifierMask;
            while (indicesToCheck)
            {
                int modifiedStatIndex = std::countr_zero(indicesToCheck);
                this->weaponModifiers[modifiedStatIndex] += currentMod.weaponModifiers[modifiedStatIndex];
                indicesToCheck &= indicesToCheck - 1;   //  this gets the result only where these match, subtracting one makes the lowest valued bit shift, so it doesn't match, so it removes the lowest bit set to 1
            }
            if (currentMod.modifiesElements)
            {
                indicesToCheck = currentMod.modifiedElementIndicesMask;
                this->modifiedStatusTypeIndicesMask |= indicesToCheck; // add the indices of modified elements from this mod
                while (indicesToCheck)
                {
                    int modifiedElementIndex = std::countr_zero(indicesToCheck);
                    this->statusTypeModifiers[modifiedElementIndex] += currentMod.statusTypeModifiers[modifiedElementIndex];
                    indicesToCheck &= indicesToCheck - 1;   //  this gets the result only where these match, subtracting one makes the lowest valued bit shift, so it doesn't match, so it removes the lowest bit set to 1
                }
            }

            currentModIndices.push_back(currentMod.indexInValidMods);
            this->modStatusTypeMasks[currentModIndices.size() - 1] = currentMod.modifiedElementIndicesMask;

            if (currentMod.isElementalMod)
            {
                elementalMods.push_back(modIndex);
            }
        }
        [[msvc::noinline]] void removeMod(weaponMod& currentMod, int modIndex) // make this constructor create a mod based on passed in data. I need to decide how to pass in the data from the json though
        {
            uint16_t indicesToCheck = currentMod.weaponModifierMask;
            while (indicesToCheck)  // until indicesToCheck == 0
            {
                int modifiedStatIndex = std::countr_zero(indicesToCheck);
                this->weaponModifiers[modifiedStatIndex] -= currentMod.weaponModifiers[modifiedStatIndex];
                indicesToCheck &= indicesToCheck - 1;   //  this gets the result only where these match, subtracting one makes the lowest valued bit shift, so it doesn't match, so it removes the lowest bit set to 1
            }
            if (currentMod.modifiesElements)
            {
                indicesToCheck = currentMod.modifiedElementIndicesMask;
                while (indicesToCheck)
                {
                    int modifiedElementIndex = std::countr_zero(indicesToCheck);
                    this->statusTypeModifiers[modifiedElementIndex] -= currentMod.statusTypeModifiers[modifiedElementIndex];
                    indicesToCheck &= indicesToCheck - 1;   //  this gets the result only where these match, subtracting one makes the lowest valued bit shift, so it doesn't match, so it removes the lowest bit set to 1
                }
            }

            currentModIndices.pop_back();                 //  remove from current mods (will always be last because of how mods are popped)

            this->modStatusTypeMasks[currentModIndices.size() - 1] = 0;

            if (currentMod.isElementalMod)
            {
                this->elementalMods.pop_back();     //  remove from elemental mods (will always be last because of how mods are popped)
            }
        }
};


//  INSTEAD OF DOING THIS I SHOULD BE ABLE TO COMBINE ALL MODS INTO ONE WITH ANOTHER CONSTRUCTOR THAT TAKES A VECTOR OF MODS
//  I CAN APPLY THE MOD IN THE DPS DAMAGE CALCULATION FUNCTION, MAYBE PERMUTATE THE ELEMENTAL OPTIONS IN THE CONSTRUCTOR AND RETURN A VECTOR OF COMBINED MOD COMBOS
//  THIS CAN BE A NEW CLASS THAT IS LIKE A MOD WITH AN EXTRA VECTOR OF STRINGS FOR THE ORDER OF MODS, FIGURE OUT HOW TO TRY ELEMENTAL COMBOS LATER
