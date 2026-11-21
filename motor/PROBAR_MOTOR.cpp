// Prueba independiente de GRIT NOVA, sin CONTACTO 2010 ni sus recursos.
#include "grit_nova_engine.hpp"
#include <array>
#include <cassert>
#include <iostream>
int main(){
 std::array<uint8_t,512> keys{};
 keys[gn::SC_W]=1;keys[gn::SC_D]=1;
 auto dir=gn::keyboard_direction(keys.data());
 assert(dir.first==1 && dir.second==-1);
 auto chars=gn::uppercase_utf8("Júpiter");
 assert(chars.size()==7 && chars[0]=='J');
 gn::Sound beep;beep.pcm={5000,5000,0,0};
 std::array<gn::AudioVoice,32> voices{};voices[0]={&beep,0,1.0f,10,0};
 const gn::Sound* track=nullptr;size_t pos=0;
 std::array<uint8_t,8> pcm{};
 gn::mix_mono_s16(pcm.data(),8,true,track,pos,false,false,voices,100,100,100);
 bool hasSound=false;for(auto byte:pcm)if(byte)hasSound=true;
 assert(hasSound);
 if(SDL_Init(0x20|0x2000)!=0){std::cerr<<SDL_GetError()<<"\n";return 2;}
 auto* win=SDL_CreateWindow("GRIT NOVA SMOKE",0,0,64,64,0);
 auto* ren=SDL_CreateRenderer(win,-1,1);
 if(!win||!ren){std::cerr<<SDL_GetError()<<"\n";return 3;}
 gn::draw_box(ren,4,4,16,16,{20,220,180,255});
 gn::draw_line(ren,0,0,15,15,{255,255,255,255});
 SDL_DestroyRenderer(ren);SDL_DestroyWindow(win);SDL_Quit();
 std::cout<<"GRIT NOVA ENGINE: INPUT, UTF8, AUDIO, SDL RENDER OK\n";
 return 0;
}
