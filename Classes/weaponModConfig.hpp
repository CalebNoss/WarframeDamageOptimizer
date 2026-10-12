#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <bit>
#include "weaponMod.hpp"
#include "weaponArcane.hpp"

// by multiplying the values at the indices for base HCET elements (0-3) you get the index to check
// in this array for the combined type.
// The value at that index in this array is the index in the normal array for the combined status type
// For removing mods, I can divide the index of the combined type by the 
std::array<int, 21> combinedStatusIndexArray = {
    2,  // index 0,     Heat
    3,  // index 1,     Cold
    4,  // index 2,     Electric
    5,  // index 3,     Toxin
    -1, // index 4,     
    -1, // index 5,     
    7,  // index 6,     Blast
    -1, // index 7,     
    11, // index 8,     Radiation
    -1, // index 9,     
    9,  // index 10,    Gas
    -1, // index 11,    
    10, // index 12,    Magnetic
    -1, // index 13,    
    -1, // index 14,    
    12, // index 15,    Viral
    -1, // index 16,    
    -1, // index 17,    
    -1, // index 18,    
    -1, // index 19,    
    8   // index 20,    Corrosive
};



class weaponModConfig
{
    // make this so I can apply a mod using a function passing in a weaponMod class to modify this
    // mods will always be valid


    public:
        // for these 0 can never be an elemental mod nor can it appear more than once, 0 is a placeholder. it being in anything but the first slot in currentModIndices is a bug, or, simply no mod being in that slot
        std::array<int, 8> currentModIndices = {0};          // indices of current mods (index in reference to validMods)
        std::array<int, 8> elementalMods = {0};        // indices of current elemental mods (index in reference to validMods)

        // this is an array of whether each element was added directly by mods (so I can devolve the elements when removing mods)
        std::array<bool, 10> addedElements = {false};
        //Entry 0: Heat
        //Entry 1: Cold
        //Entry 2: Electricity
        //Entry 3: Toxin
        //Entry 4: Blast"
        //Entry 5: Corrosive
        //Entry 6: Gas  
        //Entry 7: Magnetic
        //Entry 8: Radiation
        //Entry 9: Viral

        // this array counts the order each base element was added in. Default value of -1 is for elements that aren't added
        std::array<int, 4> orderOfAddedBaseElements = {-1, -1, -1, -1};
        // if value == 0 then it is heat
        // if value == 1 then it is cold
        // if value == 2 then it is electric
        // if value == 3 then it is toxin
        
        std::array<int, 4> countOfBaseElementMods = {0};
        // index 0 is Heat      // value is the amount of mods that give that element
        // index 1 is Cold      // value is the amount of mods that give that element
        // index 2 is Electric  // value is the amount of mods that give that element
        // index 3 is Toxin     // value is the amount of mods that give that element

        std::array<double, 11> weaponModifiers = {0};
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

        std::array<double, 13> uncombinedStatusTypeModifiers = {0};


        unsigned int modifiedStatusTypeIndicesMask = 0;
        std::array<unsigned int, 8> modStatusTypeMasks = {};

        bool arcaneBuff = false;
        int activeArcaneIndex = -1;

        // create a base instance of this then add and remove mods as the loop goes, and apply the 'config' setup
        // this also lets me save a copy of the vector of mods at the time it outperforms the current best mods

        [[msvc::noinline]] unsigned int getStatusTypeMask() // make this constructor create a mod based on passed in data. I need to decide how to pass in the data from the json though
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

        [[msvc::noinline]] void addArcane(weaponArcane& currentArcane) // make this constructor create a mod based on passed in data. I need to decide how to pass in the data from the json though
        {
            if (currentArcane.modifiesWeapon)
            {
                unsigned int indicesToCheck = currentArcane.weaponModifierMask;
                while (indicesToCheck)
                {
                    int modifiedStatIndex = std::countr_zero(indicesToCheck);
                    this->weaponModifiers[modifiedStatIndex] += currentArcane.weaponModifiers[modifiedStatIndex];
                    indicesToCheck &= indicesToCheck - 1;   //  this gets the result only where these match, subtracting one makes the lowest valued bit shift, so it doesn't match, so it removes the lowest bit set to 1
                }
            }
        }

        [[msvc::noinline]] void removeArcane(weaponArcane& currentArcane) // make this constructor create a mod based on passed in data. I need to decide how to pass in the data from the json though
        {
            if (currentArcane.modifiesWeapon)
            {
                unsigned int indicesToCheck = currentArcane.weaponModifierMask;
                while (indicesToCheck)
                {
                    int modifiedStatIndex = std::countr_zero(indicesToCheck);
                    this->weaponModifiers[modifiedStatIndex] -= currentArcane.weaponModifiers[modifiedStatIndex];
                    indicesToCheck &= indicesToCheck - 1;   //  this gets the result only where these match, subtracting one makes the lowest valued bit shift, so it doesn't match, so it removes the lowest bit set to 1
                }
            }
            else
            {
                this->arcaneBuff = true;
                this->activeArcaneIndex = currentArcane.arcaneModifierIndex;
            }
        }

