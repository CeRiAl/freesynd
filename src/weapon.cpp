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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "app.h"
#include "utils/log.h"
#include "weapon.h"
#include "ped.h"
#include "mission.h"
#include "model/shot.h"
#include "core/gamesession.h"

#define Z_SHIFT_TO_AIR   4

uint16 WeaponInstance::weaponIdCnt = 0;

Weapon::Weapon(WeaponType w_type, ConfigFile &conf)
{
    type_ = w_type;
    submittedToSearch_ = false;

    switch(w_type) {
        case Weapon::Pistol:
            idx_ = Weapon::Pistol_Anim;
            sample_ = snd::PISTOL;
            dmg_type_ = MapObject::dmg_Bullet;
            shot_property_ = Weapon::wspt_Pistol;
            impactAnims_.groundHit = SFXObject::sfxt_BulletHit;
            impactAnims_.objectHit = SFXObject::sfxt_BulletHit;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        case Weapon::Minigun:
            idx_ = Weapon::Minigun_Anim;
            sample_ = snd::MINIGUN;
            dmg_type_ = MapObject::dmg_Bullet;
            shot_property_ = Weapon::wspt_Minigun;
            impactAnims_.groundHit = SFXObject::sfxt_BulletHit;
            impactAnims_.objectHit = SFXObject::sfxt_BulletHit;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        case Weapon::Flamer:
            idx_ = Weapon::Flamer_Anim;
            sample_ = snd::FLAME;
            dmg_type_ = MapObject::dmg_Burn;
            shot_property_ = Weapon::wspt_Flamer;
            impactAnims_.groundHit = SFXObject::sfxt_FlamerFire;
            impactAnims_.objectHit = SFXObject::sfxt_FlamerFire;
            impactAnims_.trace_anim = SFXObject::sfxt_FlamerFire;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        case Weapon::LongRange:
            idx_ = Weapon::LongRange_Anim;
            sample_ = snd::LONGRANGE;
            dmg_type_ = MapObject::dmg_Bullet;
            shot_property_ = Weapon::wspt_LongRange;
            impactAnims_.groundHit = SFXObject::sfxt_BulletHit;
            impactAnims_.objectHit = SFXObject::sfxt_BulletHit;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        case Weapon::EnergyShield:
            idx_ = Weapon::EnergyShield_Anim;
            sample_ = snd::NO_SOUND;
            dmg_type_ = MapObject::dmg_None;
            shot_property_ = Weapon::wspt_EnergyShield;
            impactAnims_.groundHit = SFXObject::sfxt_Unknown;
            impactAnims_.objectHit = SFXObject::sfxt_Unknown;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        case Weapon::Uzi:
            idx_ = Weapon::Uzi_Anim;
            sample_ = snd::UZI;
            dmg_type_ = MapObject::dmg_Bullet;
            shot_property_ = Weapon::wspt_Uzi;
            impactAnims_.groundHit = SFXObject::sfxt_BulletHit;
            impactAnims_.objectHit = SFXObject::sfxt_BulletHit;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        case Weapon::Laser:
            idx_ = Weapon::Laser_Anim;
            sample_ = snd::LASER;
            dmg_type_ = MapObject::dmg_Laser;
            shot_property_ = Weapon::wspt_Laser;
            impactAnims_.groundHit = SFXObject::sfxt_Fire_LongSmoke;
            impactAnims_.objectHit = SFXObject::sfxt_Unknown;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        case Weapon::GaussGun:
            idx_ = Weapon::Gauss_Anim;
            sample_ = snd::GAUSSGUN;
            dmg_type_ = MapObject::dmg_Explosion;
            shot_property_ = Weapon::wspt_GaussGun;
            impactAnims_.groundHit = SFXObject::sfxt_ExplosionFire;
            impactAnims_.objectHit = SFXObject::sfxt_ExplosionBall;
            impactAnims_.trace_anim = SFXObject::sfxt_Smoke;
            impactAnims_.rd_anim = SFXObject::sfxt_LargeFire;
            break;
        case Weapon::Shotgun:
            idx_ = Weapon::Shotgun_Anim;
            sample_ = snd::SHOTGUN;
            dmg_type_ = MapObject::dmg_Bullet;
            shot_property_ = Weapon::wspt_Shotgun;
            impactAnims_.groundHit = SFXObject::sfxt_BulletHit;
            impactAnims_.objectHit = SFXObject::sfxt_BulletHit;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        case Weapon::MediKit:
            idx_ = Weapon::Unarmed_Anim;
            sample_ = snd::NO_SOUND;
            dmg_type_ = MapObject::dmg_Heal;
            shot_property_ = Weapon::wspt_MediKit;
            impactAnims_.groundHit = SFXObject::sfxt_Unknown;
            impactAnims_.objectHit = SFXObject::sfxt_Unknown;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        case Weapon::Scanner:
            idx_ = Weapon::Unarmed_Anim;
            sample_ = snd::NO_SOUND;
            dmg_type_ = MapObject::dmg_None;
            shot_property_ = Weapon::wspt_Scanner;
            impactAnims_.groundHit = SFXObject::sfxt_Unknown;
            impactAnims_.objectHit = SFXObject::sfxt_Unknown;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        case Weapon::AccessCard:
            idx_ = Weapon::Unarmed_Anim;
            sample_ = snd::NO_SOUND;
            dmg_type_ = MapObject::dmg_None;
            shot_property_ = Weapon::wspt_AccessCard;
            impactAnims_.groundHit = SFXObject::sfxt_Unknown;
            impactAnims_.objectHit = SFXObject::sfxt_Unknown;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        case Weapon::TimeBomb:
            idx_ = Weapon::Unarmed_Anim;
            sample_ = snd::EXPLOSION;
            dmg_type_ = MapObject::dmg_Explosion;
            shot_property_ = Weapon::wspt_TimeBomb;
            impactAnims_.groundHit = SFXObject::sfxt_ExplosionFire;
            impactAnims_.objectHit = SFXObject::sfxt_ExplosionBall;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_ExplosionFire;
            break;
        case Weapon::Persuadatron:
            idx_ = Weapon::Unarmed_Anim;
            sample_ = snd::PERSUADE;
            dmg_type_ = MapObject::dmg_Persuasion;
            shot_property_ = Weapon::wspt_Persuadatron;
            impactAnims_.groundHit = SFXObject::sfxt_Unknown;
            impactAnims_.objectHit = SFXObject::sfxt_Unknown;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        default:
#if _DEBUG
            printf("unknown weapon loaded(%i), NULL passed", w_type);
#endif
            break;
    }

    // initialize other properties
    initFromConfig(w_type, conf);
}

