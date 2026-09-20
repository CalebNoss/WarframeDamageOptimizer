# WarframeDamageOptimizer

Json parsing thanks to nlohmann/json

Data in json files downloaded from Digital Extremes public export on 9/18/2026
// Also removed second entry for MasteryReq that was in most weapon entries... I don't know why some weapons had it duplicated? not all, just some. Always identical as far as I found

SUMMARY
This has been a longstanding hope of mine to create an application that can caluclate the mathematically highest damage (single shot, burst DPS, & sustained DPS) possible in Warframe. This app does not do that, at least not yet.
I don't think anything can because not even DE actually knows all the weird interactions, and, try as I might I don't want to add hyperoptimizations like shooting through a Mutalist Quanta orb, or optimize per frame, etc. I am making the baseline, these builds *can* be brought further with more work and more interactions (Warframes, Abilities, Items, Teammates, Rivens, etc.), but this should make a very good starting point.

//
Assumptions:
1. The 'enemy' being attacked is high enough level to have the capped armor value to start, and will survive an infinite amount of damage (just so I don't need to account for changing targets).
2. The enemy does not have status effect caps (like bosses and/or acolytes)
3. Anything that is a chance of happening will be averaged. If a weapon has a fire rate of 1/1.4999 seconds or something, and needs to apply blast to not have it go off, with a 50% blast status chance, then I will assume it will happen, if it was 49.999% it won't, and so on.
4. No multipliers, positive or negative, for damage types against an enemy type (ie: corpus & toxic), to ensure the builds work across factions.
5. Arcanes and temporary buffs like Galvanized mods will be assumed at max stacks (unless they only work with toxin/cold in which case... IDK I will try to calculate how often they will be up considering the fire rate and avg procs/second and whatnot)
6. Heat Armor strip won't be considered, because an enemy would need to live 2 seconds for that and hopefully that won't happen. You can always outsource armor strip to styanax's helminth or corrosive shards/hydroid/etc.        -   Maybe changed with toggles, but not yet

Why: I made this and I don't like changing builds for different factions, so, sorry to the true minmaxers but I am also not using bane mods or adjusting for faction because that... no. I don't want to.
//





Work Stages
1. Load All Data // ---------------------------------------------------- DONE

// For the start, only work with the first weapon
// I have to include order of mods to take into account for status effect types & dps later on
2. Filter to only include compatible mods
    2a. fix charge time data

3. Find best mods for raw damage

    3a. Find way to turn written description into stats that can be used

4. Account for Status DOT

5. Account for Satus Effect Debuffs

6. Account for Arcanes

7. Add ability to set 'minimum' values on things like fire rate, accuracy, etc.

8. Add ability to toggle whether heat strips armor, maybe toggle

9. Account for Warframes // Maybe?

10. Account for Sentinels // Maybe?





// *** General info for project pseudocode ***

// to make this better on resume I need to pull data from the API
// I also should make the profile info viewer pull data fresh from a user entered code, can explain how to get from EE.log file, would let me push to GitHub and share it
// figure out what on earth is up with the formatting of that and the json, make it fix formatting automatically
// also use wiki data for mods & primary/secondary/melees, it is more thorough


// load all items
// take input about weapon type (Primary/Secondary/Melee)
// take input about weapon name

//  get weapon
//
//  filter mods for compatible ones only
//  prune lower copies of mods (if prime exists, ignore base)
//  prune certain mods based on user input (spectral serration, acuity, etc.)
//  parse mods to get actual effects on weapon from description info    -   I could just pull from the wiki that has these better organized I think.
//  
//  Filter compatible arcanes
//  prune arcanes based on user choices
//  
//  for each possible arcane
//      copy weapon to test mods
//          8 nested for loops. First starts at index 1 and ends at index END-7, last starts at index 8 and ends at index END
//              in each for loop:
//                  1. if mod in [current loop selection] is incompatible with any mod selected by [higher ranked loop] skip to next mod
//                  2. try this mod in this slot
//              at the end of the loops, then 8 mods that are compatible are chosen:
//                  if elemental mods are included
//                      for permutation of elemental mods
//                          calculate damage for that order of elemental mods in this setup
//                              if damage is higher for single shot, burst, or sustained dps then log arcane & mod config for that value (single shot/burst/sustained)
//                              next mod at top for loop
//                              if top for loop is at the end, increment previous loop and try again starting from previous loop+1 to loops final spot




// This setup should later on allow me to add things like ticking the box and turning on buffs like roar, toxic lash, venom dose, thermal transfer, etc. with user inputted effect or power strength






//  ***Explanation of Single Shot Damage, Burst DPS, and Sustained DPS calculations in regards to status effects and DOTs***

// Single Shot does not take into account status effect damage or status effects in damage calculation (ie: no health damage buff for viral)
// Burst will take into account status effects on damage, like cold, viral, corrosive, puncture, etc. but not heat or their DOT damage, burst should be under 2 seconds (hopefully you are killing what you are shooting in that time)
// Burst and Sustained will use averaged over time amount of each elements effects on the enemy (ie: if you have 2 stacks on an enemy half the time and 3 stacks the other half, the value of 2.5 stacks will be calculated with, although not possible it should be more accurate over time)

// Sustained will take status effect into damage calcs, and count all status effect damage & heat armor strip, using the averaged dps and effects explained above




// choose mods based on