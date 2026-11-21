// GRIT NOVA ENGINE 1.1 - Subsistemas extraidos del motor activo de CONTACTO 2010.
#include "grit_nova_engine.hpp"
#include <png.h>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <iostream>
namespace gn {
std::pair<float,float> keyboard_direction(const uint8_t* keys){
 if(!keys)return {0.f,0.f};
 auto k=[&](int scan){return keys[scan]!=0;};
 float dx=float(k(SC_RIGHT)||k(SC_D))-float(k(SC_LEFT)||k(SC_A));
 float dy=float(k(SC_DOWN)||k(SC_S))-float(k(SC_UP)||k(SC_W));
 return {dx,dy};
}


void close_controller(SDL_GameController*& pad,int32_t& instance){
 if(pad){SDL_GameControllerClose(pad);pad=nullptr;}instance=-1;
}
bool open_controller(int deviceIndex,SDL_GameController*& pad,int32_t& instance){
 if(pad||deviceIndex<0||!SDL_IsGameController(deviceIndex))return false;
 SDL_GameController* opened=SDL_GameControllerOpen(deviceIndex);
 if(!opened){std::cerr<<"Mando: no se pudo conectar índice "<<deviceIndex<<": "<<SDL_GetError()<<"\n";return false;}
 SDL_Joystick* joy=SDL_GameControllerGetJoystick(opened);
 int32_t id=joy?SDL_JoystickInstanceID(joy):-1;
 if(id<0){SDL_GameControllerClose(opened);return false;}
 pad=opened;instance=id;
 std::cout<<"Mando conectado: índice="<<deviceIndex<<" instancia="<<instance<<"\n";
 return true;
}
void scan_controllers(SDL_GameController*& pad,int32_t& instance,uint32_t& lastScan){
 if(pad&&SDL_GameControllerGetAttached(pad))return;
 if(pad)close_controller(pad,instance);
 for(int i=0;i<SDL_NumJoysticks();++i)if(open_controller(i,pad,instance))break;
 lastScan=SDL_GetTicks();
}
LoadedPng load_png_texture(SDL_Renderer* renderer,const std::string& path){
 LoadedPng result;
 png_image im{};im.version=PNG_IMAGE_VERSION;
 if(!png_image_begin_read_from_file(&im,path.c_str())){result.error=im.message;return result;}
 im.format=PNG_FORMAT_RGBA;
 result.pixels.resize(PNG_IMAGE_SIZE(im));
 if(!png_image_finish_read(&im,nullptr,result.pixels.data(),0,nullptr)){
   result.error=im.message;png_image_free(&im);return result;
 }
 result.width=int(im.width);result.height=int(im.height);
 result.texture=SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ABGR8888,1,result.width,result.height);
 if(!result.texture){result.error=SDL_GetError();png_image_free(&im);return result;}
 SDL_UpdateTexture(result.texture,nullptr,result.pixels.data(),result.width*4);
 SDL_SetTextureBlendMode(result.texture,1);
 png_image_free(&im);return result;
}
void draw_box(SDL_Renderer* r,int x,int y,int w,int h,RGBA fill,RGBA outline){
 SDL_Rect rect{x,y,w,h};
 SDL_SetRenderDrawColor(r,fill.r,fill.g,fill.b,fill.a);SDL_RenderFillRect(r,&rect);
 if(outline.a){SDL_SetRenderDrawColor(r,outline.r,outline.g,outline.b,outline.a);SDL_RenderDrawRect(r,&rect);}
}
void draw_line(SDL_Renderer* r,int x,int y,int x2,int y2,RGBA c){
 SDL_SetRenderDrawColor(r,c.r,c.g,c.b,c.a);SDL_RenderDrawLine(r,x,y,x2,y2);
}
std::vector<uint32_t> uppercase_utf8(const std::string&s){
  std::vector<uint32_t>out;
  for(size_t i=0;i<s.size();){uint8_t a=uint8_t(s[i++]);uint32_t v=0;
   if(a<128)v=a;
   else if((a&0xe0)==0xc0 && i<s.size()){uint8_t b=uint8_t(s[i++]);v=((a&31)<<6)|(b&63);}
   else if((a&0xf0)==0xe0 && i+1<s.size()){uint8_t b=uint8_t(s[i++]);uint8_t c=uint8_t(s[i++]);v=((a&15)<<12)|((b&63)<<6)|(c&63);}
   else v='?';
   if(v>='a'&&v<='z')v-=32;
   out.push_back(v);
  }return out;
 }



