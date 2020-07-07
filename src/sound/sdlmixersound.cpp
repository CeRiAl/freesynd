/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
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
#include "audio.h"

#ifdef HAVE_SDL_MIXER

#include "sdlmixersound.h"

/*!
 * Class constructor. Initialize date with NULL.
 */
SdlMixerSound::SdlMixerSound():sound_data_(NULL)
{
}

/*!
 * Class destructor. free the sound data if it 
 * was allocated.
 */
SdlMixerSound::~SdlMixerSound()
{
    if (sound_data_) {
        Mix_FreeChunk(sound_data_);
    }
}

/*!
 * Plays the sound a number of times.
 * The method checks if the sound system has been initialized before
 * playing the sound.
 * \param loops The number of times the sound is played. Value of -1
 * plays the sound indefinitly until the sound is stopped.
 * \param channel
 */
void SdlMixerSound::play(int loops, int channel) const
{
    if (Audio::isInitialized()) {
        int ret = Mix_PlayChannel(channel, sound_data_, loops);
        if (ret < 0) {
            Audio::error("Sound", "play", "Failed to play sound on channel 0.");
        }
    }
}

/*!
 * Stops the sound from playing.
 * \param channel
 */
void SdlMixerSound::stop(int channel) const
{
    if (Audio::isInitialized()) {
        Mix_HaltChannel(channel);
    }
}

/*!
 * Each sample has its own volume which is taken into account
 * on the mixing phase. This method sets the volume of this
 * sample.
 * \param A value between 0 and maximum volume.
 * \return true if the volume has been correctly set.
 */
bool SdlMixerSound::setVolume(int volume)
{
    if (Audio::isInitialized()) {
        int ret = Mix_VolumeChunk(sound_data_, volume);
        if (ret < 0) {
            Audio::error("Sound", "setVolume", "Failed setting volume on Sound.");
            return false;
        }
    }
    return true;
}

/*!
 * Loads the sample from the given data.
 * \param soundData Data as loaded from original resource
 * \param size The size of the input data
 */
bool SdlMixerSound::loadSound(uint8 * soundData, uint32 size)
{
    if (!Audio::isInitialized()) {
        return false;
    }

    SDL_RWops *rw = SDL_RWFromMem(soundData, size);
    if (!rw) {
        Audio::error("Sound", "loadSound", "Failed creating SDL_RW buffer from memory");
        return false;
    }
    

    SDL_RWops *src = rw;
    bool freesrc = true;

    Uint8 magic[4];
    Mix_Chunk *chunk;
    SDL_AudioSpec wavespec, *loaded;
    SDL_AudioCVT wavecvt;
    int samplesize;

    /* rcg06012001 Make sure src is valid */
    if (!src) {
        SDL_SetError("Mix_LoadWAV_RW with NULL src");
        return(NULL);
    }

    /* Make sure audio has been opened */
    /*
    if (!audio_opened) {
        SDL_SetError("Audio device hasn't been opened");
        if (freesrc) {
            SDL_RWclose(src);
        }
        return(NULL);
    }
    */

    /* Allocate the chunk memory */
    chunk = (Mix_Chunk *)SDL_malloc(sizeof(Mix_Chunk));
    if (chunk == NULL) {
        SDL_SetError("Out of memory");
        if (freesrc) {
            SDL_RWclose(src);
        }
        return(NULL);
    }

    /*
    const char *str = "CREA";
    size_t len = SDL_strlen(str);
    if (SDL_RWwrite(rw, str, 1, len) != len) {
        printf("Couldn't fully write string\n");
    } else {
        printf("Wrote %d 1-byte blocks\n", len);
    }
    SDL_RWseek(src, -len, RW_SEEK_CUR);
    */

    /* Find out what kind of audio file this is */
    if (SDL_RWread(src, magic, 1, 4) != 4) {
        if (freesrc) {
            SDL_RWclose(src);
        }
        Mix_SetError("Couldn't read first 4 bytes of audio data");
        return NULL;
    }
    /* Seek backwards for compatibility with older loaders */
    SDL_RWseek(src, -4, RW_SEEK_CUR);

    if (SDL_memcmp(magic, "WAVE", 4) == 0 || SDL_memcmp(magic, "RIFF", 4) == 0) {
        printf("Load WAVE\n");
        // loaded = SDL_LoadWAV_RW(src, freesrc, &wavespec, (Uint8 **)&chunk->abuf, &chunk->alen);
    } else if (SDL_memcmp(magic, "FORM", 4) == 0) {
        printf("Load AIFF\n");
        // loaded = Mix_LoadAIFF_RW(src, freesrc, &wavespec, (Uint8 **)&chunk->abuf, &chunk->alen);
    } else if (SDL_memcmp(magic, "CREA", 4) == 0) {
        printf("Load VOC\n");
        // loaded = Mix_LoadVOC_RW(src, freesrc, &wavespec, (Uint8 **)&chunk->abuf, &chunk->alen);
    } else {
        // Mix_MusicType music_type = detect_music_type_from_magic(magic);
        // loaded = Mix_LoadMusic_RW(music_type, src, freesrc, &wavespec, (Uint8 **)&chunk->abuf, &chunk->alen);
    }
    if (!loaded) {
        /* The individual loaders have closed src if needed */
        SDL_free(chunk);
        return(NULL);
    }


    Mix_Chunk *newsound = Mix_LoadWAV_RW(rw, 1);
    

    if (!newsound) {
        Audio::error("Sound", "loadSound", "Failed loading sound from SDL_RW buffer");
        return false;
    }
    if (sound_data_ != 0)
        Mix_FreeChunk(sound_data_);
    sound_data_ = newsound;
    return true;
}

#endif                          // HAVE_SDL_MIXER
