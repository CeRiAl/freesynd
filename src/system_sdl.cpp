/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
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

#include "config.h"
#include "gfx/screen.h"
#include "system.h"
#include "sound/audio.h"
#include "utils/file.h"
#include "utils/log.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL_image.h>

SDL_Joystick *joy = NULL;

void errorcheck() {
    GLenum error = glGetError();
    if( error != GL_NO_ERROR ) {
        printf( "OpenGL Error! %s\n", gluErrorString( error ) );
        exit(0);
    }
}

const int SystemSDL::CURSOR_WIDTH = 24;

SystemSDL::SystemSDL(int depth) {
    depth_ = depth;
    keyModState_ = 0;
    display_window_ = NULL;
    gl_context_ = NULL;
    cursor_texture_ = NULL;
#if _DEBUG
    debug_mode_ = false;
#endif
}

SystemSDL::~SystemSDL() {
    if (cursor_texture_)
        delete cursor_texture_;

    if (gl_context_)
        SDL_GL_DeleteContext(gl_context_);

    if (display_window_)
        SDL_DestroyWindow(display_window_);

#ifdef HAVE_SDL_MIXER
    Audio::quit();
#endif

    // Destroy SDL_Image Lib
    IMG_Quit();

    SDL_Quit();
}

void SystemSDL::initCursor(void) {
    // Init SDL_Image library
    int sdl_img_flags = IMG_INIT_PNG;
    int initted = IMG_Init(sdl_img_flags);
    if ( (initted & sdl_img_flags) != sdl_img_flags ) {
        printf("Failed to init SDL_Image : %s\n", IMG_GetError());
    } else {
        // Load the cursor sprites
        if (loadCursorSprites()) {
            // Cursor movement is managed by the application
            SDL_ShowCursor(SDL_DISABLE);
        }
        // At first the cursor is hidden
        hideCursor();
        useMenuCursor();
    }
}

bool SystemSDL::initialize(bool fullscreen) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Critical error, SDL could not be initialized!");
        return false;
    }

    // Keyboard init
    // SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
    // SDL_EnableUNICODE(1);

    // Audio initialisation
    if (!Audio::init()) {
        LOG(Log::k_FLG_SND, "SystemSDL", "Init", ("Couldn't initialize Sound System : no sound will be played."))
    }

    display_window_ =
        SDL_CreateWindow("FreeSynd",
                         SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED,
                         g_Screen.gameScreenWidth(), g_Screen.gameScreenHeight(),
                         SDL_WINDOW_OPENGL);

	printf("Initializing OpenGL context.\n");
	gl_context_ = SDL_GL_CreateContext(display_window_);
	if(!gl_context_) {
      fprintf(stderr, "Couldn't create OpenGL context: %s\n", SDL_GetError());
    }

    g_Screen.initScreen();

    initCursor();

    return true;
}

void SystemSDL::renderCursor() {
    if (cursor_visible_ && update_cursor_) {
        SDL_Rect dst_rect = {
            cursor_x_ - cursor_hs_x_, cursor_y_ - cursor_hs_y_,
            cursor_rect_.w, cursor_rect_.h
        };

        g_Screen.renderTexture(cursor_texture_,
            dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h,
            cursor_rect_.x, cursor_rect_.y, cursor_rect_.w, cursor_rect_.h);

#if 0
        SDL_RenderCopy(display_renderer_, cursor_texture_, &cursor_rect_, &dst_rect);

        update_cursor_ = false;
#endif
    }
}

void SystemSDL::updateScreen() {

    // Render game
    // g_Screen.renderGame();

    // Render cursor
    renderCursor();

    // g_Screen.renderScreen();

    // Swap buffer
    swapWindow();
}

void SystemSDL::swapWindow() {
    glFlush();
    SDL_GL_SwapWindow(display_window_);
}

/*!
 * Using the keysym parameter, verify if the given key is a function key (ie
 * a not printable key) returns the corresponding entry in the KeyFunc enumeration.
 * \returns If key code is not a function key, returns KEY_UNKNOWN.
 */
