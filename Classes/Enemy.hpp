#pragma once
#include <unordered_map>
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

        std::unordered_map<std::string, int> statusCaps = {
            {"Impact", 5},
            {"Puncture", 5},
            {"Slash", 1061109567},
            {"Heat", 1061109567},
            {"Cold", 10},
            {"Electricity", 1061109567},
            {"Toxin", 1061109567},
            {"Blast", 10},
            {"Corrosive", 10},
            {"Gas", 10},
            {"Magnetic", 10},
            {"Radiation", 10},
            {"Viral", 10},
            {"Tau", 10}
        };

        std::unordered_map<std::string, double> statusDurations = {
            {"Impact", 6},
            {"Puncture", 10},
            {"Slash", 6},
            {"Heat", 6},
            {"Cold", 6},
            {"Electricity", 6},
            {"Toxin", 6},
            {"Blast", 1.5},
            {"Corrosive", 8},
            {"Gas", 6},
            {"Magnetic", 6},
            {"Radiation", 12},
            {"Viral", 6},
            {"Tau", 8}
        };

        std::unordered_map<std::string, double> statusCounts = {
            {"Impact", 0},
            {"Puncture", 0},
            {"Slash", 0},
            {"Heat", 0},
            {"Cold", 0},
            {"Electricity", 0},
            {"Toxin", 0},
            {"Blast", 0},
            {"Corrosive", 0},
            {"Gas", 0},
            {"Magnetic", 0},
            {"Radiation", 0},
            {"Viral", 0},
            {"Tau", 0}
        };

    public:
        // Having these be in a separate implementation file was causing weird errors, so they are defined here instead.

        // specific getters
        double getArmor(bool heatArmorStrip) {
            this->currentArmor = this->baseArmor;
            if (heatArmorStrip && this->statusCounts["Heat"] > 0)
            {
                this->currentArmor = (this->currentArmor * (1 / 2));
            }
            if (this->statusCounts["Corrosive"] > 0)
            {
                this->currentArmor = (this->currentArmor * (1 - (0.2 + (0.06 * this->statusCounts["Corrosive"]))));  // this should already account or under 1 avg corrosive proc/second
            }
            return this->currentArmor;
        };
        double getAddedCritChance() {
            this->addedCritChance = (this->statusCounts["Puncture"] * 0.05);
            return this->addedCritChance;
        };
        double getAddedCritDamage() {
            if (this->statusCounts["Cold"] >=1) // if more than 1 cold proc, proceed normally
            {
                this->addedCritDamage = (0.1 + (0.05 * (this->statusCounts["Cold"] - 1)));
            }
            else if (this->statusCounts["Cold"] > 0) // if less than 1 but more than 0 avg cold procs, use as multiplier of first cold status effect and ignore the rest
            {
                this->addedCritDamage = 0.1 * this->statusCounts["Cold"];
            }
            return this->addedCritDamage;
        };
        double getMultiplierToHealthDamage() {
            if (this->statusCounts["Viral"] >= 1) // if more than 1 viral proc, proceed as normal
            {
                this->multiplierToHealthDamage = (2 + (0.25 * (this->statusCounts["Viral"] - 1)));
            }
            else if (this->statusCounts["Viral"] > 0) // if less than 1 but more than 0 avg cold procs, use as multiplier of first viral status effect and ignore the rest
            {
                this->multiplierToHealthDamage = 2 * this->statusCounts["Viral"];
            }
            return this->multiplierToHealthDamage;
        };
        double getAddedStatusVulnerability() {
            this->addedStatusVulnerability = (0.1 * this->statusCounts["Tau"]);
            return this->addedStatusVulnerability;
        };

        // general getters
        double getHeatInheritDamage() const { return heatInheritDamage; }
        const std::unordered_map<std::string, int>& getStatusCaps() const { return statusCaps; }
        const std::unordered_map<std::string, double>& getStatusDurations() const { return statusDurations; }
        std::unordered_map<std::string, double>* getStatusCounts() { return &statusCounts; }

        // general setters
        void setHeatInheritDamage(double newDamage) { heatInheritDamage = newDamage; }
        void setStatusCounts(const std::unordered_map<std::string, double>& newStatusCounts) { statusCounts = newStatusCounts;}
};