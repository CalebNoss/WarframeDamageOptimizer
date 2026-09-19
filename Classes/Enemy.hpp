class Enemy
{
    private:
        float baseArmor = 2700;
        float currentArmor = 2700; // armor taking into account heat & corrosive stacks
        float impactStatusCount = 0;
        float punctureStatusCount = 0;
        float slashStatusCount = 0;
        float heatStatusCount = 0;
        float coldStatusCount = 0;
        float electricStatusCount = 0;
        float toxinStatusCount = 0;
        float blastStatusCount = 0;
        float corrosiveStatusCount = 0;
        float gasStatusCount = 0;
        float magneticStatusCount = 0;
        float radiationStatusCount = 0;
        float viralStatusCount = 0;
        float tauStatusCount = 0;
        float heatInheritDamage = 0.0f;

        float addedCritChance = 0.0f; // added crit chance from puncture stacks
        float addedCritDamage = 0.0f; // added crit damage from cold stacks
        float multiplierToHealthDamage = 0.0f; // multiplier to health damage from viral stacks
        float addedStatusVulnerability = 0.0f; // added status chance from tau stacks
    // I was seeing if I need to track anything else, otherwise I need to make getters and setters, as well as something to auto calculate currentArmor and return it

    public:
        // specific getters
        float getArmor(bool heatArmorStrip);
        float getAddedCritChance();
        float getAddedCritDamage();
        float getMultiplierToHealthDamage();
        float getAddedStatusVulnerability();

        // general getters
        float getImpactStatusCount() const { return impactStatusCount; }
        float getPunctureStatusCount() const { return punctureStatusCount; }
        float getSlashStatusCount() const { return slashStatusCount; }
        float getHeatStatusCount() const { return heatStatusCount; }
        float getColdStatusCount() const { return coldStatusCount; }
        float getElectricStatusCount() const { return electricStatusCount; }
        float getToxinStatusCount() const { return toxinStatusCount; }
        float getBlastStatusCount() const { return blastStatusCount; }
        float getCorrosiveStatusCount() const { return corrosiveStatusCount; }
        float getGasStatusCount() const { return gasStatusCount; }
        float getMagneticStatusCount() const { return magneticStatusCount; }
        float getRadiationStatusCount() const { return radiationStatusCount; }
        float getViralStatusCount() const { return viralStatusCount; }
        float getTauStatusCount() const { return tauStatusCount; }
        float getHeatInheritDamage() const { return heatInheritDamage; }

        // general setters
        void setImpactStatusCount(float newCount) { impactStatusCount = newCount; }
        void setPunctureStatusCount(float newCount) { punctureStatusCount = newCount; }
        void setSlashStatusCount(float newCount) { slashStatusCount = newCount; }
        void setHeatStatusCount(float newCount) { heatStatusCount = newCount; }
        void setColdStatusCount(float newCount) { coldStatusCount = newCount; }
        void setElectricStatusCount(float newCount) { electricStatusCount = newCount; }
        void setToxinStatusCount(float newCount) { toxinStatusCount = newCount; }
        void setBlastStatusCount(float newCount) { blastStatusCount = newCount; }
        void setCorrosiveStatusCount(float newCount) {corrosiveStatusCount = newCount; }
        void setGasStatusCount(float newCount) { gasStatusCount = newCount; }
        void setMagneticStatusCount(float newCount) { magneticStatusCount = newCount; }
        void setRadiationStatusCount(float newCount) { radiationStatusCount = newCount; }
        void setViralStatusCount(float newCount) { viralStatusCount = newCount; }
        void setTauStatusCount(float newCount) { tauStatusCount = newCount; }
        void setHeatInheritDamage(float newDamage) { heatInheritDamage = newDamage; }
};