        [[msvc::noinline]] void addMod(weaponMod& currentMod, int modIndex, int modSlot) // make this constructor create a mod based on passed in data. I need to decide how to pass in the data from the json though
        {
            unsigned int indicesToCheck = currentMod.weaponModifierMask;
            while (indicesToCheck)
            {
                int modifiedStatIndex = std::countr_zero(indicesToCheck);
                this->weaponModifiers[modifiedStatIndex] += currentMod.weaponModifiers[modifiedStatIndex];
                indicesToCheck &= indicesToCheck - 1;   //  this gets the result only where these match, subtracting one makes the lowest valued bit shift, so it doesn't match, so it removes the lowest bit set to 1
            }
            if (currentMod.modifiesElements)
            {
                indicesToCheck = currentMod.modifiedElementIndicesMask;
                while (indicesToCheck)
                {
                    int modifiedElementIndex = std::countr_zero(indicesToCheck);
                    this->statusTypeModifiers[modifiedElementIndex] += currentMod.statusTypeModifiers[modifiedElementIndex];
                    this->uncombinedStatusTypeModifiers[modifiedElementIndex] += currentMod.statusTypeModifiers[modifiedElementIndex];
                    indicesToCheck &= indicesToCheck - 1;   //  this gets the result only where these match, subtracting one makes the lowest valued bit shift, so it doesn't match, so it removes the lowest bit set to 1
                }
            }

            this->locksFireRate = currentMod.locksFireRate;
            this->locksMultishot = currentMod.locksMultishot;

            currentModIndices[modSlot] = currentMod.indexInValidMods;
            this->modStatusTypeMasks[modSlot] = currentMod.modifiedElementIndicesMask;

            if (currentMod.isElementalMod)
            {
                elementalMods[modSlot] = currentMod.indexInValidMods;
                if (currentMod.isBaseElementalMod)
                {
                    int baseElementOrderIndex = 0;
                    int currModBaseElementType = currentMod.selfBaseElementIndex; // 0-3 in HCET order
                    while (this->orderOfAddedBaseElements[baseElementOrderIndex] != -1 && this->orderOfAddedBaseElements[baseElementOrderIndex] != currModBaseElementType)
                    {
                        baseElementOrderIndex++;
                    }
                    this->orderOfAddedBaseElements[baseElementOrderIndex] = currModBaseElementType;
                    this->countOfBaseElementMods[currModBaseElementType]++;
                    if (baseElementOrderIndex & 1)  // if the number is odd (second or fourth status type added)
                    {
                        int baseStatusOneIndex = this->orderOfAddedBaseElements[baseElementOrderIndex - 1] + 3;
                        int baseStatusTwoIndex = this->orderOfAddedBaseElements[baseElementOrderIndex] + 3;
                        int combinedStatusIndex = combinedStatusIndexArray[(combinedStatusIndexArray[baseStatusOneIndex - 3] * combinedStatusIndexArray[baseStatusTwoIndex - 3])];
                        this->statusTypeModifiers[combinedStatusIndex] += this->statusTypeModifiers[baseStatusOneIndex] + this->statusTypeModifiers[baseStatusTwoIndex];
                        this->statusTypeModifiers[baseStatusOneIndex] = 0;
                        this->statusTypeModifiers[baseStatusTwoIndex] = 0;
                    }
                }
            }
        }
        
