/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
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

#include "menus/menumanager.h"
#include "appcontext.h"
#include "system.h"
#include "utils/configfile.h"
#include "utils/file.h"
#include "utils/log.h"
#include "gfx/fliplayer.h"
#include "gfx/screen.h"
#include "sound/soundmanager.h"

MenuManager::MenuManager(MenuFactory *pFactory, SoundManager *pGameSounds): 
    dirtyList_(g_Screen.gameScreenWidth(), g_Screen.gameScreenHeight()),
    menuSprites_(), fonts_(),
    background_texture_(NULL)
{
    pFactory_ = pFactory;
    pGameSounds_ = pGameSounds;
    pFactory_->setMenuManager(this);
    drop_events_ = false;
    needBackground_ = false;
    
    current_ = NULL;
    nextMenuId_ = -1;

    pIntroFontSprites_ = NULL;

    since_mouse_down_ = 0;
    mouseup_was_ = false;
}

MenuManager::~MenuManager()
{
    delete pFactory_;

    if (background_texture_)
        delete background_texture_;
}

/*!
 * Initialize the menu manager.
 * \param loadIntroFont If true loads the intro sprites and font
 */
bool MenuManager::initialize(bool loadIntroFont) {
    bool res = false;
    int size = 0, tabSize = 0;
    uint8 *data, *tabData;

    // load palette
    setDefaultPalette();

    // Loads menu sprites
    LOG(Log::k_FLG_GFX, "MenuManager", "initialize", ("Loading menu sprites ..."))
    tabData = File::loadOriginalFile("mspr-0.tab", tabSize);
    if (!tabData) {
        FSERR(Log::k_FLG_UI, "MenuManager", "initialize", ("Failed reading file %s", "mspr-0.tab"));
        return false;
    }
    data = File::loadOriginalFile("mspr-0.dat", size);
    if (!data) {
        FSERR(Log::k_FLG_UI, "MenuManager", "initialize", ("Failed reading file %s", "mspr-0.dat"));
        delete[] tabData;
        return false;
    }

    res = menuSprites_.loadSprites(tabData, tabSize, data, true);
    delete[] tabData;
    delete[] data;
    if (res) {
        LOG(Log::k_FLG_GFX, "MenuManager", "initialize", ("%d sprites loaded", tabSize / 6))
    } else {
        FSERR(Log::k_FLG_UI, "MenuManager", "initialize", ("Failed loading menu sprites"));
        return false;
    }

    // loads intro sprites
    if (loadIntroFont) {
        LOG(Log::k_FLG_GFX, "MenuManager", "initialize", ("Loading intro sprites ..."))

        tabData = File::loadOriginalFile("mfnt-0.tab", tabSize);
        if (!tabData) {
            FSERR(Log::k_FLG_UI, "MenuManager", "initialize", ("Failed reading file %s", "mfnt-0.tab"));
            return false;
        }
        data = File::loadOriginalFile("mfnt-0.dat", size);
        if (!data) {
            FSERR(Log::k_FLG_UI, "MenuManager", "initialize", ("Failed reading file %s", "mfnt-0.dat"));
            delete[] tabData;
            return false;
        }

        pIntroFontSprites_ = new SpriteManager();
        res = pIntroFontSprites_->loadSprites(tabData, tabSize, data, true);
        delete[] tabData;
        delete[] data;
        if (res) {
            LOG(Log::k_FLG_GFX, "MenuManager", "initialize", ("%d sprites loaded", tabSize / 6))
        } else {
            FSERR(Log::k_FLG_UI, "MenuManager", "initialize", ("Failed loading intro sprites"));
            return false;
        }
    }

    // Loads fonts
    LOG(Log::k_FLG_GFX, "MenuManager", "initialize", ("Loading fonts ..."))
    res = fonts_.loadFonts(&menuSprites_, pIntroFontSprites_);

    // Initialize background
    background_texture_ = new Texture(Menu::kMenuScreenWidth, Menu::kMenuScreenHeight);

    return res;
}

/*!
 * Destroy all menus and resources.
 */
