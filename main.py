import dearpygui.dearpygui as dpg
import math

def calculateFirearm(sender, app_data, user_data):
    fields = user_data

    # DO MORE WORK HERE, THIS IS TEMPORARY    
    moddedMagSize    = dpg.get_value(fields["baseMagSize"])
    moddedReloadSpeed = dpg.get_value(fields["baseReloadSpeed"])
    moddedFireRate   = dpg.get_value(fields["baseFireRate"])
    moddedCC         = dpg.get_value(fields["baseCC"])
    moddedCD         = dpg.get_value(fields["baseCD"])

    baseImpact    = dpg.get_value(fields["baseImpact"])
    basePuncture  = dpg.get_value(fields["basePuncture"])
    baseSlash     = dpg.get_value(fields["baseSlash"])
    baseHeat      = dpg.get_value(fields["baseHeat"])
    baseCold      = dpg.get_value(fields["baseCold"])
    baseElectric  = dpg.get_value(fields["baseElectric"])
    baseToxin     = dpg.get_value(fields["baseToxin"])
    baseBlast     = dpg.get_value(fields["baseBlast"])
    baseRadiation = dpg.get_value(fields["baseRadiation"])
    baseGas       = dpg.get_value(fields["baseGas"])
    baseMagnetic  = dpg.get_value(fields["baseMagnetic"])
    baseViral     = dpg.get_value(fields["baseViral"])
    baseCorrosive = dpg.get_value(fields["baseCorrosive"])
    baseMulti     = dpg.get_value(fields["baseMulti"])
    ammoPerShot   = dpg.get_value(fields["ammoPerShot"])
    isSemiAuto       = dpg.get_value(fields["isSemiAuto"])
    isBurst       = dpg.get_value(fields["isBurst"])
    isCharge      = dpg.get_value(fields["isCharge"])
    
    totalBaseIPS = baseImpact + basePuncture + baseSlash
    totalBaseHCET = baseHeat + baseCold + baseElectric + baseToxin
    totalBaseCombined = baseBlast + baseRadiation + baseGas + baseMagnetic + baseViral + baseCorrosive
    
    totalBaseDamage = totalBaseIPS + totalBaseHCET + totalBaseCombined
    
    # DO MORE WORK THESE ARE JUST A PLACEHOLDERS
    totalElementalBonuses = 0
    totalDamageBonuses = 0
    totalMultishotBonuses = 0
    impactBonuses = 0
    punctureBonuses = 0
    slashBonuses = 0
    
    
    totalDamage = totalBaseDamage * (1 + totalElementalBonuses + (
        (baseImpact / totalBaseIPS) * impactBonuses) + (
        (basePuncture / totalBaseIPS) * punctureBonuses) + (
        (baseSlash / totalBaseIPS) * slashBonuses)
    ) * (1 + totalDamageBonuses) * (baseMulti * (1 + totalMultishotBonuses))
    
    AVG_NormalShot = totalDamage * (1 + math.floor(moddedCC) * (moddedCD - 1))
    
    AVG_CritShot = totalDamage * (1 + math.ceil(moddedCC) * (moddedCD - 1))
    
    final_AVG_Shot = totalDamage * (1 + moddedCC * (moddedCD - 1))
    
    if (isBurst):
        # DO MORE WORK HERE, THIS IS TEMPORARY
        effectiveFireRate = moddedFireRate
    elif (isCharge):
        # DO MORE WORK HERE, THIS IS TEMPORARY
        effectiveFireRate = moddedFireRate
    else:
        effectiveFireRate = moddedFireRate
        
    
    final_BURST_DPS = final_AVG_Shot * effectiveFireRate
    
    numShotsPerMag = moddedMagSize/ammoPerShot
    
    timeShooting_vs_Reloading = (numShotsPerMag / (effectiveFireRate * moddedReloadSpeed) + numShotsPerMag)
    
    final_AVG_DPS = final_BURST_DPS * timeShooting_vs_Reloading
    result = f"Avg DPS is: {final_AVG_DPS:.2f}"
    
    dpg.set_value(fields["primaryOutputText"], result)
    
        