void Weapon::initFromConfig(WeaponType w_type, ConfigFile &conf) {
    const char *pattern = "weapon.%d.%s";
    char propName[25];

    try {
        sprintf(propName, pattern, w_type, "name");
        name_ = g_Ctx.getMessage(conf.read<std::string>(propName));

        sprintf(propName, pattern, w_type, "icon.small");
        small_icon_ = conf.read<int>(propName);
        sprintf(propName, pattern, w_type, "icon.big");
        big_icon_ = conf.read<int>(propName);
        sprintf(propName, pattern, w_type, "cost");
        cost_ = conf.read<int>(propName);
        sprintf(propName, pattern, w_type, "ammo.nb");
        ammo_ = conf.read<int>(propName);
        sprintf(propName, pattern, w_type, "ammo.price");
        ammo_cost_ = conf.read<int>(propName);
        sprintf(propName, pattern, w_type, "range");
        range_ = conf.read<int>(propName);
        sprintf(propName, pattern, w_type, "rank");
        rank_ = conf.read<int>(propName);
        sprintf(propName, pattern, w_type, "anim");
        anim_ = conf.read<int>(propName);
        sprintf(propName, pattern, w_type, "ammopershot");
        ammo_per_shot_ = conf.read<int>(propName);
        sprintf(propName, pattern, w_type, "timeforshot");
        time_for_shot_ = conf.read<int>(propName);
        sprintf(propName, pattern, w_type, "timereload");
        time_reload_ = conf.read<int>(propName);
        sprintf(propName, pattern, w_type, "damagerange");
        range_dmg_ = conf.read<int>(propName);
        sprintf(propName, pattern, w_type, "shotangle");
        shot_angle_ = conf.read<double>(propName);
        sprintf(propName, pattern, w_type, "shotaccuracy");
        shot_accuracy_ = conf.read<double>(propName);
        sprintf(propName, pattern, w_type, "shotspeed");
        shot_speed_ = conf.read<int>(propName);
        sprintf(propName, pattern, w_type, "dmg_per_shot");
        dmg_per_shot_ = conf.read<int>(propName);
        sprintf(propName, pattern, w_type, "shots_per_ammo");
        shots_per_ammo_ = conf.read<int>(propName);
        sprintf(propName, pattern, w_type, "weight");
        weight_ = conf.read<int>(propName);
        sprintf(propName, pattern, w_type, "auto.fire_rate");
        fireRate_ = conf.read<int>(propName, 0);
    } catch (...) {
        FSERR(Log::k_FLG_GAME, "Weapon", "initFromConfig", ("Cannot load weapon %d : %s\n", w_type, propName))
    }
}

