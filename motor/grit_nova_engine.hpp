// GRIT NOVA ENGINE 1.1 - Nucleo reutilizable (C++17 / SDL2 / libpng)
// Copyright (c) 2026 Tony C. Ware. Licencia MIT.
// API portable de bajo nivel; CONTACTO 2010 conserva sus reglas en fuentes/.
#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <array>
#include <utility>

// ABI publica SDL2: permite compilar contra libSDL2 sin paquetes de desarrollo.
extern "C" {
struct SDL_Window; struct SDL_Renderer; struct SDL_Texture; struct SDL_GameController; struct SDL_Joystick;
struct SDL_Rect {int x,y,w,h;};
struct SDL_Color {uint8_t r,g,b,a;};
typedef uint32_t SDL_AudioDeviceID;
typedef void (*SDL_AudioCallback)(void*,uint8_t*,int);
struct SDL_AudioSpec {int freq; uint16_t format; uint8_t channels,silence; uint16_t samples,padding; uint32_t size; SDL_AudioCallback callback; void* userdata;};
int SDL_Init(uint32_t); void SDL_Quit(); const char* SDL_GetError(); const char* SDL_GetCurrentAudioDriver();
SDL_Window* SDL_CreateWindow(const char*,int,int,int,int,uint32_t);
void SDL_DestroyWindow(SDL_Window*); int SDL_SetWindowFullscreen(SDL_Window*,uint32_t); void SDL_SetWindowSize(SDL_Window*,int,int);
SDL_Renderer* SDL_CreateRenderer(SDL_Window*,int,uint32_t); void SDL_DestroyRenderer(SDL_Renderer*);
int SDL_RenderSetLogicalSize(SDL_Renderer*,int,int); int SDL_GetRendererOutputSize(SDL_Renderer*,int*,int*);
int SDL_SetRenderDrawBlendMode(SDL_Renderer*,int);
int SDL_SetRenderDrawColor(SDL_Renderer*,uint8_t,uint8_t,uint8_t,uint8_t);
int SDL_RenderClear(SDL_Renderer*); void SDL_RenderPresent(SDL_Renderer*);
int SDL_RenderFillRect(SDL_Renderer*,const SDL_Rect*); int SDL_RenderDrawRect(SDL_Renderer*,const SDL_Rect*);
int SDL_RenderDrawLine(SDL_Renderer*,int,int,int,int);
SDL_Texture* SDL_CreateTexture(SDL_Renderer*,uint32_t,int,int,int);
void SDL_DestroyTexture(SDL_Texture*); int SDL_UpdateTexture(SDL_Texture*,const SDL_Rect*,const void*,int);
int SDL_SetTextureBlendMode(SDL_Texture*,int); int SDL_SetTextureScaleMode(SDL_Texture*,int); int SDL_SetTextureColorMod(SDL_Texture*,uint8_t,uint8_t,uint8_t);
int SDL_SetTextureAlphaMod(SDL_Texture*,uint8_t); int SDL_RenderCopy(SDL_Renderer*,SDL_Texture*,const SDL_Rect*,const SDL_Rect*);
int SDL_RenderSetClipRect(SDL_Renderer*,const SDL_Rect*);
int SDL_RenderCopyEx(SDL_Renderer*,SDL_Texture*,const SDL_Rect*,const SDL_Rect*,double,const void*,int);
int SDL_PollEvent(void*); uint32_t SDL_GetTicks(); void SDL_Delay(uint32_t);
const uint8_t* SDL_GetKeyboardState(int*); int SDL_ShowCursor(int);
int SDL_NumJoysticks(); int SDL_IsGameController(int); SDL_GameController* SDL_GameControllerOpen(int);
void SDL_GameControllerClose(SDL_GameController*); int SDL_GameControllerGetAttached(SDL_GameController*);
SDL_Joystick* SDL_GameControllerGetJoystick(SDL_GameController*); int32_t SDL_JoystickInstanceID(SDL_Joystick*);
int SDL_JoystickAttachVirtual(int,int,int,int); int SDL_JoystickDetachVirtual(int);
int16_t SDL_GameControllerGetAxis(SDL_GameController*,int);
uint8_t SDL_GameControllerGetButton(SDL_GameController*,int);
SDL_AudioDeviceID SDL_OpenAudioDevice(const char*,int,const SDL_AudioSpec*,SDL_AudioSpec*,int);
void SDL_CloseAudioDevice(SDL_AudioDeviceID); void SDL_PauseAudioDevice(SDL_AudioDeviceID,int);
void SDL_LockAudioDevice(SDL_AudioDeviceID); void SDL_UnlockAudioDevice(SDL_AudioDeviceID);
int SDL_RenderReadPixels(SDL_Renderer*,const SDL_Rect*,uint32_t,void*,int);
}
namespace gn {
constexpr uint32_t AUDIO_S16LSB=0x8010, SDL_PIXELFORMAT_ABGR8888=0x16762004;
constexpr int SC_A=4,SC_D=7,SC_S=22,SC_W=26;
constexpr int SC_RIGHT=79,SC_LEFT=80,SC_DOWN=81,SC_UP=82;
struct RGBA {int r,g,b,a=255;};
struct Texture {SDL_Texture* t=nullptr;int w=0,h=0;};
struct Sound {std::vector<int16_t> pcm;int channels=1;};
struct AudioVoice {const Sound* sample=nullptr;size_t position=0;float gain=0.0f;int priority=0;uint32_t started=0;float pan=0.f;};
struct LoadedPng {SDL_Texture* texture=nullptr;int width=0,height=0;std::vector<uint8_t> pixels;std::string error;};
LoadedPng load_png_texture(SDL_Renderer*,const std::string& filename);
std::pair<float,float> keyboard_direction(const uint8_t* state);
void close_controller(SDL_GameController*& pad,int32_t& instance);
bool open_controller(int deviceIndex,SDL_GameController*& pad,int32_t& instance);
void scan_controllers(SDL_GameController*& pad,int32_t& instance,uint32_t& lastScan);
void mix_mono_s16(uint8_t* stream,int bytes,bool enabled,const Sound*& track,size_t& pos,
                  bool paused,bool loop,std::array<AudioVoice,32>& voices,
                  int master,int music,int effects);
void mix_stereo_s16(uint8_t* stream,int bytes,bool enabled,bool stereo,const Sound*& track,size_t& pos,
                    bool paused,bool loop,std::array<AudioVoice,32>& voices,
                    int master,int music,int effects);
void draw_box(SDL_Renderer*,int x,int y,int w,int h,RGBA fill,RGBA outline={0,0,0,0});
void draw_line(SDL_Renderer*,int x,int y,int x2,int y2,RGBA color);
std::vector<uint32_t> uppercase_utf8(const std::string&);
} // namespace gn
