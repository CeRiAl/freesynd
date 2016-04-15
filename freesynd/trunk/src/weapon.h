/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
 *   Copyright (C) 2011  Mark <mentor66@users.sourceforge.net>          *
 *                                                                      *
 *    This program is free software;  you can redistribute it and / or  *
 *  modify it  under the  terms of the  GNU General  Public License as  *
 *  published by the Free Software Foundation; either version 2 of the  *
 *  License, or (at your option) any later version.                     *
 *                                                                      *
 *    This program is  distributed in the hope that it will be useful,  *
 *  but WITHOUT  ANY WARRANTY;  without even  the implied  warranty of  *
 *  MERCHANTABILITY  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 *  General Public License for more details.                            *
 *                                                                      *
 *    You can view the GNU  General Public License, online, at the GNU  *
 *  project's  web  site;  see <http://www.gnu.org/licenses/gpl.html>.  *
 *  The full text of the license is also included in the file COPYING.  *
 *                                                                      *
 ************************************************************************/

#ifndef WEAPON_H
#define WEAPON_H

#include <string>
#include <vector>
#include "mapobject.h"
#include "sound/sound.h"
#include "utils/configfile.h"
#include "utils/timer.h"

class FlamerShot;
class PedInstance;

/*!
 * Weapon class.
 */
class Weapon {
public:
    enum WeaponType {
        Unknown = 0,
        Pistol = 1,
        GaussGun = 2,
        Shotgun = 3,
        Uzi = 4,
        Minigun = 5,
        Laser = 6,
        Flamer = 7,
        LongRange = 8,
        Scanner = 9,
        MediKit = 10,
        TimeBomb = 11,
        AccessCard = 12,
        EnergyShield = 13,
        Persuadatron = 14
    };

    typedef enum {
        Unarmed_Anim,
        Pistol_Anim,
        Minigun_Anim,
        Flamer_Anim,
        LongRange_Anim,
        EnergyShield_Anim,
        Uzi_Anim,
        Laser_Anim,
        Gauss_Anim,
        Shotgun_Anim
    } WeaponAnimIndex;

    /*!
     * This structure holds animation ids of impacts for a weapon.
     */
    struct ImpactAnims {
        //! Animation played when impact is on the ground.
        SFXObject::SfxTypeEnum groundHit;
        //! Animation played when impact is on a living object.
        SFXObject::SfxTypeEnum objectHit;
        SFXObject::SfxTypeEnum trace_anim;
        /*! if weapon can do range damage this is used for range definition
         * with animation.*/
        int rd_anim;
    };

    Weapon(WeaponType w_type, ConfigFile &conf);

    const char *getName() { return name_.c_str(); }

    int cost() { return cost_; }
    int ammo() { return ammo_; }
    int range() { return range_; }
    int damagePerShot() { return dmg_per_shot_; }
    int rank() { return rank_; }
    int anim() { return anim_; }
    int weight() {return weight_; }
    snd::InGameSample getSound() { return sample_; }
    int getSmallIconId() { return small_icon_; }
    int getBigIconId() { return big_icon_; }

    int selector() {
        return small_icon_ == 28 ? 1618 : small_icon_ - 14 + 1602;
    }

    WeaponAnimIndex index() { return idx_; }
    WeaponType getWeaponType() { return type_; }
    MapObject::DamageType dmgType() { return dmg_type_; }

    bool operator==(Weapon weapon) { return this->type_ == weapon.getWeaponType(); }

    int ammoPerShot() { return ammo_per_shot_; }
    int timeForShot() { return time_for_shot_; }
    int timeReload() { return time_reload_; }

    bool wasSubmittedToSearch() { return submittedToSearch_; }
    void submitToSearch() { submittedToSearch_ = true; }
    void resetSubmittedToSearch() { submittedToSearch_ = false; }

    bool canShoot() {
        return (shot_property_ & Weapon::spe_CanShoot) != 0;
    }

    bool doesPhysicalDmg() {
        return (dmg_type_ & MapObject::dmg_Physical) != 0;
    }