void mix_stereo_s16(uint8_t* stream,int bytes,bool enabled,bool stereo,const Sound*& track,size_t& pos,
                    bool paused,bool loop,std::array<AudioVoice,32>& voices,
                    int master,int music,int effects){
  std::memset(stream,0,size_t(bytes));
  if(!enabled)return;
  auto* out=reinterpret_cast<int16_t*>(stream);
  const int frames=bytes/4; // S16 intercalado: izquierda, derecha
  const float musicGain=.37f*float(master*music)/10000.f;
  const float fxGain=float(master*effects)/10000.f;
  for(int i=0;i<frames;++i){
    float left=0.f,right=0.f;
    if(track && !track->pcm.empty() && !paused){
      if(pos>=track->pcm.size()){
        if(loop)pos=0; else track=nullptr;
      }
      if(track){
        const float value=float(track->pcm[pos++])*musicGain;
        left+=value;right+=value; // La fuente musical actual es mono.
      }
    }
    for(auto &v:voices){
      if(!v.sample)continue;
      if(v.position>=v.sample->pcm.size()){v.sample=nullptr;continue;}
      const float sample=float(v.sample->pcm[v.position++])*v.gain*fxGain;
      const float pan=stereo?std::clamp(v.pan,-1.f,1.f):0.f;
      // Centro a nivel original; atenuar exclusivamente el canal lejano.
      left+=sample*(pan>0.f?1.f-.78f*pan:1.f);
      right+=sample*(pan<0.f?1.f+.78f*pan:1.f);
      if(v.position>=v.sample->pcm.size())v.sample=nullptr;
    }
    const auto limit=[](float v){
      if(v>27500.f)v=27500.f+(v-27500.f)*.14f;
      else if(v< -27500.f)v=-27500.f+(v+27500.f)*.14f;
      return int16_t(std::clamp(v,-32768.f,32767.f));
    };
    out[i*2]=limit(left);
    out[i*2+1]=limit(right);
  }
}

void mix_mono_s16(uint8_t* stream,int bytes,bool enabled,const Sound*& track,size_t& pos,
                  bool paused,bool loop,std::array<AudioVoice,32>& voices,
                  int master,int music,int effects){
   std::memset(stream,0,size_t(bytes));
   if(!enabled)return;
   int16_t* out=reinterpret_cast<int16_t*>(stream);
   int count=bytes/2;
   for(int i=0;i<count;++i){
     float mix=0;
     if(track && !track->pcm.empty() && !paused){
       if(pos>=track->pcm.size()){
         // Las músicas de título y GAME OVER terminan aquí: jamás se repiten.
         if(loop)pos=0; else track=nullptr;
       }
       if(track)mix+=float(track->pcm[pos++])*0.37f*float(master*music)/10000.f;
       // El resto de la banda sonora continúa en bucle, como antes.
     }
     for(auto &v:voices){
       if(!v.sample)continue;
       if(v.position>=v.sample->pcm.size()){v.sample=nullptr;continue;}
       mix+=float(v.sample->pcm[v.position++])*v.gain*float(master*effects)/10000.f;
       if(v.position>=v.sample->pcm.size())v.sample=nullptr;
     }
     // Limitador suave para evitar saturacion en combates con muchas voces.
     if(mix>27500.f)mix=27500.f+(mix-27500.f)*0.14f;
     else if(mix< -27500.f)mix=-27500.f+(mix+27500.f)*0.14f;
     out[i]=int16_t(std::clamp(mix,-32768.f,32767.f));
   }

}
} // namespace gn
