#pragma once
#include "Enemy.hpp"

float Enemy::getArmor(bool heatArmorStripYN)
{
    this->currentArmor = this->baseArmor;
    if (heatArmorStripYN && this->heatStatusCount > 0)
    {
        this->currentArmor = (this->currentArmor * (1 / 2));
    }
    if (this->corrosiveStatusCount > 0)
    {
        this->currentArmor = (this->currentArmor * (1 - (0.2 + (0.06 * this->corrosiveStatusCount))));
    }
    return this->currentArmor;
}


float Enemy::getAddedCritChance()
{
    this->addedCritChance = (this->punctureStatusCount * 0.05);
    return this->addedCritChance;
}
float Enemy::getAddedCritDamage()
{
    this->addedCritDamage = (0.1 + (0.05 * (this->coldStatusCount - 1)));
    return this->addedCritDamage;
}
float Enemy::getMultiplierToHealthDamage()
{
    this->multiplierToHealthDamage = (2 + (0.25 * (this->viralStatusCount - 1)));
    return this->multiplierToHealthDamage;
}
float Enemy::getAddedStatusVulnerability()
{
    this->addedStatusVulnerability = (0.1 * this->tauStatusCount);
    return this->addedStatusVulnerability;
}

