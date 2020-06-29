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
    cursor_texture_ = 0;
}

SystemSDL::~SystemSDL() {
    if (cursor_texture_) {
        glDeleteTextures(1, &cursor_texture_);
    }

    if (gl_context_) {
        SDL_GL_DeleteContext(gl_context_);
    }

    if (display_window_) {
        SDL_DestroyWindow(display_window_);
    }

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
                         GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT,
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
    /*** render cursor  ***/
    if (cursor_visible_) {
        SDL_Rect dst;
        dst.w = cursor_rect_.w;
        dst.h = cursor_rect_.h;
        dst.x = cursor_x_ - cursor_hs_x_;
        dst.y = cursor_y_ - cursor_hs_y_;

        GLfloat dcx = GLfloat(cursor_rect_.x) / GLfloat(96);
        GLfloat dcy = GLfloat(cursor_rect_.y) / GLfloat(48);

        GLfloat dcw = (GLfloat(cursor_rect_.x)+GLfloat(cursor_rect_.w)) / GLfloat(96);
        GLfloat dch = (GLfloat(cursor_rect_.y)+GLfloat(cursor_rect_.h)) / GLfloat(48);

        // Set our loaded texture as the current 2D texture (this isn't actually technically necessary since our
        // texture was never unselected from above, but this is most clear)
        glBindTexture(GL_TEXTURE_2D, cursor_texture_);

        // Tell OpenGL that all subsequent drawing operations should try to use the current 2D texture
        glEnable(GL_TEXTURE_2D);
    
        glBegin(GL_QUADS);
            // glTexCoord2f(0, 0);
            glTexCoord2f(dcx, dcy);
            glVertex2f(dst.x, dst.y);

            // glTexCoord2f(1, 0);
            glTexCoord2f(dcw, dcy);
            glVertex2f(dst.x + cursor_rect_.w, dst.y);

            // glTexCoord2f(1, 1);
            glTexCoord2f(dcw, dch);
            glVertex2f(dst.x + cursor_rect_.w, dst.y + cursor_rect_.h);

            // glTexCoord2f(0, 1);
            glTexCoord2f(dcx, dch);
            glVertex2f(dst.x, dst.y + cursor_rect_.h);
        glEnd();

        // Tell OpenGL that all subsequent drawing operations should NOT try to use the current 2D texture
        glDisable(GL_TEXTURE_2D);
    

        glBegin(GL_LINES);
            glColor3f(1.0f, 0.0f, 0.0f);
            glVertex3f(0.0f, 0.0f, 0.0f);
            glVertex3f(1.0f, 0.0f, 0.0f);

            glColor3f(0.0f, 1.0f, 0.0f);
            glVertex3f(0.0f, 0.0f, 0.0f);
            glVertex3f(0.0f, 1.0f, 0.0f);

            glColor3f(0.0f, 0.0f, 1.0f);
            glVertex3f(0.0f, 0.0f, 0.0f);
            glVertex3f(0.0f, 0.0f, 1.0f);
            glColor3f(1.0f, 1.0f, 1.0f);
        glEnd();
    
        // SDL_GL_SwapWindow(display_window_);

        // //////////////

#if 0
        SDL_Rect dst;

        dst.w = cursor_rect_.w;
        dst.h = cursor_rect_.h;
        dst.x = cursor_x_ - cursor_hs_x_;
        dst.y = cursor_y_ - cursor_hs_y_;
        // SDL_RenderCopy(display_renderer_, cursor_texture_, &cursor_rect_, &dst);

        {
            // glUseProgramObjectARB(0);
            glPushMatrix();
    	    glTranslatef(dst.x, dst.y, 0);

            // glEnable(GL_TEXTURE_2D);

            glBindTexture(GL_TEXTURE_2D, cursor_texture_);

            GLint whichID;
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &whichID);

            /*
            float texw;
            float texh;
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            int res1 = SDL_GL_BindTexture(cursor_texture_, &texw, &texh);
            if (res1 != 0) {
                printf("Unable to bind texture! SDL Error: %s\n", SDL_GetError());
            }
            */

            /*
            if(true) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            }

            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            */

            glBegin(GL_QUADS);
                glTexCoord2f(0, 0);
                glVertex3f(cursor_rect_.x, cursor_rect_.y, 0);
                glTexCoord2f(1, 0);
                glVertex3f(cursor_rect_.x + cursor_rect_.w, cursor_rect_.y, 0);
                glTexCoord2f(1, 1);
                glVertex3f(cursor_rect_.x + cursor_rect_.w, cursor_rect_.y + cursor_rect_.h, 0);
                glTexCoord2f(0, 1);
                glVertex3f(cursor_rect_.x, cursor_rect_.y + cursor_rect_.h, 0);
            glEnd();

            /*
            glBegin(GL_QUADS);
                glTexCoord2f(0.0, 0.0); 
                glVertex2f(cursor_rect_.x, cursor_rect_.y);

                glTexCoord2f(0.0, 1.0);
                glVertex2f(cursor_rect_.x, cursor_rect_.y + cursor_rect_.h);

                glTexCoord2f(1.0, 1.0);
                glVertex2f(cursor_rect_.x + cursor_rect_.w, cursor_rect_.y + cursor_rect_.h);

                glTexCoord2f(1.0, 0.0);
                glVertex2f(cursor_rect_.x + cursor_rect_.w, cursor_rect_.y);
            glEnd();
            */

            // glDisable(GL_TEXTURE_2D);

            glGetIntegerv(GL_TEXTURE_BINDING_2D, &whichID);

            glBindTexture(GL_TEXTURE_2D, 0);
            // SDL_GL_UnbindTexture(cursor_texture_);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);

            glPopMatrix();
        }
#endif

        update_cursor_ = false;
    }
}