void MenuManager::destroy() {
    LOG(Log::k_FLG_MEM, "MenuManager", "~MenuManager", ("Destruction..."))

    if (pIntroFontSprites_) {
        delete pIntroFontSprites_;
        pIntroFontSprites_ = NULL;
    }

    // NOTE: this code was before in destructor, but it should be
    // here to avoid corruption, that will happen because
    // agentmanager is destroyed before menumanager and
    // removeModelListener will fail with critical error,
    // for unknown reason this happens only in WindowsOS
    // NOTE: it might be better to create our deallocator
    // of resource in App class to have correct sequence of destructors
    if (current_) {
        if (menus_.find(current_->getId()) == menus_.end()) {
            delete current_;
            current_ = NULL;
        }
    }

    for (std::map<int, Menu*>::iterator it = menus_.begin();
        it != menus_.end(); it++)
        delete it->second;
}

void MenuManager::setDefaultPalette() {
    setPalette("mselect.pal", true);
}

void MenuManager::setPaletteForMission(int i_id) {
    // I'm not sure of the way we get the palette
    char spal[20];
    sprintf(spal,"hpal0%i.dat", i_id % 5 + 1);
    setPalette(spal);
}

void MenuManager::setPalette(const char *fname, bool sixbit) {
    LOG(Log::k_FLG_GFX, "MenuManager", "setPalette", ("Setting palette : %s", fname))
    int size;
    uint8 *data = File::loadOriginalFile(fname, size);

    if (data) {
        if (sixbit)
            g_System.setPalette6b3(data);
        else
            g_System.setPalette8b3(data);

        delete[] data;
    }
}

Menu * MenuManager::getMenu(int menuId) {
    // look in the cache
    if (menus_.find(menuId) != menus_.end()) {
        return menus_[menuId];
    }

    // menu is not in cache so create it
    // some menus are not saved in cache as they are not accessed many times
    Menu *pMenu = pFactory_->createMenu(menuId);

    if (pMenu && pMenu->isCachable()) {
        menus_[menuId] = pMenu;
    }

    return pMenu;
}

/*!
 * Change the current menu with the one with the given name.
 * Plays the transition animations between the two menus.
 */
void MenuManager::changeCurrentMenu()
{
    // Get the next menu
    Menu *pMenu = getMenu(nextMenuId_);
    if (pMenu == NULL) {
        return;
    }

    if (current_) {
        // Give the possibility to the old menu
        // to clean before leaving
        leaveMenu(current_);
        // If menu is not in cache, it means it must be destroyed
        if (menus_.find(current_->getId()) == menus_.end()) {
            delete current_;
            current_ = NULL;
        }
    }
    current_ = pMenu;
    nextMenuId_ = -1;
    showMenu(pMenu);
}

void MenuManager::gotoMenu(int menuId) {
    nextMenuId_ = menuId;
    // stop listening for events until window changed
    drop_events_ = true;
}

/*!
 * Display the opening animation if the flag is true.
  * After having played the animation, renders one time the menu.
 * \param pMenu The menu to show.
 * \param playAnim True if the intro can be played.
 */
void MenuManager::showMenu(Menu *pMenu) {
    g_Screen.setRenderSize(pMenu->width(), pMenu->height());

    if (pMenu->isScreenMode()) {
        g_Screen.leaveWorldMode();
    }

    if (pMenu->hasShowAnim()) {
        // Stop processing event during menu transitions
        drop_events_ = true;
        FliPlayer fliPlayer(this);
        uint8 *data;
        int size;
        data = File::loadOriginalFile(pMenu->getShowAnimName(), size);
        fliPlayer.loadFliData(data);
        fliPlayer.play();
        delete[] data;

    }

    // reset background
    needBackground_ = false;
    dirtyList_.flush();
    pMenu->handleShow();

    // then plot the mouse to draw the button
    // that could be highlighted because the mouse
    // is upon it
    int x,y;
    int state = g_System.getMousePos(&x, &y);
    pMenu->mouseMotionEvent(x, y, state, KMD_NONE);

    // Adds a dirty rect to force menu rendering
    addRect(0, 0, g_Screen.gameScreenWidth(), g_Screen.gameScreenHeight());

    // reopen the event processing
    drop_events_ = false;
}

