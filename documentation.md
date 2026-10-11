// ***General info for project pseudocode***



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