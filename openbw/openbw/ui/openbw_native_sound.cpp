#include "native_sound.h"

#include <memory>

namespace native_sound {

bool initialized = false;

int frequency = 0;
int channels = 64;

#ifndef OPENBW_NO_SDL_MIXER

void init() {
	if (initialized) return;
	initialized = true;
	Mix_Init(0);
	int freq = frequency;
	if (freq == 0) {
		freq = MIX_DEFAULT_FREQUENCY;
	}
	Mix_OpenAudio(freq, MIX_DEFAULT_FORMAT, 2, 1024);
	Mix_AllocateChannels(channels);
}

struct sdl_sound: sound {
	Mix_Chunk* c = nullptr;
	virtual ~sdl_sound() override {
		if (c) Mix_FreeChunk(c);
	}
};

void play(int channel, sound* arg_s, int volume, int pan) {
	if (!initialized) init();
	sdl_sound* s = (sdl_sound*)arg_s;
	if (!s || !s->c) return;
	int c = Mix_PlayChannel(channel, s->c, 0);
	if (c != -1) {
		Mix_Volume(c, volume);
		//int left = 254;
		//int right = 254;
		//if (pan < 0) right += pan;
		//else left -= pan;
		//Mix_SetPanning(c, left, right);
	}
}

bool is_playing(int channel) {
	return Mix_Playing(channel) != 0;
}

void stop(int channel) {
	Mix_HaltChannel(channel);
}

void set_volume(int channel, int volume) {
	Mix_Volume(channel, volume);
}

std::unique_ptr<sound> load_wav(const void* data, size_t size) {
	if (!initialized) init();
	Mix_Chunk* c = Mix_LoadWAV_RW(SDL_RWFromConstMem(data, (int)size), 1);
	if (!c) return {};
	auto r = std::make_unique<sdl_sound>();
	r->c = c;
	return std::unique_ptr<sound>(r.release());
}

#else

void init() {
}

struct sdl_sound: sound {
	virtual ~sdl_sound() override {}
};

void play(int channel, sound* arg_s, int volume, int pan) {
}

bool is_playing(int channel) {
	return false;
}

void stop(int channel) {
}

void set_volume(int channel, int volume) {
}

std::unique_ptr<sound> load_wav(const void* data, size_t size) {
	return nullptr;
}

#endif

}