void SystemSDL::checkKeyCodes(SDL_Keysym keysym, Key &key) {
    key.keyFunc = KFC_UNKNOWN;
    key.keyVirt = KVT_UNKNOWN;
    switch(keysym.sym) {
        case SDLK_ESCAPE: key.keyFunc = KFC_ESCAPE; break;
        case SDLK_BACKSPACE: key.keyFunc = KFC_BACKSPACE; break;
        case SDLK_RETURN: key.keyFunc = KFC_RETURN; break;
        case SDLK_DELETE: key.keyFunc = KFC_DELETE; break;
        case SDLK_UP:
        case SDLK_DOWN:
        case SDLK_RIGHT:
        case SDLK_LEFT:
        case SDLK_INSERT:
        case SDLK_HOME:
        case SDLK_END:
        case SDLK_PAGEUP:
        case SDLK_PAGEDOWN:
            key.keyFunc = static_cast < KeyFunc > (KFC_UP + (keysym.sym - SDLK_UP));
            break;
        case SDLK_F1:
        case SDLK_F2:
        case SDLK_F3:
        case SDLK_F4:
        case SDLK_F5:
        case SDLK_F6:
        case SDLK_F7:
        case SDLK_F8:
        case SDLK_F9:
        case SDLK_F10:
        case SDLK_F11:
        case SDLK_F12:
            key.keyFunc = static_cast < KeyFunc > (KFC_F1 + (keysym.sym - SDLK_F1));
            break;
        case SDLK_0:case SDLK_1:case SDLK_2:case SDLK_3:case SDLK_4:
        case SDLK_5:case SDLK_6:case SDLK_7:case SDLK_8:case SDLK_9:
            key.keyVirt = static_cast < KeyVirtual > (KVT_NUMPAD0 + (keysym.sym - SDLK_0));
            break;
        default:
            // unused key
            break;
    }
}

//! Pumps an event from the event queue
/*!
 * Watch the event queue and dispatch events.
 * - keyboard events : when a modifier key is pressed,
 * the system does not dispatch the event to the application:
 * it stores the key state and then passes the complete state
 * when a regular key is pressed. So that the application knows
 * if multiple modifier keys are pressed at the same time (ie Ctrl/Shift)
 */
