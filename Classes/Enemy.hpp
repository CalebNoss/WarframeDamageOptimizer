#pragma once
#include <string>

class Enemy
{
    private:
        double baseArmor = 2700;
        double currentArmor = 2700; // armor taking into account heat & corrosive stacks

        double heatInheritDamage = 0.0f;

        double addedCritChance = 0.0f; // added crit chance from puncture stacks
        double addedCritDamage = 0.0f; // added crit damage from cold stacks
        double multiplierToHealthDamage = 0.0f; // multiplier to health damage from viral stacks
        double addedStatusVulnerability = 0.0f; // added status chance from tau stacks



        std::array<double, 14> statusCounts = {0};
        //  0   = "Impact"
        //  1   = "Puncture"
        //  2   = "Slash"
        //  3   = "Heat"
        //  4   = "Cold"
        //  5   = "Electricity"
        //  6   = "Toxin"
        //  7   = "Blast"
        //  8   = "Corrosive"
        //  9  = "Gas"
        //  10  = "Magnetic"
        //  11  = "Radiation"
        //  12  = "Viral"
        //  13  = "Tau"
    

    public:
        // Having these be in a separate implementation file was causing weird errors, so they are defined here instead.

        // specific getters
        double getArmor(bool heatArmorStrip) {
            this->currentArmor = this->baseArmor;
            if (heatArmorStrip && this->statusCounts.at(3) > 0)
            {
                this->currentArmor = (this->currentArmor * (1 / 2));
            }
            if (this->statusCounts.at(8) > 0)
            {
                this->currentArmor = (this->currentArmor * (1 - (0.2 + (0.06 * this->statusCounts.at(8)))));  // this should already account or under 1 avg corrosive proc/second
            }
            return this->currentArmor;
        };
        double getAddedCritChance() {
            this->addedCritChance = (this->statusCounts.at(1) * 0.05);
            return this->addedCritChance;
        };
        double getAddedCritDamage() {
            if (this->statusCounts.at(4) >=1) // if more than 1 cold proc, proceed normally
            {
                this->addedCritDamage = (0.1 + (0.05 * (this->statusCounts.at(4) - 1)));
            }
            else if (this->statusCounts.at(4) > 0) // if less than 1 but more than 0 avg cold procs, use as multiplier of first cold status effect and ignore the rest
            {
                this->addedCritDamage = 0.1 * this->statusCounts.at(4);
            }
            return this->addedCritDamage;
        };
        double getMultiplierToHealthDamage() {
            if (this->statusCounts.at(12) >= 1) // if more than 1 viral proc, proceed as normal
            {
                this->multiplierToHealthDamage = (2 + (0.25 * (this->statusCounts.at(12) - 1)));
            }
            else if (this->statusCounts.at(12) > 0) // if less than 1 but more than 0 avg cold procs, use as multiplier of first viral status effect and ignore the rest
            {
                this->multiplierToHealthDamage = 2 * this->statusCounts.at(12);
            }
            return this->multiplierToHealthDamage;
        };
        double getAddedStatusVulnerability() {
            this->addedStatusVulnerability = (0.1 * this->statusCounts.at(13));
            return this->addedStatusVulnerability;
        };

        // general getters
        double getHeatInheritDamage() const { return heatInheritDamage; }
        std::array<double, 14>* getStatusCounts() { return &statusCounts; }

        // general setters
        void setHeatInheritDamage(double newDamage) { heatInheritDamage = newDamage; }
        void setStatusCounts(std::array<double, 14> newStatusCounts) { statusCounts = newStatusCounts;}
};