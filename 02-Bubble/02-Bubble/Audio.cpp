#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <iostream>
#include "Audio.h"
#include <string>


namespace
{
	const int VOICES = 8;

	const char* SOUND_PATHS[Audio::NUM_SOUNDS] = {
		"sounds/tnt_set.wav",
		"sounds/tnt_pss.wav",
		"sounds/tnt_explosion.wav",
		"sounds/click_stereo.wav",
		"sounds/hit.wav",
		"sounds/levelup.wav",
		"sounds/powerup.wav",
		"sounds/creeper_death.wav",
		"sounds/enderman_death.wav",
		"sounds/enderdragon_hit.wav",
		"sounds/end.wav",
		"sounds/zombie_death.wav",
	};

	ma_engine engine;
	ma_sound voices[Audio::NUM_SOUNDS][VOICES];
	bool voiceLoaded[Audio::NUM_SOUNDS][VOICES];
	bool engineStarted = false;

	const float SOUND_VOLUMES[Audio::NUM_SOUNDS] = { 0.2f, 0.2f, 0.2f, 0.1f, 1.0f, 0.05f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f };

	ma_sound music;
	bool musicLoaded = false;
	float musicVolume = 0.5f;
	std::string currentMusic;
}


Audio::Audio()
	: ready(false)
{
}

Audio::~Audio()
{
	shutdown();
}

bool Audio::init()
{
	if (ready)
		return true;

	if (ma_engine_init(NULL, &engine) != MA_SUCCESS)
	{
		std::cout << "Could not start audio engine" << std::endl;
		return false;
	}
	engineStarted = true;

	for (int s = 0; s < NUM_SOUNDS; s++)
	{
		for (int v = 0; v < VOICES; v++)
			voiceLoaded[s][v] = false;

		if (ma_sound_init_from_file(&engine, SOUND_PATHS[s], MA_SOUND_FLAG_DECODE, NULL, NULL, &voices[s][0]) != MA_SUCCESS)
		{
			std::cout << "Could not load " << SOUND_PATHS[s] << std::endl;
			continue;
		}
		voiceLoaded[s][0] = true;

		for (int v = 1; v < VOICES; v++)
			voiceLoaded[s][v] = (ma_sound_init_copy(&engine, &voices[s][0], 0, NULL, &voices[s][v]) == MA_SUCCESS);

		for (int v = 0; v < VOICES; v++)
			if (voiceLoaded[s][v])
				ma_sound_set_volume(&voices[s][v], SOUND_VOLUMES[s]);
	}

	ready = true;
	return true;
}

void Audio::shutdown()
{
	if (!ready)
		return;

	for (int s = 0; s < NUM_SOUNDS; s++)
		for (int v = 0; v < VOICES; v++)
			if (voiceLoaded[s][v])
			{
				ma_sound_uninit(&voices[s][v]);
				voiceLoaded[s][v] = false;
			}
	stopMusic();
	if (engineStarted)
	{
		ma_engine_uninit(&engine);
		engineStarted = false;
	}
	ready = false;
}

void Audio::play(Sound sound)
{
	if (!ready)
		return;

	for (int v = 0; v < VOICES; v++)
	{
		if (!voiceLoaded[sound][v] || ma_sound_is_playing(&voices[sound][v]))
			continue;

		ma_sound_seek_to_pcm_frame(&voices[sound][v], 0);
		ma_sound_start(&voices[sound][v]);
		return;
	}
}

void Audio::setMasterVolume(float volume)
{
	if (ready)
		ma_engine_set_volume(&engine, volume);
}

void Audio::playMusic(const char* path, bool loop)
{
	if (!ready)
		return;
	if (musicLoaded && currentMusic == path)
		return;

	stopMusic();
	if (ma_sound_init_from_file(&engine, path, MA_SOUND_FLAG_STREAM, NULL, NULL, &music) != MA_SUCCESS)
	{
		std::cout << "Could not load " << path << std::endl;
		return;
	}
	musicLoaded = true;
	currentMusic = path;
	ma_sound_set_looping(&music, loop ? MA_TRUE : MA_FALSE);
	ma_sound_set_volume(&music, musicVolume);
	ma_sound_start(&music);
}

void Audio::stopMusic()
{
	if (!musicLoaded)
		return;

	ma_sound_uninit(&music);
	musicLoaded = false;
	currentMusic.clear();
}

void Audio::setMusicVolume(float volume)
{
	musicVolume = volume;
	if (musicLoaded)
		ma_sound_set_volume(&music, musicVolume);
}

void Audio::stop(Sound sound)
{
	if (!ready)
		return;

	for (int v = 0; v < VOICES; v++)
		if (voiceLoaded[sound][v] && ma_sound_is_playing(&voices[sound][v]))
		{
			ma_sound_stop(&voices[sound][v]);
			ma_sound_seek_to_pcm_frame(&voices[sound][v], 0);
		}
}