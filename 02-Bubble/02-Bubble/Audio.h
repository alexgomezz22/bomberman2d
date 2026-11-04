#ifndef _AUDIO_INCLUDE
#define _AUDIO_INCLUDE


class Audio
{

private:
	Audio();

public:
	enum Sound { TNT_SET, TNT_FUSE, TNT_EXPLOSION, CLICK, HIT, LEVELUP, POWERUP,
					CREEPER_DEATH, ENDERMAN_DEATH, ENDERDRAGON_HIT, END, 
					ZOMBIE_DEATH, NUM_SOUNDS };

	static Audio& instance()
	{
		static Audio A;

		return A;
	}

	~Audio();

	bool init();
	void shutdown();
	void play(Sound sound);
	void setMasterVolume(float volume);
	void playMusic(const char* path, bool loop = true);
	void stopMusic();
	void setMusicVolume(float volume);
	void stop(Sound sound);


private:
	bool ready;
};


#endif // _AUDIO_INCLUDE