void SystemSDL::updateScreen() {
    if (g_Screen.glDirty()) {
        g_Screen.clearGlDirty();
        swapWindow();

        return;
    }

    if (g_Screen.dirty() || (cursor_visible_ && update_cursor_)) {        
        g_Screen.renderScreen();

        /**********************/
        /** render on screen **/
        /**********************/
        g_Screen.enterOnScreenMode();
            renderCursor();
        g_Screen.leaveOnScreenMode();

        swapWindow();
    }
}

void SystemSDL::swapWindow() {
    // glFlush();
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
#if _DEBUG
                    key.unicode = 0;
                    if (key.keyFunc == KFC_UNKNOWN) {
                        // key.unicode = evtIn.key.keysym.unicode;
                        printf( "Scancode: 0x%02X", evtIn.key.keysym.scancode );
                        printf( ", Name: %s", SDL_GetKeyName( evtIn.key.keysym.sym ) );
                        // printf(", Unicode: " );
                        // if( evtIn.key.keysym.unicode < 0x80 && evtIn.key.keysym.unicode > 0 ){
                        //     printf( "%c (0x%04X)\n", (char)evtIn.key.keysym.unicode,
                        //             evtIn.key.keysym.unicode );
                        // } else{
                        //     printf( "? (0x%04X)\n", evtIn.key.keysym.unicode );
                        // }
#endif
                    }
                    pEvtOut->key.key = key;
                    pEvtOut->key.keyMods = keyModState_;
                    break;
                } // end switch
            } // end case SDL_KEYDOWN
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

#if 0
        if (like(palette[i].r, 28) && like(palette[i].g, 144)
            && like(palette[i].b, 0))
            printf("col %i = %i, %i, %i\n", i, palette[i].r, palette[i].g,
                   palette[i].b);
#endif
    }

    // SDL_SetPaletteColors(g_Screen.temp_surface()->format->palette, palette, 0, cols);
    g_Screen.setPalette(palette, cols);
}

void SystemSDL::setPalette8b3(const uint8 * pal, int cols) {
    static SDL_Color palette[256];

    for (int i = 0; i < cols; ++i) {
        palette[i].r = pal[i * 3 + 0];
        palette[i].g = pal[i * 3 + 1];
        palette[i].b = pal[i * 3 + 2];
    }

    // SDL_SetPaletteColors(g_Screen.temp_surface()->format->palette, palette, 0, cols);
    g_Screen.setPalette(palette, cols);
}

