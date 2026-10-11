# WarframeDamageOptimizer

## *** TLDR SUMMARY ***
This app calculates the optimal combination of 8 distinct mods and 1 arcane for each weapon, for each damage type (Single Shot, Burst DPS, Sustained DPS) for a chosen weapon, from a pool of hundreds of mods and dozens of arcanes across the weapon types.

## *** PERFORMANCE ***
All tests were run on my computer, with the following main specs:
CPU:    AMD Ryzen 5 7600X 6-Core Processor
GPU:    NVIDIA GeForce RTX 4070 Ti SUPER
RAM:    64 GB 6000 MHz DDR5

### *** CURRENT PERFORMANCE ***
Computation time is noted in seconds
Total Search Space is the theoretical count of possible mod combinations
Completed Calculations is the amount of combinations actually calculated, not pruned by a conflicting mod
Throughput is the number of configurations checked per seconds, rounded to the nearest full digit
|Weapon Name|Computation Time|Total Search Space|Completed Calculations|Throughput|
|:---------:|:--------------:|:----------------:|:--------------------:|:--------:|
|Rubico Prime|1.648723|342324444|302391236|183409363|
|Rubico Prime|1.664816|342324444|302391236|181636431|
|Rubico Prime|1.710189|342324444|302391236|176817437|
|Rubico Prime|1.709812|342324444|302391236|176856424|
|Rubico Prime|1.687255|342324444|302391236|179220827|
|Rubico Prime|1.674491|342324444|302391236|180586958|
|Ocucor|5.781970|1450085130|1360979880|235383421|
|Ocucor|5.638175|1450085130|1360979880|241386598|
|Ocucor|5.711492|1450085130|1360979880|238287978|
|Ocucor|5.699326|1450085130|1360979880|238796637|
|Ocucor|5.775666|1450085130|1360979880|235640337|
|Ocucor|5.717009|1450085130|1360979880|238058027|
|Gram Prime|2.270595|453905100|433731540|191021094|
|Gram Prime|2.393940|453905100|433731540|181178952|
|Gram Prime|2.404077|453905100|433731540|180414995|
|Gram Prime|2.309869|453905100|433731540|187773220|
|Gram Prime|2.356009|453905100|433731540|184095876|
| Gram Prime |2.357213|453905100|433731540|184001845|

#### *** TESTED WEAPONS AVERAGE STATS ***
|Weapon Name|Average Computation Time|Average Throughput|
|:---------:|:----------------------:|:----------------:|
|Rubico Prime|1.68255|179754573|
|Ocucor|5.72061|237925499|
|Gram Prime|2.34862|184747664|


### *** OPTIMIZATION HISTORY ***
WIP SECTION   I don't want to put data here before I go back and reimplement the trackers for completed calcs and whatnot in old implementations to get exact data, so this section is a WIP. Some general data is in the commit messaged for the rubico prime test case, but they are general estimates not exact data so I am not putting them here.
|Weapon Name|Computation Time|Total Search Space|Optimizations|Notes|
|:---------:|:--------------:|:----------------:|:-----------:|:---:|

## *** Project Summary ***
### *** SUMMARY FOR THOSE THAT DON"T KNOW WARFRAME ***
Warframe has over 600 weapons, hundreds of possible mods to put on those weapons, and dozens of arcane choices as well. When you modify a weapon, there is one slot for arcane upgrades and eight slots for mods. There are some other things like weapon stance mods and exilus mods, but exilus mods are intended to be mostly QOL buffs, and stance mods are a lot more restricted and more preference based.
To determine the maximum possible (averaged) single shot, burst DPS, and sustained DPS this program uses a brute force approach. Any genetic algorithms may miss a more complex mod config and not reach the absolute global peak values. Almost any pruning algorithm would need to do enough of the calculation to prove that the mod config cannot possibly outdo the current best that all the math is done anyways. For example, pruning any config that has a lower single shot damage won't work because the right fire rate could make the weapon's burst DPS higher. Similarly, pruning configurations based on burst DPS may prune a configuration that has a higher sustained DPS due to higher magazine size and/or reload speed. Also, Warframe has weird interactions sometimes, so I want to cover absolutely every possibility in case an odd combo happens to have higher sustained DPS.
There is an included python script to pull the data from the Warframe wiki (originally the Warframe public export, but the wiki has more detail) to update the included data to the most recent mods, arcanes, and weapons. The main program then uses user input to filter all the mods to only handle valid ones, do the same for arcanes, prune any that do not affect DPS at all, prune any mods that have a strictly better version for DPS, and then it begins applying them. Secondary and Melee weapons tend to have more attacks and arcanes, and this program finds the best mods for each attack, which includes trying every mod config with every arcane for every attack, so that increases the calculation time notably.


### *** SUMMARY FOR THOSE THAT KNOW WARFRAME ***
It has been a longstanding hope of mine to create an application that can calculate the mathematically highest damage (single shot, burst DPS, & sustained DPS) possible in Warframe. This app does not do that, at least not yet. What it does do is calculate the mathematically highest damage (single shot, burst DPS, & sustained DPS) for implemented mods and arcanes. For now that means it does not take into account innate weapon buffs like incarnon mods, kuva weapon mods, etc. It also does not (yet) account for warframe buffs or warframe slot arcanes that buff weapons (like Arcane Arachne).
I don't think anything can because not even DE actually knows all the weird interactions, and, try as I might I don't want to add hyperoptimizations like shooting through a Mutalist Quanta orb, or calculating the optimized arcane and mods per attack per frame, etc. I am making the baseline, these builds *can* be brought further with more work and more interactions (Warframes, Abilities, Items, Teammates, Rivens, etc.), but this should make a very good starting point using the currently implemented features.