bool SystemSDL::pumpEvents(FS_Event *pEvtOut) {
    SDL_Event evtIn;

    pEvtOut->type = EVT_NONE;

    if (SDL_PollEvent(&evtIn)) {
        switch (evtIn.type) {
        case SDL_QUIT:
            pEvtOut->quit.type = EVT_QUIT;
            break;
        case SDL_KEYDOWN:
            {
            // Check if key pressed is a modifier
            switch(evtIn.key.keysym.sym) {
                case SDLK_RSHIFT:
                    keyModState_ = keyModState_ | KMD_RSHIFT; 
                    break;
                case SDLK_LSHIFT:
                    keyModState_ = keyModState_ | KMD_LSHIFT; 
                    break;
                case SDLK_RCTRL:
                    keyModState_ = keyModState_ | KMD_RCTRL; 
                    break;
                case SDLK_LCTRL:
                    keyModState_ = keyModState_ | KMD_LCTRL; 
                    break;
                case SDLK_RALT:
                    keyModState_ = keyModState_ | KMD_RALT; 
                    break;
                case SDLK_LALT:
                    keyModState_ = keyModState_ | KMD_LALT; 
                    break;
                default:
                    // We pass the event only if it's not a allowed modifier key
                    // Plus, the application receives event only when key is pressed
                    // not released.
                    pEvtOut->type = EVT_KEY_DOWN;
                    Key key;
                    checkKeyCodes(evtIn.key.keysym, key);
                    key.unicode = 0;
                    key.keySym = 0;
                    if (key.keyFunc == KFC_UNKNOWN) {
                        key.keySym = evtIn.key.keysym.sym;
#if _DEBUG
                        printf( "Scancode: 0x%02X\t", evtIn.key.keysym.scancode );
                        printf( "Sym: 0x%02X", evtIn.key.keysym.sym );
                        printf( " (Name: %s )\n", SDL_GetKeyName( evtIn.key.keysym.sym ) );
#endif
                    }
                    pEvtOut->key.key = key;
                    pEvtOut->key.keyMods = keyModState_;
                    break;
                } // end switch
            } // end case SDL_KEYDOWN

#if _DEBUG
            if (isLetterD(pEvtOut->key.key.keySym) && (pEvtOut->key.keyMods & KMD_ALT)) {
                debug_mode_ = !debug_mode_;

                printf( "Debug mode now: %s\n", debug_mode_ ? "ON" : "OFF" );
            }
#endif

            break;
        case SDL_KEYUP:
            {
            switch(evtIn.key.keysym.sym) {
                case SDLK_RSHIFT:
                    keyModState_ = keyModState_ & !KMD_RSHIFT;
                    break;
                case SDLK_LSHIFT:
                    keyModState_ = keyModState_ & !KMD_LSHIFT;
                    break;
                case SDLK_RCTRL:
                    keyModState_ = keyModState_ & !KMD_RCTRL;
                    break;
                case SDLK_LCTRL:
                    keyModState_ = keyModState_ & !KMD_LCTRL;
                    break;
                case SDLK_RALT:
                    keyModState_ = keyModState_ & !KMD_RALT;
                    break;
                case SDLK_LALT:
                    keyModState_ = keyModState_ & !KMD_LALT;
                    break;
                default:
                    break;
            }
            }
            break;
        case SDL_TEXTINPUT:
            // TODO: Add new text onto the end of our text
            pEvtOut->type = EVT_KEY_DOWN;
            Key key;
            key.keySym = 0;
            key.unicode = evtIn.text.text[0];
#if _DEBUG
            // printf( "Unicode: 0x%02X (Content: '%s')\n", evtIn.text.text[0], evtIn.text.text);
#endif
            pEvtOut->key.key = key;
            pEvtOut->key.keyMods = keyModState_;
            break;
        case SDL_TEXTEDITING:
#if _DEBUG
            printf("TEXTEDITING!!! composition %s\n", evtIn.edit.text); // Update the composition text.
            printf("TEXTEDITING!!! cursor %i\n", evtIn.edit.start); // Update the cursor position.
            printf("TEXTEDITING!!! selection_len %i\n", evtIn.edit.length); // Update the selection length (if any).
#endif
            break;
        case SDL_MOUSEBUTTONUP:
            pEvtOut->button.type = EVT_MSE_UP;
            pEvtOut->button.x = evtIn.button.x;
            pEvtOut->button.y = cursor_y_ = evtIn.button.y;
            pEvtOut->button.button = evtIn.button.button;
            pEvtOut->button.keyMods = keyModState_;
            break;
        case SDL_MOUSEBUTTONDOWN:
            pEvtOut->button.type = EVT_MSE_DOWN;
            pEvtOut->button.x = evtIn.button.x;
            pEvtOut->button.y = cursor_y_ = evtIn.button.y;
            pEvtOut->button.button = evtIn.button.button;
            pEvtOut->button.keyMods = keyModState_;
            break;
        case SDL_MOUSEMOTION:
            update_cursor_ = true;
            pEvtOut->motion.type = EVT_MSE_MOTION;
            pEvtOut->motion.x = cursor_x_ = evtIn.motion.x;
            pEvtOut->motion.y = cursor_y_ = evtIn.motion.y;
            pEvtOut->motion.state = evtIn.motion.state;
            pEvtOut->motion.keyMods = keyModState_;
            break;
        default:
            break;
        }
    }

    return pEvtOut->type != EVT_NONE;
}

void SystemSDL::delay(int msec) {
    SDL_Delay(msec);
}

int SystemSDL::getTicks() {
    return SDL_GetTicks();
}

bool like(int a, int b) {
    return a == b || a == b - 1 || a == b + 1;
}