/*!
 * Creates a new instance of Weapon instance for the given weapon class.
 * \param pWeaponClass Class of weapon
 * \return an instance of WeaponInstance
 */
WeaponInstance *WeaponInstance::createInstance(Weapon *pWeaponClass) {
    return new WeaponInstance(pWeaponClass, weaponIdCnt++);
}

WeaponInstance::WeaponInstance(Weapon * w, uint16 anId) : ShootableMapObject(anId, -1, MapObject::kNatureWeapon),
    bombSoundTimer(w->timeReload()), bombExplosionTimer(w->timeForShot()),
    flamerTimer_(180) {
    pWeaponClass_ = w;
    ammo_remaining_ = w->ammo();
    weapon_used_time_ = 0;
    pOwner_ = NULL;
    activated_ = false;
    time_consumed_ = false;
    if (w->getWeaponType() == Weapon::TimeBomb
        || w->getWeaponType() == Weapon::Flamer)
    {
        size_x_ = 32;
        size_y_ = 32;
        size_z_ = 32;
    }
    health_ = 1;
    start_health_ = 1;
    pFlamerShot_ = NULL;
}

bool WeaponInstance::animate(int elapsed) {

    if (activated_) {
        if (getWeaponType() == Weapon::EnergyShield) {
            if (ammo_remaining_ == 0)
                return false;
            int tm_left = elapsed;
            int ammoused = getShots(&tm_left) * pWeaponClass_->ammoPerShot();
            if (ammoused >= ammo_remaining_) {
                ammo_remaining_ = 0;
                pOwner_->selectNextWeapon();
            } else
                ammo_remaining_ -= ammoused;
            return true;
        } else if (getWeaponType() == Weapon::TimeBomb) {
            if (bombSoundTimer.update(elapsed)) {
                g_App.gameSounds().play(snd::TIMEBOMB);
            }

            if (bombExplosionTimer.update(elapsed)) {
                ShootableMapObject::DamageInflictType dmg;
                fire(g_Session.getMission(), dmg, elapsed);
                return true;
            }
            time_consumed_ = true;
        }

        updtWeaponUsedTime(elapsed);
    } else if (weapon_used_time_ != 0) {
        updtWeaponUsedTime(elapsed);
    }

    if (map_ == -1)
        return false;

    return MapObject::animate(elapsed);
}

void WeaponInstance::draw(int x, int y) {
    addOffs(x, y);
    g_App.gameSprites().drawFrame(pWeaponClass_->anim(), frame_, x, y);
}

/*!
 * Plays the sound associated with that weapon.
 */
void WeaponInstance::playSound() {
    g_App.gameSounds().play(pWeaponClass_->getSound());
}

int WeaponInstance::getShots(int *elapsed, uint32 make_shots) {
    int time_for_shot = pWeaponClass_->timeForShot();
    int time_reload = pWeaponClass_->timeReload();
#if 0
    // TODO: if owner exists these two values should change(IPA, mods)
    if (pOwner_)
#endif
    // TODO check in weaponinstance animate double consuming of elapsed
    int time_full_shot = time_for_shot + time_reload;
    int elapsed_l = *elapsed;
    *elapsed = 0;
    time_consumed_ = true;
    if (weapon_used_time_ >= time_for_shot) {
        weapon_used_time_ += elapsed_l;
        if (weapon_used_time_ >= time_full_shot) {
            // reloading after previous shot
            weapon_used_time_ -= time_full_shot;
        } else {
            // reload consumed all time, no time for shooting
            return 0;
        }
    } else
        weapon_used_time_ += elapsed_l;

    if (weapon_used_time_ == 0) {
        return 0;
    }
    elapsed_l = 0;

    uint32 shots_can_do = 0xFFFFFFFF;
    if (shotProperty() & Weapon::spe_UsesAmmo) {
        shots_can_do = ammo_remaining_ / pWeaponClass_->ammoPerShot();
        if (ammo_remaining_ % pWeaponClass_->ammoPerShot())
            shots_can_do++;
    }
    uint32 shots = weapon_used_time_ / time_full_shot;
    weapon_used_time_ %= time_full_shot;
    bool adjusted = false;
    if (weapon_used_time_ >= time_for_shot) {
        shots++;
        adjusted = true;
    }
    // Adjusting time consumed and shots done to ammo
    // that can be used
    if (shots_can_do < shots) {
        if (adjusted) {
            shots--;
            adjusted = false;
        }
        if (shots_can_do < shots) {
         elapsed_l = time_full_shot * (shots - shots_can_do);
         shots = shots_can_do;
        } else
            elapsed_l = weapon_used_time_;
        weapon_used_time_ = 0;
    }

    if (make_shots != 0 && shots != 0 && make_shots < shots) {
        // we might have some time left here
        if (adjusted)
            shots--;
        if (make_shots < shots) {
         *elapsed = time_full_shot  * (shots - make_shots) + elapsed_l;
         *elapsed += weapon_used_time_;
         shots = make_shots;
        } else
            *elapsed = elapsed_l + weapon_used_time_;
        weapon_used_time_ = 0;
    } else
        *elapsed = elapsed_l;
    return shots;
}