void SystemSDL::setColor(uint8 index, uint8 r, uint8 g, uint8 b) {
    static SDL_Color color;

    color.r = r;
    color.g = g;
    color.b = b;

    // SDL_SetColors(temp_surf_, &color, index, 1);
    SDL_SetPaletteColors(g_Screen.temp_surface()->format->palette, &color, 0, 1);
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

    // Determine the data format of the surface by seeing how SDL arranges a test pixel.
    // This probably only works correctly for little-endian machines.
    /*
    GLenum gl_mode = GL_RGB;    
    */
    GLenum gl_mode;
    Uint8 test = SDL_MapRGB(cursor_surf->format, 0xAA, 0xBB, 0xCC) & 0xFF;
    if      (test == 0xAA) gl_mode =         GL_RGB;
    else if (test == 0xCC) gl_mode = 0x80E0;//GL_BGR;
    else {
        printf("Error: \"Loaded surface was neither RGB or BGR!\"");
        return false;
    }
    if(cursor_surf->format->BytesPerPixel == 4) {
        gl_mode = GL_RGBA;
    }
 
    // Generate an array of textures.  We only want one texture (one element array), so trick
    // it by treating "texture" as array of length one.
    glGenTextures(1, &cursor_texture_);

    // Select (bind) the texture we just generated as the current 2D texture OpenGL is using/modifying.
    // All subsequent changes to OpenGL's texturing state for 2D textures will affect this texture.
    glBindTexture(GL_TEXTURE_2D, cursor_texture_);
    // Specify the texture's data.  This function is a bit tricky, and it's hard to find helpful documentation.  A summary:
    //   GL_TEXTURE_2D:    The currently bound 2D texture (i.e. the one we just made)
    //               0:    The mipmap level.  0, since we want to update the base level mipmap image (i.e., the image itself,
    //                         not cached smaller copies)
    //         GL_RGBA:    The internal format of the texture.  This is how OpenGL will store the texture internally (kinda)--
    //                         it's essentially the texture's type.
    //         surf->w:    The width of the texture
    //         surf->h:    The height of the texture
    //               0:    The border.  Don't worry about this if you're just starting.
    //        data_fmt:    The format that the *data* is in--NOT the texture!  Our test image doesn't have an alpha channel,
    //                         so this must be RGB.
    // GL_UNSIGNED_BYTE:    The type the data is in.  In SDL, the data is stored as an array of bytes, with each channel
    //                         getting one byte.  This is fairly typical--it means that the image can store, for each channel,
    //                         any value that fits in one byte (so 0 through 255).  These values are to be interpreted as
    //                         *unsigned* values (since 0x00 should be dark and 0xFF should be bright).
    // surface->pixels:    The actual data.  As above, SDL's array of bytes.
    /*
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA, cursor_surf->w, cursor_surf->h, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, cursor_surf->pixels);
    glTexImage2D(
        GL_TEXTURE_2D, 0, gl_mode, cursor_surf->w, cursor_surf->h, 0, gl_mode,
        GL_UNSIGNED_BYTE, cursor_surf->pixels);
    */

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cursor_surf->w, cursor_surf->h,
                                0, gl_mode, GL_UNSIGNED_BYTE, cursor_surf->pixels);

    //Set the minification and magnification filters.  In this case, when the texture is minified (i.e., the texture's pixels (texels) are
    //*smaller* than the screen pixels you're seeing them on, linearly filter them (i.e. blend them together).  This blends four texels for
    //each sample--which is not very much.  Mipmapping can give better results.  Find a texturing tutorial that discusses these issues
    //further.  Conversely, when the texture is magnified (i.e., the texture's texels are *larger* than the screen pixels you're seeing
    //them on), linearly filter them.  Qualitatively, this causes "blown up" (overmagnified) textures to look blurry instead of blocky.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 
    // Unload SDL's copy of the data; we don't need it anymore because OpenGL now stores it in the texture.
    SDL_FreeSurface(cursor_surf);

    // //

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
    if (cursor_texture_ != 0) {
        cursor_visible_ = false;
    } else {
        // Custom cursor surface doesn't
        // exists so use the default SDL Cursor
        SDL_ShowCursor(SDL_DISABLE);
    }
}

void SystemSDL::showCursor() {
    if (cursor_texture_ != 0) {
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