## *** Assumptions ***
1. The 'enemy' being attacked is high enough level to have the capped armor value to start, and will survive an infinite amount of damage (just so I don't need to account for changing targets).
2. The enemy does not have status effect caps (like bosses and/or acolytes)
3. Anything that is a chance of happening will be averaged. If a weapon has a fire rate of 1/1.4999 seconds or something, and needs to apply blast to not have it go off, with a 50% blast status chance, then I will assume it will happen, if it was 49.999% it won't, and so on.
4. No multipliers, positive or negative, for damage types against an enemy type (ie: corpus & toxic), to ensure the builds work across factions.
5. Arcanes and temporary buffs like Galvanized mods will be assumed at max stacks if they are based on kills or warframe stats (armor, max energy, etc.), and will be at their avg stacks over an infinite amount of time if they are like Primary Blight
6. Heat Armor strip won't be considered, because an enemy would need to live 2 seconds for that and hopefully that won't happen. You can always outsource armor strip to styanax's helminth or corrosive shards/hydroid/etc.        -   Maybe changed with toggles, but not yet

Why: Removing faction specific mods like bane, cleanse, etc. reduces the mod configs drastically speeding up calculation time. Also, there is no way for me to know which faction you are going against, and not all factions even have a corresponding mod anyways, and most players don't like using them. Me included.



## *** Roadmap ***
1. Load All Data                                                                -   [X]
    1a. Parse weapons into Weapon class, and adjust DPS calc to match           -   [X]
2. Filter to only include compatible mods                                       -   [X]
3. Find best mods for raw damage                                                -   [X]
    3a. Find way to turn written description into stats that can be used    -   manual parser to allow for maximum control in development and testing phase
4. Account for Status DOT                                                       -   [X]
5. Account for Status Effect Debuffs (Cold/Puncture)                            -   [X]
6. Account for Arcanes                                                  -   50% DONE (simple arcanes done, ones with more complex triggers, like activating a stack on heat and/or toxin status effects, or enemies taking increased damage based on cold stacks are not implemented *yet*)
7. Add ability to set 'minimum' values on things like fire rate, accuracy, etc. -   [ ]
8. Add ability to toggle whether heat strips armor, maybe toggle                -   [ ]
9. Weapon augment mods                                                          -   [ ]
10. Account for Warframe Buffs // Maybe?                                        -   [ ]
11. Account for Sentinel Primers // Maybe?                                      -   [ ]


## *** Acknowledgements & Data Sources ***
Json parsing thanks to nlohmann/json

Data in json files for Warframes and Sentinels was downloaded from Digital Extremes public export on 9/18/2026
Data in json files under wikiData was downloaded from the Warframe Wiki's public API on 9/19/2026




// *** General info for project pseudocode ***



// load all items
// take input about weapon type (Primary/Secondary/Melee)
// take input about weapon name

//  get weapon
//
//  filter mods for compatible ones only
//  prune lower copies of mods (for example if prime exists, ignore base)
//  prune certain mods based on user input (spectral serration, acuity, etc.) (for now just pruned automatically if they are too specific, like weakpoint only or only midair or something)
//  parse mods to get actual effects on weapon from description info - manually written parser for now, allows maximum control
//  
//  Filter compatible arcanes
//  prune arcanes based on user choices
//  
//  for each attack on the weapon
//    for each possible arcane
//          copy weapon to test mods
//              8 nested for loops. First starts at index 1 and ends at index END-7, last starts at index 8 and ends at index END
//                  in each for loop:
//                      1. if mod in [current loop selection] is incompatible with any mod selected by [higher ranked loop] skip to next mod
//                      2. try this mod in this slot
//                at the end of the loops, then 8 mods that are compatible are chosen:
//                      if elemental mods are included
//                          for permutation of elemental mods
//                              calculate damage for that order of elemental mods in this setup
//                                  if damage is higher for single shot, burst, or sustained dps then log arcane & mod config for that value (single shot/burst/sustained)
//                                  next mod at top for loop
//                                  if top for loop is at the end, increment previous loop and try again starting from previous loop+1 to loops final spot








//  ***Explanation of Single Shot Damage, Burst DPS, and Sustained DPS calculations in regards to status effects and DOTs***

// Single Shot does not take into account status effect damage or status effects in damage calculation (ie: no health damage buff for viral)
// Burst will take into account status effects on damage, like cold, viral, corrosive, puncture, etc. but not heat or their DOT damage, burst should be under 2 seconds (hopefully you are killing what you are shooting in that time)
// Burst and Sustained will use averaged over time amount of each elements effects on the enemy (ie: if you have 2 stacks on an enemy half the time and 3 stacks the other half, the value of 2.5 stacks will be calculated with, although not possible it should be more accurate over time)

// Sustained will take status effect into damage calcs, and count all status effect damage & heat armor strip, using the averaged dps and effects explained above