    typedef enum {
        spe_None = 0x0,
        // can target only owner
        spe_Owner = 0x0001,

        spe_PointToPoint = 0x0002,
        spe_PointToManyPoints = 0x0004,

        spe_TargetReachInstant = 0x0008,

        spe_TargetReachNeedTime = 0x0010,
        spe_CreatesProjectile = 0x0010,

        spe_RangeDamageOnReach = 0x0020,
        // ignore accuracy
        spe_ShootsWhileNoTarget = 0x0040,
        spe_UsesAmmo = 0x0080,
        spe_ChangeAttribute = 0x0100,
        spe_SelfDestruction = 0x0200,
        spe_TargetPedOnly = 0x0400,
        spe_CanShoot = 0x0800,
        //! Automatic weapon can shot continuously
        spe_Automatic = 0X1000
    } ShotPropertyEnum;

    typedef enum {
        wspt_None = spe_None,
        wspt_Persuadatron = spe_None,
        wspt_Pistol =
            (spe_PointToPoint | spe_TargetReachInstant | spe_UsesAmmo
            | spe_CanShoot),
        wspt_GaussGun =
            (spe_PointToPoint | spe_TargetReachNeedTime | spe_UsesAmmo
            | spe_RangeDamageOnReach | spe_CanShoot),
        wspt_Shotgun =
            (spe_PointToManyPoints | spe_TargetReachInstant | spe_UsesAmmo
            | spe_CanShoot),
        wspt_Uzi = (spe_PointToPoint | spe_TargetReachInstant
            | spe_UsesAmmo | spe_CanShoot | spe_Automatic),
        wspt_Minigun =
            (spe_PointToManyPoints | spe_TargetReachInstant | spe_UsesAmmo
            | spe_CanShoot | spe_Automatic),
        wspt_Laser =
            (spe_PointToPoint | spe_TargetReachInstant
            | spe_RangeDamageOnReach | spe_UsesAmmo | spe_CanShoot),
        wspt_Flamer =
            (spe_PointToPoint | spe_TargetReachNeedTime | spe_UsesAmmo
            | spe_CanShoot | spe_Automatic),
        wspt_LongRange =
            (spe_PointToPoint | spe_TargetReachInstant | spe_UsesAmmo
            | spe_CanShoot),
        wspt_Scanner = (spe_Owner | spe_ChangeAttribute),
        wspt_MediKit = (spe_Owner | spe_UsesAmmo),
        wspt_TimeBomb = (spe_ShootsWhileNoTarget | spe_TargetReachInstant
            | spe_RangeDamageOnReach | spe_SelfDestruction),
        wspt_AccessCard = (spe_Owner | spe_ChangeAttribute),
        wspt_EnergyShield =
            (spe_Owner | spe_ChangeAttribute | spe_UsesAmmo),
    } WeaponShotPropertyType;

    enum SearchTargetMask {
        stm_AllObjects = MapObject::kNaturePed | MapObject::kNatureVehicle
        | MapObject::kNatureStatic | MapObject::kNatureWeapon
    };

    // (WeaponShotPropertyType)
    unsigned int shotProperty() { return shot_property_; }

    ImpactAnims * impactAnims() { return &impactAnims_; }
    int rangeDmg() { return range_dmg_; }
    double shotAngle() { return shot_angle_; }
    double shotAcurracy() { return shot_accuracy_; }
    int ammoCost() { return ammo_cost_; }
    int shotSpeed() { return shot_speed_; }
    int shotsPerAmmo() { return shots_per_ammo_; }
    //! Returns the fire rate expressed in time between two shots
    int fireRate() { return fireRate_; }
    bool usesAmmo() {
        return (shotProperty() & Weapon::spe_UsesAmmo) != 0;
    }
    /*!
     * Return true if weapon is automatic.
     * With automatic weapon, player can keep mouse clicked
     * to shoot continuously.
     */
    bool isAutomatic() {
        return (shotProperty() & Weapon::spe_Automatic) != 0;
    }

protected:
    //! Init weapon from given config file
    void initFromConfig(WeaponType w_type, ConfigFile &conf);

protected:
    std::string name_;
    int small_icon_, big_icon_;
    /*! The price of this weapon.*/
    int cost_;
    /*! The price to reload the weapon.*/
    int ammo_cost_;
    int ammo_, range_, dmg_per_shot_;
    int rank_;  //!> weapon rank
    WeaponType type_;
    MapObject::DamageType dmg_type_;
    int ammo_per_shot_;
    //! time weapon uses to do a single shot
    int time_for_shot_;
    //! time required to make weapon ready to shoot
    int time_reload_;
    /*! True when weapon was found and submit to search manager.*/
    bool submittedToSearch_;
    //WeaponShotPropertyType
    uint32 shot_property_;
    int range_dmg_;
    //! some weapons have wider shot
    double shot_angle_;
    //! agent accuracy will be applied to this, later to shot_angle_
    double shot_accuracy_;
    //! only projectiles have this set (gauss, flamer)
    int shot_speed_;
    //! shotgun only for now
    int shots_per_ammo_;
    //! The weight of a weapon influences the agent's speed
    int weight_;
    //! The fire rate expressed by time between two shots. Only for automatic weapons
    int fireRate_;
    //!
    WeaponAnimIndex idx_;
    int anim_;
    //! Set of ids of impacts animation
    ImpactAnims impactAnims_;
    //! Sound of weapon
    snd::InGameSample sample_;
};