        [[msvc::noinline]] void removeMod(weaponMod& currentMod, int modIndex, int modSlot) // make this constructor create a mod based on passed in data. I need to decide how to pass in the data from the json though
        {
            unsigned int indicesToCheck = currentMod.weaponModifierMask;
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
                    this->uncombinedStatusTypeModifiers[modifiedElementIndex] -= currentMod.statusTypeModifiers[modifiedElementIndex];
                    indicesToCheck &= indicesToCheck - 1;   //  this gets the result only where these match, subtracting one makes the lowest valued bit shift, so it doesn't match, so it removes the lowest bit set to 1
                }
            }

            if (this->locksFireRate && currentMod.locksFireRate)
            {
                this->locksFireRate = false;
            }
            if (this->locksMultishot && currentMod.locksMultishot)
            {
                this->locksMultishot = false;
            }

            currentModIndices[modSlot] = -1;                 //  remove from current mods (will always be last because of how mods are popped)

            this->modStatusTypeMasks[modSlot] = 0;

            if (currentMod.isElementalMod)
            {
                this->elementalMods[modSlot] = -1;     //  remove from elemental mods (will always be last because of how mods are popped)
                if (currentMod.isBaseElementalMod)
                {
                    int currModBaseElementType = currentMod.selfBaseElementIndex; // 0-3 in HCET order
                    this->countOfBaseElementMods[currModBaseElementType]--;
                    if (this->countOfBaseElementMods[currModBaseElementType] == 0)  // if there is no other mod before this that has this element the combined type will be split
                    {
                        // this should always be removing the last added mod, so it should always be the final filled position
                        int baseElementOrderIndex = this->orderOfAddedBaseElements.size() - 1;
                        while (this->orderOfAddedBaseElements[baseElementOrderIndex] != -1)
                        {
                            baseElementOrderIndex++;
                        }
                        this->orderOfAddedBaseElements[baseElementOrderIndex - 1] = -1;

                        if (baseElementOrderIndex & 1)  // if the number is odd (second or fourth status type removed)
                        {
                            int baseStatusOneIndex = this->orderOfAddedBaseElements[baseElementOrderIndex - 1] + 3;
                            int baseStatusTwoIndex = this->orderOfAddedBaseElements[baseElementOrderIndex] + 3;
                            int combinedStatusIndex = combinedStatusIndexArray[(combinedStatusIndexArray[baseStatusOneIndex - 3] * combinedStatusIndexArray[baseStatusTwoIndex - 3])];
                            this->statusTypeModifiers[combinedStatusIndex] -= (this->uncombinedStatusTypeModifiers[baseStatusOneIndex] + this->uncombinedStatusTypeModifiers[baseStatusTwoIndex]);
                            this->statusTypeModifiers[baseStatusOneIndex] = this->uncombinedStatusTypeModifiers[baseStatusOneIndex];
                            this->statusTypeModifiers[baseStatusTwoIndex] = this->uncombinedStatusTypeModifiers[baseStatusTwoIndex];
                        }
                    }
                    else
                    {   //  there is another mod with this status type in a higher priority slot, so the combined type will stay
                        int baseElementOrderIndex = this->orderOfAddedBaseElements.size() - 1;
                        while (this->orderOfAddedBaseElements[baseElementOrderIndex] != currentMod.selfBaseElementIndex)
                        {
                            baseElementOrderIndex--;
                        }

                        if (baseElementOrderIndex == 0 || baseElementOrderIndex == 1)
                        {
                            int baseStatusOneIndex = this->orderOfAddedBaseElements[0] + 3;
                            int baseStatusTwoIndex = this->orderOfAddedBaseElements[1] + 3;
                            int baseStatusToBeModifiedIndex = this->orderOfAddedBaseElements[baseElementOrderIndex] + 3;
                            int combinedStatusIndex = combinedStatusIndexArray[(combinedStatusIndexArray[baseStatusOneIndex - 3] * combinedStatusIndexArray[baseStatusTwoIndex - 3])];
                            this->statusTypeModifiers[combinedStatusIndex] -= currentMod.statusTypeModifiers[baseStatusToBeModifiedIndex];
                            this->uncombinedStatusTypeModifiers[this->orderOfAddedBaseElements[baseElementOrderIndex] + 3] -= currentMod.statusTypeModifiers[baseStatusToBeModifiedIndex];
                        }
                        else if (baseElementOrderIndex == 2 || baseElementOrderIndex == 3)
                        {
                            int baseStatusOneIndex = this->orderOfAddedBaseElements[2] + 3;
                            int baseStatusTwoIndex = this->orderOfAddedBaseElements[3] + 3;
                            int baseStatusToBeModifiedIndex = this->orderOfAddedBaseElements[baseElementOrderIndex] + 3;
                            int combinedStatusIndex = combinedStatusIndexArray[(combinedStatusIndexArray[baseStatusOneIndex - 3] * combinedStatusIndexArray[baseStatusTwoIndex - 3])];
                            this->statusTypeModifiers[combinedStatusIndex] -= currentMod.statusTypeModifiers[baseStatusToBeModifiedIndex];
                            this->uncombinedStatusTypeModifiers[this->orderOfAddedBaseElements[baseElementOrderIndex] + 3] -= currentMod.statusTypeModifiers[baseStatusToBeModifiedIndex];
                        }
                    }
                }
            }
        }
};


//  INSTEAD OF DOING THIS I SHOULD BE ABLE TO COMBINE ALL MODS INTO ONE WITH ANOTHER CONSTRUCTOR THAT TAKES A VECTOR OF MODS
//  I CAN APPLY THE MOD IN THE DPS DAMAGE CALCULATION FUNCTION, MAYBE PERMUTATE THE ELEMENTAL OPTIONS IN THE CONSTRUCTOR AND RETURN A VECTOR OF COMBINED MOD COMBOS
//  THIS CAN BE A NEW CLASS THAT IS LIKE A MOD WITH AN EXTRA VECTOR OF STRINGS FOR THE ORDER OF MODS, FIGURE OUT HOW TO TRY ELEMENTAL COMBOS LATER