def main():
    dpg.create_context()
    dpg.create_viewport(title='Custom Title', width=500, height=700)

    with dpg.window(tag="Damage Optimizer"):
        with dpg.tab_bar(label="tabBar"):
            with dpg.tab(label="primaryWeapons"):
                dpg.add_text("primaryWeapons")
                isMultCO = dpg.add_checkbox(label="Condition Overload Effects are multiplicitive")
                isSemiAuto = dpg.add_checkbox(label="Semi Auto Fire Type")
                isBurst = dpg.add_checkbox(label="Burst Fire Type")
                isCharge = dpg.add_checkbox(label="Charge Fire Type")
                baseMulti = dpg.add_input_int(label="Innate Multishot", default_value=1)
                baseFireRate = dpg.add_input_int(label="Innate Fire Rate", default_value=1)
                ammoPerShot = dpg.add_input_int(label="Ammo Per Shot", default_value=1)
                baseCC = dpg.add_input_int(label="Innate Crit Chance")
                baseCD = dpg.add_input_int(label="Innate Crit Damage")
                baseSC = dpg.add_input_int(label="Innate Status Chance")
                baseReloadSpeed = dpg.add_input_int(label="Innate Reload Speed", default_value=1)
                baseMagSize = dpg.add_input_int(label="Innate Mag Size", default_value=1)
                baseImpact = dpg.add_input_int(label="Innate Impact Damage")
                basePuncture = dpg.add_input_int(label="Innate Puncture Damage")
                baseSlash = dpg.add_input_int(label="Innate Slash Damage", default_value=1)
                baseHeat = dpg.add_input_int(label="Innate Heat Damage")
                baseCold = dpg.add_input_int(label="Innate Cold Damage")
                baseElectric = dpg.add_input_int(label="Innate Electric Damage")
                baseToxin = dpg.add_input_int(label="Innate Toxin Damage")
                baseBlast = dpg.add_input_int(label="Innate Blast Damage")
                baseRadiation = dpg.add_input_int(label="Innate Radiation Damage")
                baseGas = dpg.add_input_int(label="Innate Gas Damage")
                baseMagnetic = dpg.add_input_int(label="Innate Magnetic Damage")
                baseViral = dpg.add_input_int(label="Innate Viral Damage")
                baseCorrosive = dpg.add_input_int(label="Innate Corrosive Damage")
                dpg.add_button(label="calculate",
                               callback=calculateFirearm,
                               user_data={
                                   "isBurst": isBurst, "isCharge": isCharge, "isMultiCO": isMultCO,
                                   "baseMulti": baseMulti, "baseFireRate": baseFireRate,
                                   "baseCC": baseCC, "baseCD": baseCD, "baseSC": baseSC,
                                   "baseReloadSpeed": baseReloadSpeed, "baseMagSize": baseMagSize, "ammoPerShot": ammoPerShot,
                                   "baseImpact": baseImpact, "basePuncture": basePuncture, "baseSlash": baseSlash,
                                   "baseHeat": baseHeat, "baseCold": baseCold, "baseElectric": baseElectric, "baseToxin": baseToxin,
                                   "baseBlast": baseBlast, "baseRadiation": baseRadiation, "baseGas": baseGas,
                                   "baseMagnetic": baseMagnetic, "baseViral": baseViral, "baseCorrosive": baseCorrosive,
                                   "primaryOutputText": "primaryWeaponOutput"
                               })
                dpg.add_separator
                dpg.add_text("Avg DPS is: 0", tag="primaryWeaponOutput")
            with dpg.tab(label="secondaryWeapons"):
                dpg.add_text("secondaryWeapons")
                dpg.add_checkbox(label="Condition Overload Effects are multiplicitive")
                dpg.add_button(label="calculate")
            with dpg.tab(label="meleeWeapons"):
                dpg.add_text("meleeWeapons")
                dpg.add_checkbox(label="Condition Overload Effects are multiplicitive")
                dpg.add_button(label="calculate")
            with dpg.tab(label="archgunWeapons"):
                dpg.add_text("archgunWeapons")
                dpg.add_checkbox(label="Condition Overload Effects are multiplicitive")
                dpg.add_button(label="calculate")
            with dpg.tab(label="archmeleeWeapons"):
                dpg.add_text("archmeleeWeapons")
                dpg.add_checkbox(label="Condition Overload Effects are multiplicitive")
                dpg.add_button(label="calculate")
            with dpg.tab(label="warframes"):
                dpg.add_text("warframes")
                dpg.add_button(label="calculate")
            with dpg.tab(label="companions"):
                dpg.add_text("companions")
                dpg.add_button(label="calculate")
    
    dpg.set_primary_window("Damage Optimizer", True)
    dpg.setup_dearpygui()
    dpg.show_viewport()
    dpg.start_dearpygui()
    dpg.destroy_context()
    
main()