void SystemSDL::setPalette6b3(const uint8 * pal, int cols) {
    static SDL_Color palette[256];

    for (int i = 0; i < cols; ++i) {
        uint8 r = pal[i * 3 + 0];
        uint8 g = pal[i * 3 + 1];
        uint8 b = pal[i * 3 + 2];

        // multiply by 255 divide by 63 isn't good enough?
        palette[i].r = (r << 2) | (r >> 4);
        palette[i].g = (g << 2) | (g >> 4);
        palette[i].b = (b << 2) | (b >> 4);
        palette[i].a = 255;

#if 0
        if (like(palette[i].r, 28) && like(palette[i].g, 144)
            && like(palette[i].b, 0))
            printf("col %i = %i, %i, %i\n", i, palette[i].r, palette[i].g,
                   palette[i].b);
#endif
    }

    palette[255].a = 0;

    g_Screen.setPalette(palette, cols);
}

void SystemSDL::setPalette8b3(const uint8 * pal, int cols) {
    static SDL_Color palette[256];

    for (int i = 0; i < cols; ++i) {
        palette[i].r = pal[i * 3 + 0];
        palette[i].g = pal[i * 3 + 1];
        palette[i].b = pal[i * 3 + 2];
        palette[i].a = 255;
    }

    g_Screen.setPalette(palette, cols);
}

void SystemSDL::setColor(uint8 index, uint8 r, uint8 g, uint8 b) {
    static SDL_Color color;

    color.r = r;
    color.g = g;
    color.b = b;

    // SDL_SetColors(temp_surf_, &color, index, 1);
    // FIXME!!
}

/* Quick utility function for texture creation */
static int powerOfTwo(int input) {
	int value = 1;

	while (value < input) {
		value <<= 1;
	}
	return value;
}

SDL_Surface *prepGLTexture(SDL_Surface *surface, GLfloat *texCoords, const bool freeSource) {
	/* Use the surface width and height expanded to powers of 2 */
	int w = powerOfTwo(surface->w);
	int h = powerOfTwo(surface->h);
	if (texCoords != 0) {
		texCoords[0] = 0.0f;					/* Min X */
		texCoords[1] = 0.0f;					/* Min Y */
		texCoords[2] = (GLfloat)surface->w / w;	/* Max X */
		texCoords[3] = (GLfloat)surface->h / h;	/* Max Y */
	}

	SDL_Surface *image = SDL_CreateRGBSurface(
		SDL_SWSURFACE,
		w, h,
		32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN /* OpenGL RGBA masks */
		0x000000FF, 
		0x0000FF00, 
		0x00FF0000, 
		0xFF000000
#else
		0xFF000000,
		0x00FF0000, 
		0x0000FF00, 
		0x000000FF
#endif
	);
	if ( image == NULL ) {
		return 0;
	}

	/* Save the alpha blending attributes */
    /*
	Uint32 savedFlags = surface->flags&(SDL_SRCALPHA|SDL_RLEACCELOK);
	Uint8  savedAlpha = surface->format->alpha;
	if ( (savedFlags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
		SDL_SetAlpha(surface, 0, 0);
	}
    */

	SDL_Rect srcArea, destArea;
	/* Copy the surface into the GL texture image */
	srcArea.x = 0; destArea.x = 0;
	/* Copy it in at the bottom, because we're going to flip
	   this image upside-down in a moment
	*/
	srcArea.y = 0; destArea.y = h - surface->h;
	srcArea.w = surface->w;
	srcArea.h = surface->h;
	SDL_BlitSurface(surface, &srcArea, image, &destArea);

	/* Restore the alpha blending attributes */
    /*
	if ((savedFlags & SDL_SRCALPHA) == SDL_SRCALPHA) {
		SDL_SetAlpha(surface, savedFlags, savedAlpha);
	}
    */

	/* Turn the image upside-down, because OpenGL textures
	   start at the bottom-left, instead of the top-left
	*/
#ifdef _MSC_VER
	Uint8 *line = new Uint8[image->pitch];
#else
	Uint8 line[image->pitch];
#endif
	/* These two make the following more readable */
	Uint8 *pixels = static_cast<Uint8*>(image->pixels);
	Uint16 pitch = image->pitch;
	int ybegin = 0;
	int yend = image->h - 1;

	// TODO: consider if this lock is legal/appropriate
	if (SDL_MUSTLOCK(image)) { SDL_LockSurface(image); }
	while (ybegin < yend) {
		memcpy(line, pixels + pitch*ybegin, pitch);
		memcpy(pixels + pitch*ybegin, pixels + pitch*yend, pitch);
		memcpy(pixels + pitch*yend, line, pitch);
		ybegin++;
		yend--;
	}
	if (SDL_MUSTLOCK(image)) { SDL_UnlockSurface(image); }

	if (freeSource) {
		SDL_FreeSurface(surface);
	}

#ifdef _MSC_VER
	delete[] line;
#endif

	return image;
}