void WeaponInstance::activate() {
    activated_ = true;
}

void WeaponInstance::deactivate() {
    activated_ = false;
    if (getWeaponType() == Weapon::TimeBomb)
        weapon_used_time_ = 0;
}

/*!
 * Use weapon and decrease ammo.
 * This method is used only for shooting weapons and Medikit.
 * \param pMission Mission data
 * \param dmg Information on the damage to perform
 * \param elapsed Time since last frame
 */
void WeaponInstance::fire(Mission *pMission, ShootableMapObject::DamageInflictType &dmg, int elapsed) {
    bool updateStats = true;
    if (getWeaponType() == Weapon::MediKit) {
        dmg.d_owner->resetHealth();
        updateStats = false;
    } else if (getWeaponType() == Weapon::GaussGun) {
        GaussGunShot *pShot = new GaussGunShot(dmg);
        pMission->addPrjShot(pShot);
    } else if (getWeaponType() == Weapon::Flamer) {
        // when targeting a point with the flamer, the point of impact
        // circles around the target.
        // We use the weapon's direction field to store a logical direction
        // which will give the current moving point of impact
        switch(direction()) {
        case 0:
            dmg.aimedLocW.y += 160;
            break;
        case 1:
            dmg.aimedLocW.x += 96;
            dmg.aimedLocW.y += 96;
            break;
        case 2:
            dmg.aimedLocW.x += 160;
            break;
        case 3:
            dmg.aimedLocW.x += 96;
            dmg.aimedLocW.y -= 96;
            break;
        case 4:
            dmg.aimedLocW.y -= 160;
            break;
        case 5:
            dmg.aimedLocW.x -= 96;
            dmg.aimedLocW.y -= 96;
            break;
        case 6:
            dmg.aimedLocW.x -= 160;
            break;
        case 7:
            dmg.aimedLocW.x -= 96;
            dmg.aimedLocW.y += 96;
            break;
        }

        FlamerShot *pFlamerShot = new FlamerShot(pMission, dmg);
        pMission->addPrjShot(pFlamerShot);

        // Change direction for next time
        if (flamerTimer_.update(elapsed)) {
            setDirection((direction() + 1) % 8);
        }
    }  else if (getWeaponType() == Weapon::TimeBomb) {
        updateStats = false;
        map_ = -1;
        health_ = 0;
        deactivate();
        Explosion::createExplosion(pMission, this,
            (double)pWeaponClass_->rangeDmg(), pWeaponClass_->damagePerShot());
    } else {
        // For other weapons, damage are done immediatly because projectile speed
        // is too high to draw them
        InstantImpactShot shot(dmg);
        shot.inflictDamage(pMission);
    }

    int ammoUsed = pWeaponClass_->ammoPerShot();
    ammo_remaining_ -= ammoUsed;
    if (ammo_remaining_ < 0) {
        ammo_remaining_ = 0;
    }

    if (updateStats) {
        if (pOwner_ && pOwner_->isOurAgent()) {
            pMission->stats()->incrShots(ammoUsed);
        }
    }
}

void WeaponInstance::handleHit(ShootableMapObject::DamageInflictType & d)
{
    // When a bomb is hit, it explodes
    if (getWeaponType() == Weapon::TimeBomb && health_ > 0) {
        // we pass the given DamageInflictType just for the compiler
        // as it is not used by the fire method for a Bomb
        // same for elapsed
        fire(g_Session.getMission(), d, 0);
    }
}

void WeaponInstance::updtWeaponUsedTime(int elapsed) {
    if (time_consumed_) {
        time_consumed_ = false;
    } else if (weapon_used_time_ != 0 ) {
        weapon_used_time_ += elapsed;
        if (weapon_used_time_ >= (pWeaponClass_->timeForShot()
            + pWeaponClass_->timeReload()))
        {
            weapon_used_time_ = 0;
        }
    }
}
