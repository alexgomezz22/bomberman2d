#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <iostream>
#include "Audio.h"


namespace
{
	const int VOICES = 8;

	const char* SOUND_PATHS[Audio::NUM_SOUNDS] = {
		"sounds/tnt_set.wav",
		"sounds/tnt_pss.wav",
		"sounds/tnt_explosion.wav"
	};

	ma_engine engine;
	ma_sound voices[Audio::NUM_SOUNDS][VOICES];
	bool voiceLoaded[Audio::NUM_SOUNDS][VOICES];
	bool engineStarted = false;
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