/*!
 * Weapon instance class.
 */
class WeaponInstance : public ShootableMapObject {
public:
    //! Creates a instance for the given weapon class
    static WeaponInstance *createInstance(Weapon *pWeaponClass);

    WeaponInstance(Weapon *w, uint16 id);

    /*! Sets the owner of the weapon. */
    void setOwner(PedInstance *pOwner) { pOwner_ = pOwner; }
    /*! Return the owner of the weapon.*/
    PedInstance *owner() { return pOwner_; }
    /*! Return true if the weapon has an owner.*/
    bool hasOwner() { return pOwner_ != NULL; }

    int ammoRemaining() { return ammo_remaining_; }
    void setAmmoRemaining(int n) {  ammo_remaining_ = n; }

    bool animate(int elapsed);
    void draw(int x, int y);

    Weapon *getWeaponClass() { return pWeaponClass_; }

    int range() { return pWeaponClass_->range(); }
    int ammo() { return pWeaponClass_->ammo(); }
    int rank() { return pWeaponClass_->rank(); }
    uint32 shotProperty() { return pWeaponClass_->shotProperty(); }
    const char * name() { return pWeaponClass_->getName(); }

    Weapon::WeaponAnimIndex index() { return pWeaponClass_->index(); }
    Weapon::WeaponType getWeaponType() { return pWeaponClass_->getWeaponType(); }

    bool operator==(WeaponInstance wi) {
        return getWeaponType() == wi.getWeaponType();
    }

    //! Plays the weapon's sound.
    void playSound();

    void resetWeaponUsedTime() { weapon_used_time_ = 0; }

    int getShots(int *elapsed = NULL, uint32 make_shots = 0);

    void activate();
    void deactivate();

    void handleHit(ShootableMapObject::DamageInflictType & d);

    bool canShoot() {
        return pWeaponClass_->canShoot();
    }

    bool doesPhysicalDmg() {
        return pWeaponClass_->doesPhysicalDmg();
    }

    bool needsReloading() {
        return pWeaponClass_->ammo() > ammo_remaining_;
    }

    bool usesAmmo() {
        return (shotProperty() & Weapon::spe_UsesAmmo) != 0;
    }

    bool doesDmgStrict(uint32 dmg_type) {
        return pWeaponClass_->dmgType() == dmg_type;
    }
    bool doesDmgNonStrict(uint32 dmg_type) {
        return (pWeaponClass_->dmgType() & dmg_type) != 0;
    }
    MapObject::DamageType dmgType() {
        return pWeaponClass_->dmgType();
    }
    int getWeight() { return pWeaponClass_->weight(); }
    void updtWeaponUsedTime(int elapsed);

    //! Use weapon
    void fire(Mission *pMission, ShootableMapObject::DamageInflictType &dmg, int elapsed);

protected:
    static uint16 weaponIdCnt;
    Weapon *pWeaponClass_;
    /*! Owner of the weapon.*/
    PedInstance *pOwner_;
    int ammo_remaining_;
    /*! if this value is smaller time_for_shot_ shot cannot be done
    * if is greater then time_for_shot_ reload is in execution
    * if is greater then time_for_shot_ + time_reload_ then full shot is done
    * */
    int weapon_used_time_;
    // used for timebomb sound effect
    fs_utils::Timer bombSoundTimer;
    /*! Timer used for bomb explosion.*/
    fs_utils::Timer bombExplosionTimer;
    /*! Timer used for rotating flamer direction.*/
    fs_utils::Timer flamerTimer_;
    bool activated_;
    /*! used to avoid double consuming of same elapsed time,
    * if ped shoots, time is consumed and should not be reused by weapon,
    * NOTE: ped animate executed before weapon animate
    */
    bool time_consumed_;
    FlamerShot *pFlamerShot_;
};

#endif
