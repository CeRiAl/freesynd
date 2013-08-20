/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
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
#include <assert.h>

#include "loadingmenu.h"
#include "gfx/screen.h"
#include "mission.h"
#include "core/gamecontroller.h"
#include "core/gamesession.h"
#include "menus/menumanager.h"
#include "menus/gamemenuid.h"

LoadingMenu::LoadingMenu(MenuManager * m):Menu(m, fs_game_menus::kMenuIdLoading, fs_game_menus::kMenuIdMain),
    timer_(2000)
{
    isCachable_ = false;
    do_load_ = true;
    addStatic(0, 180, g_Screen.gameScreenWidth(), "#LDGAME_TITLE", FontManager::SIZE_4, true);
}

void LoadingMenu::handleTick(int elapsed)
{
    if (do_load_) {
        // Loads mission
        int id = g_Session.getSelectedBlock().mis_id;
        Mission *pMission = g_gameCtrl.missions().loadMission(id);
        assert(pMission != NULL);
        pMission->setSurfaces();
        g_Session.setMission(pMission);

        do_load_ = false;
    }

    if (timer_.update(elapsed)) {
        menu_manager_->gotoMenu(fs_game_menus::kMenuIdGameplay);
    }
}