/*!
 * Displays the closing menu animation if the flag is true.
 * Before playing animation calls Menu.handleLeave().
 * \param pMenu The closing menu
 * \param playAnim True to play the animation.
 */
void MenuManager::leaveMenu(Menu *pMenu) {
    if (pMenu->isScreenMode()) {
        g_Screen.leaveWorldMode();
    }

    pMenu->leave();

    if (pMenu->hasLeaveAnim()) {
        drop_events_ = true;
        FliPlayer fliPlayer(this);
        uint8 *data;
        int size;
        data = File::loadOriginalFile(pMenu->getLeaveAnimName(), size);
        fliPlayer.loadFliData(data);
        pGameSounds_->play(snd::MENU_CHANGE);
        fliPlayer.play();
        delete[] data;
        drop_events_ = false;
    }
}

/*!
 * Copy all current screen pixels to a back buffer.
 */
void MenuManager::saveBackground() {
    needBackground_ = true;
}

/*!
 * Blit a portion of the background to the current screen.
 * \param x Origin of the blit rect. 
 * \param y Origin of the blit rect.
 * \param width Width of the blit rect.
 * \param height Height of the blit rect.
 */
void MenuManager::blitFromBackground(int x, int y, int width, int height) {
    g_Screen.renderTexture(background_texture_, x, y, width * 2, height * 2);
}

/*!
 * Renders the complete background.
 */
void MenuManager::renderBackground(void) {
    blitFromBackground(0, 0, background_texture_->width(), background_texture_->height());
}

/*!
 * Renders the current menu if there is one 
 * and if it needs to be refreshed.
 */
void MenuManager::renderMenu() {

    if (current_->isScreenMode()) {
        g_Screen.leaveWorldMode();
    }

    // updateOffset();
    renderBackground();

    if (current_ && !dirtyList_.isEmpty()) {
        /*
        if (needBackground_) {
            for (int i=0; i < dirtyList_.getSize(); i++) {
                DirtyRect *rect = dirtyList_.getRectAt(i);
                blitFromBackground(rect->x, rect->y, rect->width, rect->height);
            }
        }
        */
        // renderBackground();

        current_->render(dirtyList_);
        // flush dirty list
        // dirtyList_.flush();
    }
}

void MenuManager::handleEvents() {
    FS_Event evt;
    while(g_System.pumpEvents(&evt)) {
        switch(evt.type) {
        case EVT_QUIT:
            gotoMenu(Menu::kMenuIdLogout);
            break;
        case EVT_MSE_MOTION:
            if (current_ && !drop_events_)
                current_->mouseMotionEvent(evt.motion.x, evt.motion.y, evt.motion.state, evt.motion.keyMods);
            break;
        case EVT_MSE_DOWN:
            since_mouse_down_ = 0;
            mouseup_was_ = false;
#ifdef _DEBUG
            // Display mouse coordinate
            if (evt.button.keyMods & KMD_SHIFT) {
                printf("Mouse is at %d, %d\n", evt.motion.x, evt.motion.y);
            }
#endif

            if (current_ && !drop_events_) {
                current_->mouseDownEvent(evt.button.x, evt.button.y, evt.button.button, evt.button.keyMods);
            }
            break;
        case EVT_MSE_UP:
            mouseup_was_ = true;
            if (current_ && !drop_events_) {
                current_->mouseUpEvent(evt.button.x, evt.button.y, evt.button.button, evt.button.keyMods);
            }
            break;
        case EVT_KEY_DOWN:
            if (current_ && !drop_events_) {
                current_->keyEvent(evt.key.key, evt.key.keyMods);
            }
            break;
        case EVT_NONE:
            break;
        }
    }
}

bool MenuManager::simpleMouseDown() {
    return g_Ctx.getTimeForClick() < since_mouse_down_;
}