/*!
 * This method uses the SDL_Image library to load a file called
 * cursors/cursors.png under the root path.
 * The file is loaded into the cursor surface.
 * \return False if the loading has failed. If it's the case, 
 * cursor_texture_ will be NULL.
 */
bool SystemSDL::loadCursorSprites() {
    SDL_Surface *cursor_surf = IMG_Load(File::dataFullPath("cursors/cursors.png").c_str());
    if (cursor_surf == NULL) {
        printf("Cannot load cursors image: %s\n", IMG_GetError());
        return false;
    }

    cursor_rect_.w = cursor_rect_.h = CURSOR_WIDTH;


    cursor_texture_ = new Texture(cursor_surf->w, cursor_surf->h);
    cursor_texture_->update(cursor_surf);

    SDL_FreeSurface(cursor_surf);

    // cursor_surf = prepGLTexture(cursor_surf);

    return true;
}

/*! 
 * Returns the mouse pointer coordinates using SDL_GetMouseState. 
 * \param x The x coordinate.
 * \param y The y coordinate.
 * \return See SDL_GetMouseState.
 */
int SystemSDL::getMousePos(int *x, int *y) {
    return SDL_GetMouseState(x, y);
}

void SystemSDL::hideCursor() {
    if (cursor_texture_) {
        cursor_visible_ = false;
    } else {
        // Custom cursor surface doesn't
        // exists so use the default SDL Cursor
        SDL_ShowCursor(SDL_DISABLE);
    }
}

void SystemSDL::showCursor() {
    if (cursor_texture_) {
        cursor_visible_ = true;
    } else {
        // Custom cursor surface doesn't
        // exists so use the default SDL Cursor
        SDL_ShowCursor(SDL_ENABLE);
    }
}

void SystemSDL::useMenuCursor() {
    update_cursor_ = true;
    cursor_rect_.x = cursor_rect_.y = 0;
    cursor_hs_x_ = cursor_hs_y_ = 0;
}

void SystemSDL::usePointerCursor() {
    update_cursor_ = true;
    cursor_rect_.x = 24;
    cursor_rect_.y = 0;
    cursor_hs_x_ = cursor_hs_y_ = 0;
}

void SystemSDL::usePointerYellowCursor() {
    update_cursor_ = true;
    cursor_rect_.x = 24;
    cursor_rect_.y = 24;
    cursor_hs_x_ = cursor_hs_y_ = 0;
}

void SystemSDL::useTargetCursor() {
    update_cursor_ = true;
    cursor_rect_.x = 48;
    cursor_rect_.y = 0;
    cursor_hs_x_ = cursor_hs_y_ = 10;
}

void SystemSDL::useTargetRedCursor() {
    update_cursor_ = true;
    cursor_rect_.x = 72;
    cursor_rect_.y = 0;
    cursor_hs_x_ = cursor_hs_y_ = 10;
}

void SystemSDL::usePickupCursor() {
    update_cursor_ = true;
    cursor_rect_.x = 0;
    cursor_rect_.y = 24;
    cursor_hs_x_ = cursor_hs_y_ = 2;
}
