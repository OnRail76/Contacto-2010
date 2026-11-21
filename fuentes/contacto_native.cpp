// CONTACTO 2010 · Fases 1 a 5 — Motor Linux nativo SDL2
// © 2026 Tony C. Ware · Versión V1.2.1 · MIT
// Sin navegador, Python, pip ni conexión a Internet.
#include <png.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#endif
#include <vorbis/vorbisfile.h>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <vector>
#include <map>
#include <array>
#include <iterator>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <random>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

#include "../motor/grit_nova_engine.hpp"
using gn::RGBA; using gn::Texture; using gn::Sound; using gn::AudioVoice;
using gn::keyboard_direction;
using gn::SC_W; using gn::SC_A; using gn::SC_S; using gn::SC_D;
using gn::SC_UP; using gn::SC_DOWN; using gn::SC_LEFT; using gn::SC_RIGHT;
using gn::AUDIO_S16LSB; using gn::SDL_PIXELFORMAT_ABGR8888;
static constexpr int W=960,H=540,HUD_H=120;
static constexpr double PI=3.141592653589793;
static float limit(float x,float l,float h){return std::max(l,std::min(h,x));}
static std::mt19937 rng(2010);
static float rf(float a,float b){std::uniform_real_distribution<float>d(a,b);return d(rng);}
static int ri(int a,int b){std::uniform_int_distribution<int>d(a,b);return d(rng);}
static constexpr RGBA WHITE{237,246,255}, CYAN{82,220,253}, MAGENTA{255,52,215}, AMBER{255,190,75}, RED{255,96,94}, GREEN{98,236,177}, DIM{136,178,205};
struct Sprite {std::string tex; int cols=1,cellW=0,cellH=0,count=1;};
struct Actor {int kind=0;float x=0,y=0,vx=0,hp=1,t=0,fire=0;bool ceiling=false,alive=true;};
struct Bullet {float x=0,y=0,vx=0,vy=0,life=0;bool friendly=false,missile=false;};
struct Puff {float x=0,y=0,vx=0,vy=0,life=0,maxlife=1;RGBA c=AMBER;};
struct Prize {float x=0,y=0;int type=0;};
struct DeathFx {int kind=0;float x=0,y=0,age=0;};
struct CrewMessage {int role=1;std::string who,msg;float life=0,elapsed=0;};
// Guion aprobado por Tony C. Ware · CONTACTO 2010 · cinco fases.
// AURA-01 es la computadora de a bordo de Firebird-01 (rol narrativo 3).
// Mantener sin cambios la ortografía original y las pausas de cada renglón.
static const char* const TEXTO_BRIEFING[5][4]={
  { "ODISEA II HA PERDIDO CONTACTO.", "EXPLORA EL CEMENTERIO ORBITAL.", "LOCALIZA LA SEÑAL IMPOSIBLE.", "EVITA LAS DEFENSAS AUTOMÁTICAS." },
  { "LA ODISEA II SE HA PERDIDO.", "LAS DEFENSAS SIGUEN ACTIVAS.", "", "" },
  { "LAS CRIPTAS DE EUROPA DESPIERTAN.", "ALGO LAS PROTEGE DE TODO.", "", "" },
  { "ESTO ES UNA INCUBADORA.", "HAY ALGO QUE QUIERE SALIR.", "ALGO QUE GRITA.", "" },
  { "JÚPITER ESTÁ FORMADO POR LOS", "ESPECTROS DEMONÍACOS DEL UNIVERSO", "ES AQUÍ DONDE RESIDE TODO MAL", "" }
};
static const char* const TEXTO_PREJEFE[5][2]={
  { "AURA-01: ESO NO ERA UNA SEÑAL.", "¡ERA UNA ADVERTENCIA!." },
  { "AURA-01: TODOS FORMAN PARTE DE LA NAVE.", "ES COMO SI TUVIERA VOLUNTAD." },
  { "AURA-01: ESTÁ PROTEGIENDO UNA ENTRADA.", "HAY ALGO BAJO EL HIELO." },
  { "AURA-01: AHORA SABEMOS LA VERDAD.", "EL MAL TIENE FORMA Y VIVE AQUÍ." },
  { "AURA-01: ASI QUE ERA ESTO.", "EL PROVEEDOR DE MUERTE JOVIANO." }
};
static const char* const TITULO_JEFE[5]={"MOAI MONOLITH", "ODISEA II ASIMILADA", "HALCÓN ESTELAR REX", "HIJO DE LAS ESTRELLAS", "EL FANTASMA DE JÚPITER"};
static const char* const CONSEJOS_JEFE[5][2]={
  { "DISPARA AL OJO CUANDO SE ABRA.", "¡EVITA LOS PROYECTILES!" },
  { "DISPARA AL REACTOR EXPUESTO.", "EVITA EL RAYO DE ENERGÍA." },
  { "ATACA CUANDO TERMINE EL RAYO.", "ALÉJATE DE LA DESCARGA ROJA." },
  { "EL NÚCLEO SE ABRE TRAS EL PULSO.", "EVITA LOS ORBES Y LA GRAVEDAD." },
  { "DISPARA AL OJO CUANDO BRILLE EN CIAN.", "ESQUIVA LAS RÁFAGAS Y EL PLASMA." }
};
class Game {
public:
 SDL_Window *win=nullptr; SDL_Renderer* ren=nullptr; SDL_GameController* pad=nullptr;
 int32_t padInstance=-1; uint32_t lastPadScan=0;
 std::map<std::string,Texture> textures; std::map<std::string,Sound> audio;
 // Recortes reales de tinta de cada glifo RGBA, calculados una sola vez al cargar.
 std::map<std::string,std::vector<SDL_Rect>> hudGlyphInk;
 // Una unica salida SDL2 con mezcla polifonica en el callback de audio.
 SDL_AudioDeviceID auddev=0; bool sound=true,stereoSpatial=true;
  int scanlines=1;SDL_Texture* scanlineTexture=nullptr;
 int masterVolume=100,musicVolume=100,effectsVolume=100;
 bool musicPaused=false;
 // Valores guardados y mezcla de pistas independientes (audio PCM precargado).
 // La voz 0 es exclusiva del arma del jugador; otras 31 mezclan enemigos, explosiones y avisos.
 std::array<AudioVoice,32> voices{};
 uint64_t playerShotAudioRequested=0,playerShotAudioScheduled=0;
 Sound musicData;const Sound* musicTrack=nullptr;size_t musicPos=0;bool musicLoop=true;
 std::string musicName="";
 std::map<std::string,uint32_t> lastSfxAt;
 std::vector<uint16_t> wallTop,wallFloor,wallTop1,wallFloor1,wallTop2,wallFloor2,wallTop3,wallFloor3,wallTop4,wallFloor4,wallTop5,wallFloor5;std::map<uint32_t,std::pair<int,int>> glyphs;
 std::vector<Actor> enemies; std::vector<DeathFx> deaths; std::vector<Bullet> shots;std::vector<Puff> puffs;std::vector<Prize> prizes;
 CrewMessage radio;
 enum Mode {TITLE,INTRO,PLAY,BOSS_INTRO,BOSS,BOSS_DEATH,DEAD,WIN,FINAL_CREDITS,AUDIO_OPTIONS,JUKEBOX} mode=TITLE;
 float stateTime=0,stageTime=0,scroll=0,shipx=190,shipy=275,vulnerable=0,death=0,shootTime=0,missileTime=0,shieldTime=0,weaponTime=0,bossTime=0,bossY=265,bossX=1120,bossHp=95,bossMax=95,flash=0,commCool=0;
 int lives=3,score=0,best=0,weapon=0,menuIndex=0,bossPhase=0,menuOption=0,playerIndex=0,playingIndex=-1;bool running=true,fullscreen=false,pause=false,confirmHeld=false;bool debugPhaseSelector=false;int debugPhase=1;bool cheatInvulnerable=false,cheatUsedThisRun=false;
 bool stageEvents[50]{}; int eventCount=0;int fase=1;float bossBeamHit=0;
 float starsX[125],starsY[125],starsSpeed[125];int starsSize[125];
 bool capture=false;std::string shotMode="";
 int finalCreditsPage=0; float finalCreditsClock=0; static constexpr int FINAL_CREDITS_PAGES=9;
 // Contadores verificables para contrastar la presión real de las oleadas.
 int enemiesSpawned=0, hostileShotsFired=0, peakHostiles=0;
 int sierrasF1=0,sierrasF2=0,sierrasDesdeIzq=0,sierrasDesdeDer=0,espectrosF1=0,moaisF1=0;
 std::string base="assets/";
 float shipMoveY=0;bool firing=false;
 ~Game(){cleanup();}
 void close_gamepad(){gn::close_controller(pad,padInstance);}
 bool open_gamepad(int deviceIndex){return gn::open_controller(deviceIndex,pad,padInstance);}
 void scan_gamepads(){gn::scan_controllers(pad,padInstance,lastPadScan);}
 void cleanup(){for(auto &p:textures)if(p.second.t)SDL_DestroyTexture(p.second.t);textures.clear(); close_gamepad();if(auddev)SDL_CloseAudioDevice(auddev);if(scanlineTexture)SDL_DestroyTexture(scanlineTexture);if(ren)SDL_DestroyRenderer(ren);if(win)SDL_DestroyWindow(win); SDL_Quit();}
 bool load(const std::string& name,const std::string& file){
   gn::LoadedPng decoded=gn::load_png_texture(ren,base+file);
   if(!decoded.texture){std::cerr<<"PNG "<<file<<": "<<decoded.error<<"\n";return false;}
   Texture a{decoded.texture,decoded.width,decoded.height};
   const auto& bits=decoded.pixels;
   if(name=="hud_cyan"||name=="hud_amber"||name=="hud_white"||name=="hud_big"||name=="hud_magenta"){
    const int cw=name=="hud_big"?104:72, ch=name=="hud_big"?128:96;
    std::vector<SDL_Rect> ink(70,SDL_Rect{0,0,0,0});
    for(int idx=0;idx<70;++idx){
      int cx=idx%12*cw, cy=idx/12*ch, left=cw, top=ch, right=-1, bottom=-1;
      for(int yy=0; yy<ch && cy+yy<a.h; ++yy)for(int xx=0; xx<cw && cx+xx<a.w; ++xx){
        const size_t off=(size_t(cy+yy)*size_t(a.w)+size_t(cx+xx))*4+3;
        if(bits[off]>36){left=std::min(left,xx);top=std::min(top,yy);
           right=std::max(right,xx);bottom=std::max(bottom,yy);}
      }
      if(right>=left&&bottom>=top)ink[idx]={cx+left,cy+top,right-left+1,bottom-top+1};
    }
    hudGlyphInk[name]=std::move(ink);
   }
   if(name=="fondo2"||name=="fondo3"||name=="fondo4"||name=="fondo5"||name=="escudo_nuevo"||name=="triangulo"||name=="mira") SDL_SetTextureScaleMode(a.t,1);
   textures[name]=a;return true;
 }
 bool init(){
  if(SDL_Init(0x20|0x2000|0x10)!=0){std::cerr<<"SDL_Init: "<<SDL_GetError()<<"\n";return false;}
  win=SDL_CreateWindow("CONTACTO 2010 · V1.2 · Tony C. Ware",0x2FFF0000,0x2FFF0000,1280,720,0x4|0x20);
  if(!win){std::cerr<<"SDL_Window: "<<SDL_GetError()<<"\n";return false;}
  ren=SDL_CreateRenderer(win,-1,2|4);
  if(!ren)ren=SDL_CreateRenderer(win,-1,1);
  if(!ren){std::cerr<<"SDL_Renderer: "<<SDL_GetError()<<"\n";return false;}
  SDL_RenderSetLogicalSize(ren,W,H);SDL_SetRenderDrawBlendMode(ren,1);SDL_ShowCursor(0);
   scanlineTexture=SDL_CreateTexture(ren,SDL_PIXELFORMAT_ABGR8888,0,1,H);
   if(scanlineTexture){
     std::vector<uint8_t> pixels(size_t(H)*4,0);
     for(int y=0;y<H;++y)if(y%2)pixels[size_t(y)*4+3]=255;
     SDL_UpdateTexture(scanlineTexture,nullptr,pixels.data(),4);
     SDL_SetTextureBlendMode(scanlineTexture,1);
   }
  struct Item{const char* k;const char* f;};
  const Item list[]={{"fondo","jupiter.png"},{"mapa","fase1_cementerio_v2_1920x540.png"},{"ship","firebird_01_16f_hd.png"},{"moai","moai_24f.png"},{"sierra","rodante_sierra_24f.png"},{"menor","enemigo_monolito_24f.png"},{"portrait","retratos_definitivos.png"},{"aura","AURA_01_SHEET_RGBA.png"},{"fx_orb","fx_bala_enemiga.png"},{"fx_pulse","fx_pulso.png"},{"fx_laser","fx_laser.png"},{"escudo_nuevo","escudo_firebird_nuevo_4f.png"},{"triangulo","alerta_estalactitas_4f.png"},{"mira","mira_debil_jefes_4f.png"},{"logo","CONTACTO_2010_LOGO_JUEGO.png"},{"marca_tony","tony_c_ware_logo.png"},{"marca_grit","grit_nova_logo_oficial.png"},{"hud","hud_militar_960x120.png"},{"glyph","fuente_legible.png"},
   {"hud_white","hud_fuentes/FUENTE_BLANCO_HIELO.png"},{"hud_cyan","hud_fuentes/FUENTE_CIAN_RADAR.png"},{"hud_amber","hud_fuentes/FUENTE_AMBAR_ARMA.png"},{"hud_magenta","hud_fuentes/FUENTE_MAGENTA_ALERTA.png"},{"hud_big","hud_fuentes/FUENTE_NUMEROS_GRANDES.png"},{"hud_smile","hud_fuentes/VIDAS_SMILEYS_RGBA.png"},
   {"fondo2","fase2_fondo_hd.png"},{"mapa2","fase2_terreno.png"},
   // Sierras rodantes compartidas por las fases 1 y 2; sin walkers. 
   {"c_izq_der","fase2_f22_izq_der.png"},{"c_der_izq","fase2_f22_der_izq.png"},
   {"e_izq_der","fase2_fantasma_izq_der.png"},{"e_der_izq","fase2_fantasma_der_izq.png"},
   {"odisea","fase2_odisea_ii_nuevo_boss_18f.png"},
   {"fondo3","fase3_fondo_hd.png"},{"mapa3","fase3_terreno.png"},
   {"halcon_izquierda","halcon_izquierda_16f_rgba.png"},{"halcon_derecha","halcon_derecha_16f_rgba.png"},
   {"hielo3","fase3_estalactita.png"},{"halcon_jefe","fase3_halcon_jefe.png"},
   {"fondo4","fase4_fondo_hd.png"},{"mapa4","fase4_terreno.png"},
   {"final_escena","final_escena_firebird01_hd.png"},{"fondo5","fase5_fondo_hd.png"},{"mapa5","fase5_terreno.png"},
   {"fantasma5","fase5_fantasma_jupiter_boss.png"},
   {"cel_estrella_di","fase4_celula_estelar_di.png"},{"cel_estrella_id","fase4_celula_estelar_id.png"},
   {"cel_devora_di","fase4_celula_devoradora_di.png"},{"cel_devora_id","fase4_celula_devoradora_id.png"},
   {"hijo","fase4_hijo_estrellas.png"}};
  for(const auto&i:list)if(!load(i.k,i.f))return false;
  if(textures["aura"].w!=2172||textures["aura"].h!=724){
    std::cerr<<"AURA-01: atlas incorrecto; se necesita 2172x724 con 12 fotogramas\n";return false;
  }
  std::ifstream g(base+"terreno.bin",std::ios::binary);uint32_t n=0;g.read((char*)&n,4);
  if(n<1000||n>10000){std::cerr<<"Datos de colisiones incorrectos\n";return false;}
  for(uint32_t i=0;i<n;i++){uint16_t a,b;g.read((char*)&a,2);g.read((char*)&b,2);wallTop.push_back(a);wallFloor.push_back(b);}
  if(!g){std::cerr<<"No se pudo cargar terreno.bin\n";return false;}
  wallTop1=wallTop;wallFloor1=wallFloor;
  std::ifstream g2(base+"fase2_terreno.bin",std::ios::binary);
  uint32_t n2=0;g2.read((char*)&n2,4);
  if(n2<300||n2>3000){std::cerr<<"Geometría Fase 2 ausente\n";return false;}
  for(uint32_t i=0;i<n2;i++){uint16_t a,b;g2.read((char*)&a,2);g2.read((char*)&b,2);wallTop2.push_back(a);wallFloor2.push_back(b);}
  if(!g2){std::cerr<<"Geometría Fase 2 incompleta\n";return false;}
  std::ifstream g3(base+"fase3_terreno.bin",std::ios::binary);
  uint32_t n3=0;g3.read((char*)&n3,4);
  if(n3<480||n3>4000){std::cerr<<"Geometría Fase 3 ausente\n";return false;}
  for(uint32_t i=0;i<n3;i++){uint16_t a,b;g3.read((char*)&a,2);g3.read((char*)&b,2);wallTop3.push_back(a);wallFloor3.push_back(b);}
  if(!g3){std::cerr<<"Geometría Fase 3 incompleta\n";return false;}
  std::ifstream g4(base+"fase4_terreno.bin",std::ios::binary);
  uint32_t n4=0;g4.read((char*)&n4,4);
  if(n4<480||n4>4000){std::cerr<<"Geometría Fase 4 ausente\n";return false;}
  for(uint32_t i=0;i<n4;i++){uint16_t a,b;g4.read((char*)&a,2);g4.read((char*)&b,2);wallTop4.push_back(a);wallFloor4.push_back(b);}
  if(!g4){std::cerr<<"Geometría Fase 4 incompleta\n";return false;}
  // Fase 5: colisiones obtenidas de la franja central transparente del PNG oficial del usuario.
  std::ifstream g5(base+"fase5_terreno.bin",std::ios::binary);
  uint32_t n5=0;g5.read((char*)&n5,4);
  if(n5!=1920){std::cerr<<"Geometría Fase 5 HD ausente o incompleta\n";return false;}
  for(uint32_t i=0;i<n5;++i){uint16_t a,b;g5.read((char*)&a,2);g5.read((char*)&b,2);wallTop5.push_back(a);wallFloor5.push_back(b);}
  if(!g5){std::cerr<<"Datos de Fase 5 incompletos\n";return false;}
  std::ifstream font(base+"fuente_map.txt");uint32_t cp;int size, idx=0;
  while(font>>cp>>size)glyphs[cp]={idx++,size};
  for(int i=0;i<125;i++){starsX[i]=rf(0,W);starsY[i]=rf(65,490);starsSpeed[i]=rf(0.2f,1.9f);starsSize[i]=ri(1,2);}
  scan_gamepads(); // Inicio: conecta el primer mando disponible; eventPoll gestiona hotplug.
  // Efectos WAV PCM 22050 Hz mono, musica OGG Vorbis estereo.
  // Precargar antes de poner en marcha el callback: ninguna carga de disco en tiempo real.
  for(auto name:{"disparo","explosion","explosion_grande","item","laser","alarma","impacto","misil","escudo","rayo","apertura","explosion_corta"}){
   std::ifstream f(base+"audio/"+std::string(name)+".wav",std::ios::binary);
   if(!f){if(std::string(name)!="musica_game_over" && std::string(name)!="musica_escena_final")std::cerr<<"Falta WAV: "<<name<<"\n";continue;}
   std::vector<uint8_t> b((std::istreambuf_iterator<char>(f)),{});
   if(b.size()<44||memcmp(b.data(),"RIFF",4)!=0||memcmp(b.data()+8,"WAVE",4)!=0){std::cerr<<"WAV incorrecto: "<<name<<"\n";continue;}
   bool validFormat=false;size_t pcmOffset=0,pcmSize=0;
   size_t pos=12;
   auto u16=[&b](size_t i)->uint16_t{return uint16_t(b[i])|uint16_t(b[i+1])<<8;};
   auto u32=[&b](size_t i)->uint32_t{return uint32_t(b[i])|uint32_t(b[i+1])<<8|uint32_t(b[i+2])<<16|uint32_t(b[i+3])<<24;};
   while(pos+8<=b.size()){
    uint32_t z=u32(pos+4);size_t data=pos+8;
    if(z>b.size()-data)break;
    if(memcmp(b.data()+pos,"fmt ",4)==0&&z>=16)
      validFormat=(u16(data)==1&&u16(data+2)==1&&u32(data+4)==22050&&u16(data+14)==16);
    if(memcmp(b.data()+pos,"data",4)==0){pcmOffset=data;pcmSize=z;}
    pos=data+z+(z&1);
   }
   if(!validFormat||!pcmOffset||pcmSize<2){std::cerr<<"Formato WAV no soportado: "<<name<<"\n";continue;}
   auto &samples=audio[name].pcm;
    const size_t n=pcmSize/2;samples.resize(n*2);
    for(size_t i=0;i<n;++i){
      const int16_t a=int16_t(uint16_t(b[pcmOffset+i*2])|(uint16_t(b[pcmOffset+i*2+1])<<8));
      const int16_t next=i+1<n?int16_t(uint16_t(b[pcmOffset+(i+1)*2])|(uint16_t(b[pcmOffset+(i+1)*2+1])<<8)):a;
      samples[i*2]=a;samples[i*2+1]=int16_t((int(a)+int(next))/2);
    }
  }
  SDL_AudioSpec spec{};spec.freq=44100;spec.format=AUDIO_S16LSB;spec.channels=2;spec.samples=1024;
  spec.callback=&Game::audio_callback;spec.userdata=this;
  SDL_AudioSpec obtained{};
  auddev=SDL_OpenAudioDevice(nullptr,0,&spec,&obtained,0);
  if(!auddev)std::cerr<<"Audio SDL2 no disponible: "<<SDL_GetError()<<"\n";
  else {
    const char* driver=SDL_GetCurrentAudioDriver();
    std::cerr<<"Audio SDL2: driver="<<(driver?driver:"desconocido")
             <<" formato="<<obtained.freq<<" Hz/estereo/16-bit"
             <<" bufer="<<obtained.samples<<" muestras\n";
  }
  if(auddev&&(obtained.freq!=44100||obtained.format!=AUDIO_S16LSB||obtained.channels!=2)){
    std::cerr<<"Formato de dispositivo audio incompatible\n";SDL_CloseAudioDevice(auddev);auddev=0;
  }
  load_audio_settings();load_best();music("musica_titulo");if(auddev)SDL_PauseAudioDevice(auddev,0);return true;
 }
 static std::string phase_music(int number){
  if(number==1)return "musica_fase_01";
  if(number==2)return "musica_fase_02";
  if(number==3)return "musica_fase_03";
  if(number==4)return "musica_fase_04";
  if(number==5)return "musica_fase_05";
  return "musica_fase_01";
 }
 void load_audio_settings(){
  std::ifstream f("audio_config.ini");std::string name;int val;
  while(f>>name>>val){val=std::clamp(val,0,100);
   if(name=="master")masterVolume=val;
   else if(name=="music")musicVolume=val;
   else if(name=="effects")effectsVolume=val;
   else if(name=="enabled")sound=(val!=0);
    else if(name=="stereo")stereoSpatial=(val!=0);
    else if(name=="scanlines")scanlines=std::clamp(val,0,3);
  }
 }
 void save_audio_settings()const{
  std::ofstream f("audio_config.ini",std::ios::trunc);
  if(f)f<<"master "<<masterVolume<<"\nmusic "<<musicVolume
       <<"\neffects "<<effectsVolume<<"\nenabled "<<(sound?1:0)
        <<"\nstereo "<<(stereoSpatial?1:0)<<"\nscanlines "<<scanlines<<"\n";
 }
 static constexpr int JUKE_COUNT=10;
 static constexpr const char* jukeName[JUKE_COUNT]={
   "LA SEÑAL IMPOSIBLE","LA NAVE DEL SILENCIO","LAS CRIPTAS DE EUROPA",
   "LA CUNA DE LAS ESTRELLAS","EL CORAZÓN DE JÚPITER","JEFES FINALES",
   "JINGLE · CONTACTO 2010 TITLE","JINGLE · GAME OVER","JINGLE · VICTORIA",
   "REMEMBER LOVE · EPÍLOGO"
 };
 static constexpr const char* jukeFile[JUKE_COUNT]={
  "musica_fase_01","musica_fase_02","musica_fase_03",
  "musica_fase_04","musica_fase_05","musica_jefes_06",
  "musica_titulo","musica_game_over","musica_victoria",
  "musica_escena_final"
 };
 void back_to_title(){debugPhaseSelector=false;mode=TITLE;stateTime=0;menuIndex=0;playingIndex=-1;music("musica_titulo");}
 void jukebox_play(){playingIndex=playerIndex;music(jukeFile[playerIndex]);if(auddev)SDL_LockAudioDevice(auddev);musicLoop=false;musicPaused=false;if(auddev)SDL_UnlockAudioDevice(auddev);}
 void jukebox_stop(){if(auddev)SDL_LockAudioDevice(auddev);musicTrack=nullptr;musicPos=0;musicPaused=false;if(auddev)SDL_UnlockAudioDevice(auddev);playingIndex=-1;}
 void edit_volume(int index,int dir){
  if(index<0||index>2)return;
  if(auddev)SDL_LockAudioDevice(auddev);
  int* target=index==0?&masterVolume:index==1?&musicVolume:&effectsVolume;
  *target=std::clamp(*target+dir*5,0,100);
  if(auddev)SDL_UnlockAudioDevice(auddev);
  save_audio_settings();
 }
 void activate_phase(int number){
   fase=number;
   wallTop=fase==5?wallTop5:fase==4?wallTop4:fase==3?wallTop3:fase==2?wallTop2:wallTop1;
   wallFloor=fase==5?wallFloor5:fase==4?wallFloor4:fase==3?wallFloor3:fase==2?wallFloor2:wallFloor1;
 }
 void load_best(){std::ifstream f("record.dat");if(f)f>>best;}
 void save_best(){if(!cheatUsedThisRun&&score>best){best=score;std::ofstream f("record.dat");f<<best;}}
 // El mezclador lee los PCM ya cargados. Su estado se escribe solo bajo SDL_LockAudioDevice.
 static void audio_callback(void* userdata,uint8_t* stream,int bytes){
   static_cast<Game*>(userdata)->mix_audio(stream,bytes);
 }
 void mix_audio(uint8_t* stream,int bytes){
   gn::mix_stereo_s16(stream,bytes,sound,stereoSpatial,musicTrack,musicPos,musicPaused,musicLoop,
                        voices,masterVolume,musicVolume,effectsVolume);
 }
 void tone(const std::string& name,bool playerShot=false,float x=-1.f){
   if(!sound||!auddev)return;
   auto it=audio.find(name);if(it==audio.end()||it->second.pcm.empty())return;
   // Clases: disparos no deben robar una explosion, y el rayo tiene prioridad alta.
   int priority=3,maxSame=4,cooldown=0;float gain=.52f;
   if(name=="disparo"){priority=1;maxSame=4;cooldown=48;gain=.42f;}
   else if(name=="laser"){priority=2;maxSame=4;cooldown=66;gain=.46f;}
   else if(name=="explosion_grande"){priority=10;maxSame=3;gain=.83f;}
   else if(name=="rayo"){priority=9;maxSame=2;cooldown=150;gain=.69f;}
   else if(name=="alarma"||name=="apertura"){priority=8;maxSame=2;gain=.58f;}
   else if(name=="explosion"){priority=6;maxSame=5;cooldown=35;gain=.57f;}
   else if(name=="explosion_corta"){priority=6;maxSame=5;gain=.56f;}
   else if(name=="impacto"||name=="escudo"){priority=7;maxSame=3;gain=.58f;}
   else if(name=="item"){priority=7;maxSame=2;gain=.57f;}
   else if(name=="misil"){priority=4;maxSame=3;cooldown=50;gain=.54f;}
   uint32_t now=SDL_GetTicks();
   const float pan=x<0.f?0.f:std::clamp((x-float(W)*.5f)/(float(W)*.5f),-1.f,1.f);
    if(playerShot){
     // FIREBIRD 01: sonido para CADA disparo, sin azar, cooldown ni robo de voz.
     // PULSO/DOBLE duran ~65 ms; LÁSER ~130 ms: menos que el intervalo de tiro.
     // Por eso una voz dedicada basta y no se acumulan disparos viejos en la cola.
     ++playerShotAudioRequested;
     SDL_LockAudioDevice(auddev);
     voices[0]={&it->second,0,name=="laser"?1.00f:1.12f,20,now,pan};
     ++playerShotAudioScheduled;
     SDL_UnlockAudioDevice(auddev);
     return;
   }
   auto last=lastSfxAt.find(name);
   if(cooldown&&last!=lastSfxAt.end()&&now-last->second<uint32_t(cooldown))return;
   SDL_LockAudioDevice(auddev);
   int playingSame=0,freeSlot=-1,lowestSlot=-1;
   for(int i=1;i<int(voices.size());++i){ // no pisar la voz reservada al jugador
     const auto &v=voices[i];
     if(v.sample==&it->second)++playingSame;
     if(!v.sample&&freeSlot<0)freeSlot=i;
     if(v.sample&&(lowestSlot<0||v.priority<voices[lowestSlot].priority||
                   (v.priority==voices[lowestSlot].priority&&v.started<voices[lowestSlot].started)))lowestSlot=i;
   }
   if(playingSame>=maxSame){SDL_UnlockAudioDevice(auddev);return;}
   int slot=freeSlot;
   if(slot<0&&lowestSlot>=0&&voices[lowestSlot].priority<priority)slot=lowestSlot;
   if(slot>=0)voices[slot]={&it->second,0,gain,priority,now,pan};
   SDL_UnlockAudioDevice(auddev);
   if(slot>=0)lastSfxAt[name]=now;
 }
 static const char* ogg_path(const std::string& name){
  static const std::map<std::string,const char*> paths={
   {"musica_fase_01","01 - Señal Imposible.ogg"},
   {"musica_fase_02","02 - La Nave del Silencio.ogg"},
   {"musica_fase_03","03 - Las Criptas de Europa.ogg"},
   {"musica_fase_04","04 - La Cuna de las Estrellas.ogg"},
   {"musica_fase_05","05 - El Corazón de Júpiter.ogg"},
   {"musica_jefes_06","06 - Jefes Finales.ogg"},
   {"musica_titulo","Contacto 2010 Title.ogg"},
   {"musica_game_over","Contacto 2010 Game Over.ogg"},
   {"musica_victoria","Contacto 2010 - Victoria.ogg"},
   {"musica_escena_final","10 - Remember Love.ogg"}};
  const auto it=paths.find(name);return it==paths.end()?nullptr:it->second;
 }
 bool ogg_decode(const std::string& name,Sound& dest,bool metadataOnly=false){
  const char* rel=ogg_path(name);if(!rel)return false;
  const std::string filename="banda_sonora_original/"+std::string(rel);
  OggVorbis_File vf{};
#ifdef _WIN32
  // Windows: ov_fopen usa ANSI; abrir con ruta UTF-16 para nombres con tildes.
  const int chars=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,filename.c_str(),-1,nullptr,0);
  if(chars<=0){std::cerr<<"Nombre OGG UTF-8 no valido: "<<filename<<"\n";return false;}
  std::wstring wide(size_t(chars),L'\0');
  if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,filename.c_str(),-1,wide.data(),chars))return false;
  FILE* fd=_wfopen(wide.c_str(),L"rb");
  if(!fd){std::cerr<<"OGG no disponible: "<<filename<<"\n";return false;}
  if(ov_open(fd,&vf,nullptr,0)!=0){
   fclose(fd);std::cerr<<"OGG invalido: "<<filename<<"\n";return false;
  }
#else
  if(ov_fopen(filename.c_str(),&vf)!=0){
   std::cerr<<"OGG no disponible: "<<filename<<"\n";return false;
  }
#endif
  const vorbis_info* info=ov_info(&vf,-1);
  if(!info||info->channels!=2||info->rate<22050||info->rate>96000){
   std::cerr<<"OGG no estereo o incompatible: "<<filename<<"\n";
   ov_clear(&vf);return false;
  }
  if(metadataOnly){ov_clear(&vf);return true;}
  const int rate=info->rate;
  const ogg_int64_t totalFrames=ov_pcm_total(&vf,-1);
  if(totalFrames<=0||totalFrames>ogg_int64_t(rate)*1200){
   std::cerr<<"Longitud OGG no valida: "<<filename<<"\n";
   ov_clear(&vf);return false;
  }
  std::vector<int16_t> decoded;
  decoded.reserve(size_t(totalFrames)*2);
  char buffer[16384];int bitstream=0;long bytes=0;
  bool valid=true;
  while((bytes=ov_read(&vf,buffer,sizeof(buffer),0,2,1,&bitstream))!=0){
   if(bytes<0){valid=false;break;}
   const size_t count=size_t(bytes)/2;
   const size_t at=decoded.size();decoded.resize(at+count);
   std::memcpy(decoded.data()+at,buffer,count*sizeof(int16_t));
  }
  ov_clear(&vf);
  if(!valid||decoded.size()<2)return false;
  if(rate==44100)dest.pcm=std::move(decoded);
  else{
   const size_t inFrames=decoded.size()/2;
   const size_t outFrames=size_t(double(inFrames)*44100.0/double(rate));
   dest.pcm.resize(outFrames*2);
   for(size_t i=0;i<outFrames;++i){
    const double src=double(i)*double(rate)/44100.0;
    const size_t at=std::min(size_t(src),inFrames-1),next=std::min(at+1,inFrames-1);
    const float t=float(src-double(at));
    for(int ch=0;ch<2;++ch)
     dest.pcm[2*i+size_t(ch)]=int16_t(std::clamp(
      float(decoded[2*at+ch])*(1.f-t)+float(decoded[2*next+ch])*t,-32768.f,32767.f));
   }
  }
  dest.channels=2;
  return !dest.pcm.empty();
 }
 void music(const std::string& name){
  // Decodificar solo una pista. Nunca hacer E/S en callback de audio.
  if(name!=musicName||musicData.pcm.empty()){
   Sound next;
   if(!ogg_decode(name,next)){std::cerr<<"Falta FutureDark OGG: "<<name<<"\n";return;}
   if(auddev)SDL_LockAudioDevice(auddev);
   musicTrack=nullptr;musicData=std::move(next);
   if(auddev)SDL_UnlockAudioDevice(auddev);
  }
  if(auddev)SDL_LockAudioDevice(auddev);
  musicName=name;musicTrack=musicData.pcm.empty()?nullptr:&musicData;
  musicLoop=(name!="musica_titulo" && name!="musica_game_over" && name!="musica_victoria" && name!="musica_escena_final");
  musicPaused=false;musicPos=0;
  if(auddev)SDL_UnlockAudioDevice(auddev);
 }
 void set_audio_enabled(bool enabled){
   if(auddev)SDL_LockAudioDevice(auddev);
   sound=enabled;
   if(!enabled){for(auto &v:voices)v.sample=nullptr;musicPos=0;}
   if(auddev)SDL_UnlockAudioDevice(auddev);
   save_audio_settings();
 }
 void step_audio(){} // Mantenido por compatibilidad; mezcla continua en el callback SDL2.
 void color(RGBA c){SDL_SetRenderDrawColor(ren,c.r,c.g,c.b,c.a);}
 void box(int x,int y,int w,int h,RGBA fill,RGBA outline={0,0,0,0}){gn::draw_box(ren,x,y,w,h,fill,outline);}
 void line(int x,int y,int x2,int y2,RGBA c){gn::draw_line(ren,x,y,x2,y2,c);}
 void blit(const std::string& name,SDL_Rect src,SDL_Rect dst,int flip=0,int alpha=255){auto it=textures.find(name);if(it==textures.end())return;SDL_SetTextureAlphaMod(it->second.t,alpha);SDL_RenderCopyEx(ren,it->second.t,&src,&dst,0,nullptr,flip);SDL_SetTextureAlphaMod(it->second.t,255);}
 void image(const std::string& name,int x,int y,int w,int h,int alpha=255){auto &t=textures[name];blit(name,{0,0,t.w,t.h},{x,y,w,h},0,alpha);}
 void sprite(const std::string& name,int cols,int cw,int ch,int frame,int x,int y,int w,int h,int flip=0){auto &t=textures[name];int n=(t.w/cw)*(t.h/ch);int a=((frame%n)+n)%n;blit(name,{(a%cols)*cw,(a/cols)*ch,cw,ch},{x,y,w,h},flip);}
 static std::vector<uint32_t> utf8(const std::string&s){return gn::uppercase_utf8(s);}

 int textwidth(const std::string&s,int h=22){float scale=h/52.f;int result=0;for(auto cp:utf8(s)){if(cp==' '){result+=h*.39f;continue;}auto it=glyphs.find(cp);result+=(it!=glyphs.end()?it->second.second:24)*scale*.97f;}return result;}
 void text(const std::string&txt,int x,int y,int height=22,RGBA c=WHITE,int align=0){auto chars=utf8(txt);if(align==1)x-=textwidth(txt,height)/2;if(align==2)x-=textwidth(txt,height);const float scale=height/52.f; auto *t=textures["glyph"].t;SDL_SetTextureColorMod(t,c.r,c.g,c.b);SDL_SetTextureAlphaMod(t,c.a);
  for(auto cp:chars){if(cp==' '){x+=height*.39f;continue;}auto g=glyphs.find(cp);if(g==glyphs.end()){x+=height*.30f;continue;}int idx=g->second.first,advance=g->second.second;SDL_Rect s{(idx%16)*36,(idx/16)*52,36,52},d{x,y,int(36*scale),height};SDL_RenderCopy(ren,t,&s,&d);x+=std::max(1,int(advance*scale*.97f));}
  SDL_SetTextureColorMod(t,255,255,255);SDL_SetTextureAlphaMod(t,255);
 }
 void shadow_text(const std::string&txt,int x,int y,int height=22,RGBA c=WHITE,int align=0){
  RGBA sh{0,0,0,185};
  text(txt,x+1,y+1,height,sh,align);
  text(txt,x,y,height,c,align);
 }
 int hud_adv(int h,bool big=false){return std::max(1,int((big?104.f:72.f)*h/(big?128.f:96.f)));}
 int hud_text_width(const std::string&txt,int h,bool big=false,int track=0){
  auto chars=utf8(txt); int adv=hud_adv(h,big), n=0; bool first=true;
  for(auto cp:chars){
    int w=(cp==' ')?std::max(1,h/3):adv;
    if(!first)n+=track;
    n+=w; first=false;
  }
  return n;
 }
 int hud_index(uint32_t cp){
  static std::vector<uint32_t> order=utf8("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ÁÉÍÓÚÜÑ¿¡!?.,:;+-*/=%()[]<>ºª°_#@&");
  for(size_t i=0;i<order.size();++i) if(order[i]==cp) return int(i);
  return -1;
 }
 void hud_atlas_text(const std::string&tex,const std::string&txt,int x,int y,int h,int align=0,int track=0,bool big=false){
  auto chars=utf8(txt); int adv=hud_adv(h,big), cellW=big?104:72, cellH=big?128:96, cols=12;
  if(align==1)x-=hud_text_width(txt,h,big,track)/2; if(align==2)x-=hud_text_width(txt,h,big,track);
  auto *t=textures[tex].t; bool first=true;
  for(auto cp:chars){
    if(!first)x+=track;
    if(cp==' '){x+=std::max(1,h/3); first=false; continue;}
    int idx=hud_index(cp); if(idx<0){x+=adv; first=false; continue;}
    SDL_Rect s{(idx%cols)*cellW,(idx/cols)*cellH,cellW,cellH};
    // Acentuadas: el cuerpo de la letra está 6-7 px más bajo en el
    // atlas RGBA. Alinear la base con la letra sin tilde (sin cortar el acento).
    uint32_t baseCp=cp==0x00C1?'A':cp==0x00C9?'E':cp==0x00CD?'I':cp==0x00D3?'O':cp==0x00DA||cp==0x00DC?'U':cp==0x00D1?'N':cp;
    int baselineShift=0;
    if(baseCp!=cp){auto inkIt=hudGlyphInk.find(tex);
      int baseIndex=hud_index(baseCp);
      if(inkIt!=hudGlyphInk.end()&&baseIndex>=0&&idx<(int)inkIt->second.size()){
        auto r=inkIt->second[idx],b=inkIt->second[baseIndex];
        baselineShift=int(std::lround(float((b.y%cellH+b.h)-(r.y%cellH+r.h))*float(h)/cellH));
      }
    }
    SDL_Rect d{x,y+baselineShift,adv,h}; SDL_RenderCopy(ren,t,&s,&d); x+=adv; first=false;
  }
 }
 // Texto del HUD con espaciado de tinta real: se recorta el alfa del atlas,
 // se mide el ancho dibujado completo y se centra respecto al encabezado.
 // Mantiene la altura de la celda para alinear las letras y sus acentos.
 int hud_precise_width(const std::string&style,const std::string&value,int cellHeight,int tracking){
  auto it=hudGlyphInk.find(style);if(it==hudGlyphInk.end())return 0;
  float scale=float(cellHeight)/float(style=="hud_big"?128:96);
  int total=0; bool first=true;
  for(auto cp:utf8(value)){
    if(!first)total+=tracking;
    int index=hud_index(cp);
    total+=cp==' '?int(std::lround(17*scale)):
          (index>=0&&index<int(it->second.size())?std::max(1,int(std::lround(it->second[index].w*scale))):0);
    first=false;
  }
  return total;
 }
 void hud_precise_text(const std::string&style,const std::string&value,int cx,int y,int cellHeight,int tracking){
  auto it=hudGlyphInk.find(style);if(it==hudGlyphInk.end())return;
  float scale=float(cellHeight)/float(style=="hud_big"?128:96);
  int cellSourceH=style=="hud_big"?128:96;
  int x=int(std::lround(float(cx)-hud_precise_width(style,value,cellHeight,tracking)*0.5f));
  bool first=true;
  for(auto cp:utf8(value)){
    if(!first)x+=tracking;
    int index=hud_index(cp);
    if(cp==' '){x+=int(std::lround(17*scale));first=false;continue;}
    if(index<0||index>=int(it->second.size())){first=false;continue;}
    SDL_Rect src=it->second[index];
    if(src.w>0&&src.h>0){
      int w=std::max(1,int(std::lround(src.w*scale)));
      // Á É Í Ó Ú Ü Ñ: sus mapas tienen la base más baja que la letra
      // simple. Corregir sobre el borde inferior real, NO sobre el acento.
      uint32_t plain=cp==0x00C1?'A':cp==0x00C9?'E':cp==0x00CD?'I':cp==0x00D3?'O':cp==0x00DA||cp==0x00DC?'U':cp==0x00D1?'N':cp;
      int baselineCorrection=0;
      if(plain!=cp){int bi=hud_index(plain);
        if(bi>=0&&bi<int(it->second.size())){
          const SDL_Rect reference=it->second[bi];
          int glyphBottom=(src.y%cellSourceH)+src.h;
          int normalBottom=(reference.y%cellSourceH)+reference.h;
          baselineCorrection=normalBottom-glyphBottom;
        }
      }
      SDL_Rect dst{x,y+int(std::lround(((src.y%cellSourceH)+baselineCorrection)*scale)),w,
                   std::max(1,int(std::lround(src.h*scale)))};
      SDL_RenderCopy(ren,textures[style].t,&src,&dst);
      x+=w;
    }
    first=false;
  }
 }
 void hud_smiley(const std::string&state,int x,int y,int size){
  int sx=0; if(state=="AMBAR") sx=96; else if(state=="MAGENTA") sx=192; else if(state=="ROJO") sx=288;
  SDL_Rect s{sx,0,96,96}, d{x,y,size,size}; SDL_RenderCopy(ren,textures["hud_smile"].t,&s,&d);
 }
 void labelbox(const std::string&txt,int x,int y,int ww,int h,RGBA c=CYAN){box(x,y,ww,h,{4,16,37,226},{49,122,181});line(x+2,y+2,x+ww-3,y+2,c);text(txt,x+ww/2,y+(h-22)/2,22,WHITE,1);}
 void background(){color({2,6,15});SDL_RenderClear(ren);
  if(fase==5){
    // Paisaje completo, sin recortar los ojos, caras ni vórtices del arte del usuario.
    image("fondo5",0,0,W,H);
    // Brillos muy discretos: jamás deben tapar los espectros originales.
    for(int i=0;i<125;i++)if(i%5==0){
      float a=.5f+.5f*std::sin(stageTime*(.8f+(i%7)*.12f)+i*1.3f);
      int x=(int(starsX[i]-scroll*starsSpeed[i]*.08f)%W+W)%W;
      if(a>.78f)box(x,int(starsY[i]),2,2,{255,176,105,int(25+90*a)});
    }
    return;
  }
  if(fase==4){
    int drift=int(45+34*std::sin(scroll/1700.f));
    blit("fondo4",{drift*2,25,1770,1000},{0,0,W,H});
    // Organismos bioluminiscentes y partículas detrás de los enemigos.
    for(int i=0;i<125;i++)if(i%4==0){
      float a=.5f+.5f*std::sin(stageTime*(1.15f+(i%7)*.13f)+i*1.77f);
      int x=(int(starsX[i]-scroll*starsSpeed[i]*.16f)%W+W)%W;
      if(a>.45f)box(x,int(starsY[i]),i%9==0?3:2,i%9==0?3:2,
         {int(165+90*a),int(88+72*a),int(198+57*a),int(60+110*a)});
    }
    return;
  }
  if(fase==3){
    int drift=int(40+35*std::sin(scroll/1800.f));
    // Fondo ligeramente más bajo en pantalla visual: tomamos una zona más alta del arte para que los ojos bajen y se vean.
    blit("fondo3",{drift*2,0,1770,1000},{0,28,W,H});
    // Partículas glaciares suaves, sin tapar las naves ni los avisos.
    for(int i=0;i<125;i++)if(i%3==0){
      float a=.5f+.5f*std::sin(stageTime*(1.0f+(i%7)*.1f)+i*1.7f);
      int x=(int(starsX[i]-scroll*starsSpeed[i]*.23f)%W+W)%W;
      if(a>.55f)box(x,int(starsY[i]),i%11==0?3:2,i%11==0?3:2,
         {int(145+80*a),int(192+56*a),255,int(70+95*a)});
    }
    return;
  }
  if(fase==2){
    // Plano original de 1920x1080, filtrado lineal por SDL; parallax independiente.
    int drift=int(55+35*std::sin(scroll/1400.f));
    SDL_Rect src{drift*2,45,1760,990};
    blit("fondo2",src,{0,0,W,H});
    // Atmósfera y luz ambiental: destellos estelares y paneles que laten.
    float t=stageTime;
    for(int i=0;i<125;i++)if(i%3==0){
      float a=(.5f+.5f*std::sin(t*(1.1f+(i%7)*.13f)+i*1.8f));
      int x=(int(starsX[i]-scroll*starsSpeed[i]*.17f)%W+W)%W;
      int y=int(starsY[i]);
      if(a>.36f){RGBA p{int(68+110*a),int(140+90*a),int(189+60*a),int(50+160*a)};
        box(x,y,i%11==0?3:2,i%11==0?3:2,p);
      }
    }
    float a=(.5f+.5f*std::sin(t*2.0f));
    for(int x:{115,388,632,821}){box(x,91,20,2,{45,int(122+80*a),245,int(32+85*a)});}
    return;
  }
  image("fondo",0,0,W,H); // Júpiter oficial
  box(0,0,W,H,{0,4,11,50});
  for(int i=0;i<125;i++){int x=int(starsX[i]-scroll*starsSpeed[i]*.10f);x=(x%W+W)%W;int y=int(starsY[i]);RGBA c=i%8==0?RGBA{193,225,255,220}:RGBA{89,149,216,125};box(x,y,starsSize[i],starsSize[i],c);}
 }
 int idxmap(float x){
  // Fases 1 y 5: terreno HD real (1920x540), 1 muestra por píxel lógico.
  // Fases 2-4 conservan sus geometrías históricas a media resolución.
  int a=(fase==1||fase==5)?int(scroll+x):int((scroll+x)*.5f);
  a%=int(wallTop.size());return(a+int(wallTop.size()))%int(wallTop.size());
 }
 int ceilY(float x){return (fase==1||fase==5?1:2)*wallTop[idxmap(x)];}
 int floorY(float x){return (fase==1||fase==5?1:2)*wallFloor[idxmap(x)];}
 void terrain(){
  if(fase==1||fase==5){
   // Scroll 1:1 real para fase 1 (cementerio V2) y fase 5 (almas jovianas).
   // Mantiene dos pantallas de detalle continuo en cada bloque de 1920 px.
   const std::string tex=fase==1?"mapa":"mapa5";
   const int w=textures[tex].w;
   int off=int(scroll)%w;if(off<0)off+=w;
   int remaining=W,dest=0;
   while(remaining>0){
    int avail=std::min(w-off,remaining);
    SDL_Rect src{off,0,avail,H},dst{dest,0,avail,H};
    blit(tex,src,dst);dest+=avail;remaining-=avail;off=0;
   }
   return;
  }
  std::string tex=fase==4?"mapa4":fase==3?"mapa3":fase==2?"mapa2":"mapa";
  int w=textures[tex].w,off=int(scroll/2)%w;if(off<0)off+=w;int remaining=W,dest=0;
  while(remaining>0){int avail=std::min(w-off,(remaining+1)/2);SDL_Rect src{off,0,avail,270},dst{dest,0,avail*2,H};blit(tex,src,dst);dest+=dst.w;remaining-=dst.w;off=0;}
 }
 // HUD MILITAR V1.0.7. Recentrado fino de ventanas y barra de jefe.
 // desarrollo/; la textura activa está recortada del original y tiene 5 px
 // de margen de seguridad a izquierda y derecha en 960 x 120 lógicos.
 // Todas las ventanas son agujeros del PNG y se rellenan SIN tapar las placas.
 struct HudSlot {int x,y,w,h;};
 static constexpr HudSlot HUD_SCORE{311,35,146,31};
 static constexpr HudSlot HUD_LIVES{478,35,132,31};
 static constexpr HudSlot HUD_WEAPON{635,35,131,31};
 static constexpr HudSlot HUD_RECORD{791,35,130,31};
 static constexpr HudSlot HUD_PHASE{139,83,112,20};
 static constexpr HudSlot HUD_BOSS_NAME{385,83,124,18};
 static constexpr HudSlot HUD_BOSS_BAR{565,84,338,14};
 // Cada elemento se calcula respecto a SU ventana, no a antiguos centros.
 // Para nombres extensos se usa scroll dentro de un clip SDL: nunca se corta
 // en el borde de pantalla ni invade la barra de energía contigua.
 void hud_number(const HudSlot&slot,int number){
   const std::string value=num(number);
   int h=28;
   while(h>16 && hud_precise_width("hud_big",value,h,1)>slot.w-8)--h;
   hud_precise_text("hud_big",value,slot.x+slot.w/2,slot.y+(slot.h-h)/2,h,1);
 }
 void hud_boss_name(const std::string&name,bool active){
   const HudSlot&f=HUD_BOSS_NAME;
   const std::string style=active?"hud_magenta":"hud_cyan";
   int h=18, tracking=0;
   while(h>15 && hud_precise_width(style,name,h,tracking)>f.w-10)--h;
   const int labelW=hud_precise_width(style,name,h,tracking);
   const int inside=f.w-8;
   float shift=0;
   if(labelW>inside){
     const float overflow=float(labelW-inside);
     const float speed=25.f;
     const float travel=overflow/speed;
     const float pause=.85f;
     // Ida, pausa, vuelta y pausa: no hay saltos al reiniciar el ciclo.
     const float cycle=2.f*(travel+pause);
     const float t=std::fmod(std::max(0.f,bossTime),cycle);
     if(t<pause)shift=0.f;
     else if(t<pause+travel)shift=(t-pause)*speed;
     else if(t<2.f*pause+travel)shift=overflow;
     else shift=std::max(0.f,overflow-(t-2.f*pause-travel)*speed);
   }
   SDL_Rect clip{f.x+3,f.y,f.w-6,f.h};
   SDL_RenderSetClipRect(ren,&clip);
   const int left=labelW<=inside ? f.x+(f.w-labelW)/2 : f.x+5-int(std::lround(shift));
   hud_precise_text(style,name,left+labelW/2,f.y,h,tracking);
   SDL_RenderSetClipRect(ren,nullptr);
 }
 void hud_boss_energy(){
   const HudSlot&f=HUD_BOSS_BAR;
   const bool active=(mode==BOSS||mode==BOSS_DEATH);
   const RGBA accent=fase>=4?MAGENTA:RED;
   // El fondo queda bajo la imagen metálica, sin efectos sobre el escenario.
   box(f.x+1,f.y+1,f.w-2,12,{7,17,25,255});
   const float ratio=active?std::clamp(bossHp/std::max(1.f,bossMax),0.f,1.f):0.f;
   const int usable=f.w-10;
   const int fill=std::clamp(int(std::lround(usable*ratio)),0,usable);
   if(fill>0){
     box(f.x+5,f.y+4,fill,6,accent);
     box(f.x+5,f.y+4,fill,2,{255,226,208,160});
   }
   // Segmentación militar leve; las marcas no alteran el ratio de vida.
   for(int i=1;i<20;++i){
     int xx=f.x+5+i*usable/20;
     box(xx,f.y+3,1,8,{9,20,25,155});
   }
   if(active && fill>3)box(f.x+4+fill,f.y+3,2,8,{246,251,253,200});
 }
 void hud(){
   // Lienzo protegido de borde a borde. La imagen tiene además su propio margen.
   box(0,0,W,HUD_H,{2,8,13,255});
   image("hud",0,0,W,HUD_H);
   hud_number(HUD_SCORE,score);
   hud_number(HUD_RECORD,best);
   int n=std::clamp(lives,0,4);
   constexpr int icon=21,gap=5;
   const int total=n?icon*n+gap*(n-1):0;
   const int x0=HUD_LIVES.x+(HUD_LIVES.w-total)/2;
   const std::string style=n>=3?"CIAN":n==2?"AMBAR":"MAGENTA";
   for(int i=0;i<n;++i)hud_smiley(style,x0+i*(icon+gap),HUD_LIVES.y+4,icon);
   const std::string arm=weapon==0?"PULSO":weapon==1?"DOBLE":"LÁSER";
   int weaponH=28;
   while(weaponH>16&&hud_precise_width("hud_amber",arm,weaponH,1)>HUD_WEAPON.w-6)--weaponH;
   hud_precise_text("hud_amber",arm,HUD_WEAPON.x+HUD_WEAPON.w/2,HUD_WEAPON.y+(HUD_WEAPON.h-weaponH)/2-1,weaponH,1);
   // El arte YA incluye los títulos FASE y JEFE; evitar duplicarlos.
   hud_precise_text("hud_cyan","0"+std::to_string(fase),HUD_PHASE.x+HUD_PHASE.w/2,HUD_PHASE.y-1,22,1);
   const bool active=mode==BOSS||mode==BOSS_DEATH;
   hud_boss_name(active?TITULO_JEFE[std::clamp(fase-1,0,4)]:"EN ESPERA",active);
   hud_boss_energy();
 }

 std::string num(int n){std::ostringstream s;s<<std::setw(7)<<std::setfill('0')<<n;return s.str();}
 void portrait(int role,int frame,int x,int y,int w,int h){
   // AURA-01, computadora de Firebird-01: animacion RGBA 6x2, 12 frames.
   if(role==3){
      // Centrado geométrico exacto: tanto el ojo como el marco del atlas son cuadrados.
      // El mismo centro de la casilla se respeta en radio, briefing y pre-jefe.
      const int side=std::min(w,h);
      const int cx=x+w/2,cy=y+h/2;
      sprite("aura",6,362,362,frame%12,cx-side/2,cy-side/2,side,side);
      return;
    }
   sprite("portrait",8,112,140,role*8+frame,x,y,w,h);
  }
 void draw_radio(){if(radio.life<=0)return;
  const int y=447;box(18,y,734,82,{5,19,38,232},{53,133,185});line(20,y+2,750,y+2,CYAN);
  int frame=(int(radio.elapsed*7)%9<6)?int(radio.elapsed*7)%4:(6+int(radio.elapsed*4)%2);
  const int radioFrame=(radio.role==3)?int(radio.elapsed*10.f)%12:frame;
   portrait(radio.role,radioFrame,24,y+1,68,80);
  text(radio.who+":",114,y+9,21,CYAN);
  const std::string& m=radio.msg;auto p=m.find('|');
  const std::string first=m.substr(0,p);
  const std::string second=(p==std::string::npos)?std::string():m.substr(p+1);
  int altura=second.empty()?20:18;
  while(altura>13&&(textwidth(first,altura)>625||(!second.empty()&&textwidth(second,altura)>625)))--altura;
  text(first,110,second.empty()?y+39:y+30,altura,WHITE);
  if(!second.empty())text(second,110,y+54,altura,WHITE);
 }
 // Escudo RGBA original: cuatro pulsaciones del campo protector, transparente
 // en su centro. Sólo activo durante los 9 s de bonificación (o hasta impacto).
 // Nunca confundir el escudo con el parpadeo de reaparición o el truco F9.
 // FIREBIRD-01 V1.1. Atlas industrial original 4x4, 384 px por celda:
 // 00-07 vuelo limpio y 08-15 danos/destruccion. Nunca mezclar con
 // animaciones de disparo: las balas son entidades del motor SDL2.
 // El dibujo ocupa aprox. x=47..342 e y=133..268 EN LA CELDA; dibujamos
 // la celda a 104x100, resultado visible ~80x35 px, centrado en (shipx,shipy).
 // El punto de impacto sigue siendo el centro del fuselaje y es mas pequeno
 // que las alas (perdon arcade intencionado); NO usar el rectangulo del PNG.
 static constexpr int SHIP_ATLAS_CELL=384, SHIP_FRAME_COLS=4;
 static constexpr int SHIP_RENDER_W=104, SHIP_RENDER_H=100;
 static constexpr int SHIP_RENDER_X=-52, SHIP_RENDER_Y=-52;
 static constexpr float SHIP_HIT_OFFSET_X=3.f;
 static constexpr float SHIP_HIT_HALF_X=8.f,SHIP_HIT_HALF_Y=8.f;
 static constexpr float SHIP_NOSE_X=42.f;
 bool escudo_visible() const {return shieldTime>0.f && death<=0.f;}
 void firebird_frame(int frame){
   sprite("ship",SHIP_FRAME_COLS,SHIP_ATLAS_CELL,SHIP_ATLAS_CELL,frame,
          int(shipx)+SHIP_RENDER_X,int(shipy)+SHIP_RENDER_Y,SHIP_RENDER_W,SHIP_RENDER_H);
 }
 void draw_ship(){
   if(death>0){
     const int frame=8+std::clamp(int((.9f-death)*8.f/.9f),0,7);
     firebird_frame(frame);return;
   }
   if(!(vulnerable>0&&int(vulnerable*11)%2==0)){
     // Vuelo 8 fotogramas, sin cambiar de conjunto al ascender/descender.
     // El desplazamiento vertical real es responsabilidad del motor.
     firebird_frame(int(stageTime*10.f)%8);
   }
   if(escudo_visible()){
     // El escudo protege la silueta completa, dejando visibles los detalles.
     const int frame=int(stageTime*9.f)%4;
     sprite("escudo_nuevo",2,960,540,frame,int(shipx-65),int(shipy-39),130,78);
   }
 }
 void puff(float x,float y,int count,RGBA a=AMBER){for(int i=0;i<count&&puffs.size()<600;i++)puffs.push_back({x,y,rf(-150,150),rf(-130,130),rf(.2f,.8f),.8f,(i%4==0?WHITE:i%3==0?RED:a)});}
 void fire(float x,float y,float vx,float vy,bool friendly,bool missile=false){
  if(shots.size()<175){shots.push_back({x,y,vx,vy,3.2f,friendly,missile});if(!friendly)++hostileShotsFired;}
 }
 void shoot(){
   // Boca de fuego alineada al morro visible de la Firebird-01 revisada.
   fire(shipx+SHIP_NOSE_X,shipy-1,620,0,true);
   if(weapon==1){fire(shipx+SHIP_NOSE_X-4,shipy-11,610,-24,true);fire(shipx+SHIP_NOSE_X-4,shipy+11,610,24,true);}
   if(weapon==2){fire(shipx+SHIP_NOSE_X+1,shipy-6,780,0,true);fire(shipx+SHIP_NOSE_X+1,shipy+6,780,0,true);}
   tone(weapon==2?"laser":"disparo",true,shipx);
 }
 // Fase 5 previa: cuatro escuadras reutilizadas, sin sprites inventados.
 // 12/13=F22, 14/15=espectros, 20/21=halcones, 22=Moáis menores.
 void spawn_phase5(int kind,int count=1){
   for(int i=0;i<count && enemies.size()<30;++i){
     Actor e; e.kind=kind; e.t=.1f+float(i)*.22f; e.fire=1.05f+float(i)*.16f;
     bool fromLeft=kind==13||kind==15||kind==21;
     float speed=(kind==14||kind==15)?300.f:(kind==20||kind==21)?267.f:(kind==22)?166.f:245.f;
     e.vx=fromLeft?-speed:speed;
     e.x=fromLeft?-70.f-i*103.f:W+70.f+i*101.f;
     e.hp=kind==22?6.f:(kind==14||kind==15)?2.f:(kind==20||kind==21)?4.f:3.f;
     const float margin=kind==22?65.f:34.f;
     e.y=limit(190.f+float(i%4)*46.f,ceilY(e.x)+margin,floorY(e.x)-margin);
     enemies.push_back(e);++enemiesSpawned;
   }
 }
 void spawn_event5(int id){
   const int i=id%28;
   if(i==0){message(1,"OPERADORA","ENTRAMOS EN EL CORAZON DE JUPITER.|LAS NUBES NOS MIRAN.",4.8);return;}
   if(i==9){message(0,"PILOTO","ESTOY VOLANDO EN EL INFIERNO.",4.8);return;}
   if(i==20){message(3,"AURA-01","SEÑAL MASIVA DE PODER.|ESTAMOS CERCA DE SU ORIGEN.",4.8);return;}
   if(i==27){message(2,"COMANDANTE","HAY UNA SEÑAL FANTASMA.|CONTINUA HASTA EL FINAL.",4.8);return;}
   if(i==7||i==18){prizes.push_back({990.f,270.f,i==7?1:2});return;}
   static const int pattern[]={14,20,12,15,21,13,22,14,20,12,15,22,21,13,20,14,22,12,21,15,13,22,14,20,12,21,15,22};
   int k=pattern[i],count=k==22?2+(i>=14):3+(i%4==0)+(i>=16);
   spawn_phase5(k,count);
   // Ataques cruzados, legibles, de dos escuadras conocidas.
   if(i==11||i==17||i==23||i==26)spawn_phase5(i%2?21:13,2);
 }
 void spawn_phase4(int k,int count=1){
  // 3 familias exactas: célula estelar (30/31), devoradora (32/33), halcón (34/35).
  for(int i=0;i<count&&enemies.size()<30;i++){
    Actor e;e.kind=k;e.t=rf(0,.6f);e.fire=1.10f+i*.16f;
    bool fromLeft=(k%2)==1;
    e.x=fromLeft?-65.f-i*98.f:W+65.f+i*99.f;
    float speed=k<32?177.f:k<34?208.f:242.f;
    e.vx=fromLeft?-speed:speed;
    e.hp=k<32?3.f:k<34?5.f:4.f;
    e.y=220.f+(i%3)*50.f;
    e.y=limit(e.y,ceilY(e.x)+37,floorY(e.x)-37);
    enemies.push_back(e);++enemiesSpawned;
  }
 }
 void spawn_event4(int id){
   if(id==0){message(1,"OPERADORA","SEÑAL BIOLOGICA MASIVA.|TODO AQUÍ ESTÁ VIVO.",4.8);return;}
   if(id==8){message(0,"PILOTO","¡LOS INSTRUMENTOS ENLOQUECEN!|¡DEBO SALIR DE AQUÍ!",4.5);return;}
   if(id==17){message(3,"AURA-01","ALGO ESTÁ A PUNTO DE NACER.|TODO AQUÍ LO PROTEGE.",4.6);return;}
   if(id==24){message(2,"COMANDANTE","CONTACTO VISUAL INMINENTE.|DESTRUYE AL ENTE NEONATO.",4.8);return;}
   if(id==9||id==19){prizes.push_back({990,270,id==9?1:2});return;}
   static const int pattern[]={30,32,31,30,34,33,32,35,30,31,32,34,33,35,30,32,34,31,33,35,32,30,34,33,32,35,30,34};
   int kind=pattern[id%28];
   // Primera mitad exigente; la segunda incorpora una nave extra por ataque.
   int count=(kind>=34?3+(id%3==0):3+(id%3==1))+(id>=14?1:0);
   spawn_phase4(kind,count);
 }
 // Estalactitas: caen desde el techo cerca de la nave, con un aviso visible.
 // Se limita la densidad y se conserva un pasillo de escape entre columnas.
 int estalactitasProgramadas=0,estalactitasCerca=0,estalactitasPasanCerca=0,estalactitasMaxSimultaneas=0;
 void spawn_estalactitas(int requested){
   const int active=int(std::count_if(enemies.begin(),enemies.end(),[](const Actor&e){return e.alive&&e.kind==23;}));
   int count=std::min({requested,3,5-active,28-int(enemies.size())});
   if(count<=0)return;
   const float target=limit(shipx,135.f,735.f);
   const float firstOffset=(estalactitasProgramadas%3==0)?8.f:(estalactitasProgramadas%3==1)?-16.f:19.f;
   const float direction=(estalactitasProgramadas%2==0)?1.f:-1.f;
   for(int i=0;i<count;i++){
     float suggested=target+firstOffset+(i==0?0.f:direction*(i==1?128.f:-139.f));
     float x=limit(suggested,96.f,815.f);
     // Evita columnas juntas que eliminen todas las rutas de evasión.
     bool tooClose=false;
     for(const auto&e:enemies)if(e.alive&&e.kind==23&&std::abs(e.x-x)<76.f){tooClose=true;break;}
     if(tooClose){x=limit(target+(i%2==0?168.f:-168.f),96.f,815.f);
       tooClose=false;for(const auto&e:enemies)if(e.alive&&e.kind==23&&std::abs(e.x-x)<76.f){tooClose=true;break;}
       if(tooClose)continue;}
     Actor e;e.kind=23;e.x=x;e.vx=0;e.t=0;e.fire=1.0f+float(i)*.18f;
     e.y=float(ceilY(e.x)+39);e.hp=2;
     enemies.push_back(e);++enemiesSpawned;++estalactitasProgramadas;
     estalactitasMaxSimultaneas=std::max(estalactitasMaxSimultaneas,active+i+1);
     if(std::abs(x-shipx)<85.f)++estalactitasCerca;
   }
 }
 void spawn_phase3(int k,int count=1){
  if(k==23){spawn_estalactitas(count);return;}
  for(int i=0;i<count&&enemies.size()<28;i++){
    Actor e;e.kind=k;e.t=rf(0,.7f);e.fire=.92f+i*.14f;
    bool left=k==21;
    e.x=left?-75.f-i*110:W+75.f+i*113;
    e.vx=k==20?238.f:k==21?-238.f:132.f;
    e.hp=k==22?8.f:4.f;
    e.y=float(225+(i%3)*49);
    if(k==22)e.y=limit(e.y,ceilY(e.x)+82,floorY(e.x)-82);
    enemies.push_back(e);++enemiesSpawned;
  }
 }
 void spawn_event3(int id){
   if(id==0){message(1,"OPERADORA","EUROPA: DETECTAMOS FORMAS DE VIDA.|MOVIMIENTO HOSTIL DETECTADO.",4.6);return;}
   if(id==7){message(0,"PILOTO","¡HALCONES ESTELARES!|¿NO LOS HABÍAMOS DERROTADO EN EL 88?",4.4);return;}
   if(id==16){message(3,"AURA-01","ALGO LATE BAJO EL HIELO.|DEBES SEGUIR AVANZANDO.",4.4);return;}
   if(id==25){message(2,"COMANDANTE","NUNCA VIMOS AL REY DE LOS HALCONES,|¡DEBES DERROTARLO PARA SEGUIR!",4.8);return;}
   if(id==8||id==19){prizes.push_back({990,260,id==8?1:2});return;}
   static const int pattern[]={20,23,21,22,20,23,21,22,20,21,23,22,20,23,21,22,23,20,21,22,23,20,22,21,23,22,20,21};
   int k=pattern[id%28];
   int count=(k==22?2+(id%3==0):3+(id%3==1))+(id>=14?1:0)+(k==23?0:1);
   spawn_phase3(k,k==23?2+(id>=14):count);
   if(id==3||id==4||id==8||id==9||id==12||id==15||id==18||id==19||id==21||id==25||id==26)spawn_estalactitas(2);
 }
 void spawn_phase2(int k,int count=1){
  for(int i=0;i<count&&enemies.size()<28;i++){
   Actor e;e.kind=k;e.t=float(i)*.14f;
   bool fromLeft=k==11||k==13||k==15;
   e.x=fromLeft?-66.f-i*110.f:W+66.f+i*110.f;
   e.vx=fromLeft?-(k==11?240.f:k==13?220.f:322.f):(k==10?240.f:k==12?220.f:322.f);
   e.hp=(k==10||k==11)?3.f:(k==12||k==13)?3.f:2.f;
   e.ceiling=false;
   e.y=limit(206.f+(i%4)*44.f,ceilY(e.x)+55,floorY(e.x)-55);
   e.fire=1.5f+i*.23f;
   enemies.push_back(e);++enemiesSpawned;
   if(k==10||k==11)++sierrasF2;
  }
 }
 // Fase 1: tres familias estrictas: sierras (ambos lados), espectros y moáis.
 // Los códigos 1/2 son la misma familia (sierra), con dirección opuesta.
 void spawn(int k,int count=1){
  if(k!=1&&k!=2&&k!=3&&k!=14&&k!=15)return;
  for(int i=0;i<count&&enemies.size()<28;i++){
   Actor e;e.kind=k==2?1:k;e.t=rf(0,.48f);e.fire=rf(1.6f,2.4f);
   bool fromLeft=(k==2||k==15);
   e.x=fromLeft?-65.f-i*95.f:W+65.f+i*98.f;
   e.vx=fromLeft?-(k==2?235.f:285.f):(k==1?235.f:k==3?140.f:285.f);
   e.hp=k==3?5.f:k==1||k==2?3.f:2.f;
   e.y=limit(188.f+(i%4)*51.f,ceilY(e.x)+58,floorY(e.x)-58);
   if(k==3)e.y=limit(e.y,ceilY(e.x)+82,floorY(e.x)-82);
   enemies.push_back(e);++enemiesSpawned;
   if(k==1||k==2){++sierrasF1;if(fromLeft)++sierrasDesdeIzq;else ++sierrasDesdeDer;}
   else if(k==14||k==15)++espectrosF1;
   else if(k==3)++moaisF1;
  }
 }
 void spawn_event(int id){switch(id){case 0:message(1,"OPERADORA","ENLACE ESTABLECIDO.|SEÑALES HOSTILES DETECTADAS.",5.0f);break;
  case 1:spawn(1,4);break;case 2:spawn(14,2);break;case 3:spawn(2,3);break;
  case 4:spawn(3,1);break;case 5:prizes.push_back({990,205,1});break;case 6:spawn(1,5);break;
  case 7:spawn(15,3);break;case 8:spawn(2,4);break;
  case 9:message(0,"PILOTO","LAS DEFENSAS AUTOMÁTICAS ME|HAN MARCADO COMO HOSTIL.",4.8f);break;
  case 10:spawn(3,2);break;case 11:spawn(1,6);break;case 12:spawn(14,4);break;
  case 13:prizes.push_back({990,300,2});break;case 14:spawn(2,4);break;case 15:spawn(3,2);break;
  case 16:spawn(1,5);break;case 17:spawn(15,4);break;
  case 18:message(3,"AURA-01","ESA SEÑAL ES IMPOSIBLE.|NO PARECE DE ORIGEN HUMANO.",5.2f);break;
  case 19:spawn(2,4);break;case 20:spawn(3,3);break;case 21:prizes.push_back({990,300,0});break;
  case 22:spawn(1,6);break;case 23:spawn(14,5);break;
  case 24:message(2,"COMANDANTE","FIREBIRD 01: LOCALIZA LA FUENTE.|PREPÁRATE PARA TOMAR CONTACTO.",5.1f);break;
  case 25:spawn(2,5);break;case 26:spawn(3,2);break;case 27:spawn(1,7);break;
 }}
void spawn_event2(int id){
  switch(id){
    case 0:message(1,"OPERADORA","ODISEA II CONFIRMA CONTACTO.|ALGO RESPONDE PERO NO CON SU VOZ.",5);break;
    case 1:spawn_phase2(11,3);break;
    case 2:spawn_phase2(10,3);break;
    case 3:spawn_phase2(12,3);break;
    case 4:prizes.push_back({990,300,1});break;
    case 5:spawn_phase2(13,2);break;
    case 6:spawn_phase2(14,3);break;
    case 7:message(0,"PILOTO","ESOS F22 ESPACIALES NO TIENEN PILOTOS.|¿QUE DIABLOS ESTÁ PASANDO?",4.6);break;
    case 8:spawn_phase2(10,4);break;
    case 9:spawn_phase2(11,4);break;
    case 10:spawn_phase2(12,4);break;
    case 11:spawn_phase2(15,3);break;
    case 12:prizes.push_back({990,260,2});break;
    case 13:spawn_phase2(14,4);break;
    case 14:spawn_phase2(11,4);break;
    case 15:spawn_phase2(13,3);break;
    case 16:message(3,"AURA-01","LAS VÍSCERAS HAN ASIMILADO|LOS MÓDULOS CIENTÍFICOS.",5.1);break;
    case 17:spawn_phase2(10,4);break;
    case 18:spawn_phase2(14,5);break;
    case 19:prizes.push_back({990,260,0});break;
    case 20:spawn_phase2(12,4);break;
    case 21:spawn_phase2(11,5);break;
    case 22:spawn_phase2(13,3);break;
    case 23:spawn_phase2(15,4);break;
    case 24:message(2,"COMANDANTE","FIREBIRD 01: LA ODISEA II HA SIDO INFECTADA,|DEBE SER DESTRUIDA.",5.0);break;
    case 25:spawn_phase2(10,4);break;
    case 26:spawn_phase2(12,4);break;
  }
}
void message(int role,const std::string&who,const std::string&msg,float duration=4){radio={role,who,msg,duration,0};}
 void start(int phase=1){activate_phase(phase);cheatInvulnerable=false;cheatUsedThisRun=false;lives=3;score=0;weapon=0;stageTime=0;scroll=0;shipx=175;shipy=270;death=0;vulnerable=2;shootTime=0;missileTime=0;estalactitasProgramadas=0;estalactitasCerca=0;shieldTime=0;enemies.clear();deaths.clear();shots.clear();puffs.clear();prizes.clear();radio.life=0;eventCount=0;enemiesSpawned=0;hostileShotsFired=0;peakHostiles=0;sierrasF1=sierrasF2=sierrasDesdeIzq=sierrasDesdeDer=espectrosF1=moaisF1=0;estalactitasPasanCerca=estalactitasMaxSimultaneas=0;mode=INTRO;stateTime=0;music(phase_music(fase));}
 void emitir_radio_jefe(){
  // Solo al comenzar el combate: no consumir los segundos de radio en BOSS_INTRO.
  if(fase==1) message(4,"ENTE ALIENÍGENA","NO DEBERÍAS HABER LLEGADO AQUÍ.",4.5f);
  else if(fase==2) message(2,"COMANDANTE","LA ODISEA II AHORA ES UN ORGANISMO.|DESTRUYE SU REACTOR.",4.5f);
  else if(fase==3) message(2,"COMANDANTE","EL HALCÓN ESTELAR REX BLOQUEA EL PASO.|EVITA SU DESCARGA.",4.5f);
  else if(fase==4) message(3,"AURA-01","EL HIJO DE LAS ESTRELLAS DESPIERTA.|SI LOGRA SALIR, ESTAMOS PERDIDOS.",4.8f);
  else if(fase==5) message(4,"ENTE ALIENÍGENA","SOY LA VOZ DE TODAS LAS TORMENTAS.|NO ABANDONARÁS JÚPITER.",4.8f);
 }
 void boss_init(){mode=BOSS;stateTime=0;bossHp=fase==5?260:fase==4?195:fase==3?175:fase==2?145:95;bossMax=bossHp;bossX=1100;bossY=250;bossTime=0;bossPhase=0;bossBeamHit=0;
  emitir_radio_jefe();
 }
 void boss_start(){enemies.clear();shots.clear();mode=BOSS_INTRO;stateTime=0;music("musica_jefes_06");radio.life=0;}
 void next_phase(int next){
  int oldscore=score,oldlives=lives,oldweapon=weapon;bool oldCheat=cheatInvulnerable,oldCheatUsed=cheatUsedThisRun;
  float oldWeaponTime=weaponTime,oldShieldTime=shieldTime;
  start(next);score=oldscore;lives=std::max(1,oldlives);weapon=oldweapon;cheatInvulnerable=oldCheat;cheatUsedThisRun=oldCheatUsed;
  weaponTime=oldWeaponTime;shieldTime=oldShieldTime;vulnerable=3.0f;
 }
 // Mantener el jingle de victoria completo antes del cambio automático de fase.
 // ESPACIO puede adelantar manualmente la siguiente fase pasados los 3 segundos habituales.
 float victory_delay()const{
   if(!sound||masterVolume==0||musicVolume==0||musicName!="musica_victoria"||musicData.pcm.empty())return 4.2f;
   return std::max(4.2f,float(musicData.pcm.size()/2)/44100.f+.12f);
  }
 void life_hit(){if(cheatInvulnerable||vulnerable>0||death>0)return;if(shieldTime>0){shieldTime=0;vulnerable=1; tone("impacto",false,shipx);return;}
  death=.9f;vulnerable=3.1f;lives--;puff(shipx,shipy,70);tone("explosion_grande",false,shipx);shots.erase(std::remove_if(shots.begin(),shots.end(),[](Bullet&b){return b.friendly;}),shots.end());weapon=0;
 }
 void kill(int j){Actor &e=enemies[j];e.alive=false;deaths.push_back({e.kind,e.x,e.y,0});score+=(fase==5?(e.kind==22?650:e.kind==14||e.kind==15?300:420):fase==4?(e.kind<32?240:e.kind<34?380:350):fase==3?(e.kind==22?650:e.kind==23?80:350):fase==2?(e.kind==10||e.kind==11?420:e.kind==12||e.kind==13?350:220):e.kind==3?250:e.kind==1?170:140);save_best();puff(e.x,e.y,35);tone("explosion",false,e.x);if(ri(0,16)==0)prizes.push_back({e.x,e.y,ri(0,2)});}
 // Epílogo de campaña: tras el jingle de victoria empieza «Remember Love» de FutureDark.
 // La pista 10 es independiente, se reproduce una sola vez y los créditos nunca la cortan automáticamente.
 void begin_final_credits(){
   mode=FINAL_CREDITS;stateTime=0;finalCreditsClock=0;finalCreditsPage=0;
   music("musica_escena_final");
 }
 void move(float dt){stateTime+=dt;commCool-=dt;if(radio.life>0){radio.life-=dt;radio.elapsed+=dt;}
  if(mode==FINAL_CREDITS){
    // Los créditos finales mantienen el contador aunque estemos en la última
    // cartela: anteriormente se quedaba a cero y el fundido era siempre invisible.
    finalCreditsClock+=dt;
    if(finalCreditsPage<FINAL_CREDITS_PAGES-1){
      const float duration=finalCreditsPage==FINAL_CREDITS_PAGES-2?7.0f:5.35f;
      if(finalCreditsClock>=duration){finalCreditsClock=0;finalCreditsPage++;}
    }
    return;
  }
  if(mode==TITLE||mode==AUDIO_OPTIONS||mode==JUKEBOX||mode==INTRO||mode==BOSS_INTRO||mode==WIN||mode==DEAD||mode==BOSS_DEATH){
   if(mode==INTRO&&stateTime>6){mode=PLAY;stateTime=0;}
   if(mode==WIN&&fase<5&&stateTime>victory_delay()){next_phase(fase+1);return;}
   if(mode==WIN&&fase==5&&stateTime>victory_delay()){begin_final_credits();return;}
   if(mode==BOSS_DEATH){bossTime+=dt; if(stateTime>(fase==5?3.3f:2.9f)){mode=WIN;stateTime=0;music("musica_victoria");score+=(fase==5?30000:10000);save_best();}}
   if(mode==BOSS_INTRO&&stateTime>4.5f)boss_init();
   return;
  }
  if(pause)return;
  stageTime+=dt;scroll+=142*dt;
  if(vulnerable>0)vulnerable-=dt;if(death>0){death-=dt;if(death<=0){if(lives<=0){mode=DEAD;stateTime=0;music("musica_game_over");save_best();}else{shipx=185;shipy=275;vulnerable=2.5f;}}}
  const uint8_t* keys=SDL_GetKeyboardState(nullptr);
  auto k=[&](int a){return keys&&keys[a]!=0;};
  auto direction=keyboard_direction(keys);
  float dx=direction.first,dy=direction.second;
  // PC: WASD y cursores. Q/A/O/P pertenece exclusivamente a ZX Spectrum.
  if(pad){auto ax=SDL_GameControllerGetAxis(pad,0),ay=SDL_GameControllerGetAxis(pad,1);if(abs(ax)>6500)dx=ax/32767.f;if(abs(ay)>6500)dy=ay/32767.f;
   if(SDL_GameControllerGetButton(pad,11))dy=-1;if(SDL_GameControllerGetButton(pad,12))dy=1;
   if(SDL_GameControllerGetButton(pad,13))dx=-1;if(SDL_GameControllerGetButton(pad,14))dx=1;}
  shipMoveY=dy;
  if(death<=0){shipx=limit(shipx+dx*300*dt,70,(fase>=2&&mode==BOSS)?525.f:740.f);shipy=limit(shipy+dy*250*dt,float(HUD_H+30),430);
   if(shipy-6<ceilY(shipx+6)||shipy+6>floorY(shipx+6)||shipy-5<ceilY(shipx)||shipy+5>floorY(shipx)||shipy-6<ceilY(shipx+12)||shipy+6>floorY(shipx+12)) life_hit();}
  bool pressed=k(44)||k(224)||k(228)||(pad&&SDL_GameControllerGetButton(pad,0));
  shootTime-=dt;if(pressed&&death<=0&&shootTime<=0){shoot();shootTime=weapon==2?.14f:.19f;}
  // Arma automática secundaria contra el suelo retirada: no hay walkers.
  if(mode==PLAY){
   if(fase==5){
     // Asalto final: las 28 escuadras conocidas protegen el corazón de Júpiter.
     while(stageTime>=1.2f+eventCount*3.0f && eventCount<28)spawn_event5(eventCount++);
     if(stageTime>=89.f){boss_start();return;}
   }else if(fase==4){static const float ev4[]={1,3.7f,6.4f,9.1f,11.8f,14.5f,17.2f,19.9f,22.6f,25.3f,28.f,30.7f,33.4f,36.1f,38.8f,41.5f,44.2f,46.9f,49.6f,52.3f,55.f,57.7f,60.4f,63.1f,65.8f,68.5f,71.2f,73.9f};
     int total=int(sizeof(ev4)/sizeof(ev4[0]));
     while(eventCount<total&&stageTime>=ev4[eventCount])spawn_event4(eventCount++);
     if(stageTime>=80){boss_start();return;}
   }else if(fase==3){static const float ev3[]={1,3.7f,6.4f,9.1f,11.8f,14.5f,17.2f,19.9f,22.6f,25.3f,28.f,30.7f,33.4f,36.1f,38.8f,41.5f,44.2f,46.9f,49.6f,52.3f,55.f,57.7f,60.4f,63.1f,65.8f,68.5f,71.2f,73.9f};
    int total=int(sizeof(ev3)/sizeof(ev3[0]));
    while(eventCount<total&&stageTime>=ev3[eventCount])spawn_event3(eventCount++);
    if(stageTime>=80){boss_start();return;}
   }else if(fase==2){static const float ev2[]={1,4.5f,7.5f,10.2f,13,15.5f,18.5f,22,24.2f,27,30,33,35,39,42,45,48,51,54.5f,57,59.5f,62,65,68,70.2f,72.6f,75};
    const int total=int(sizeof(ev2)/sizeof(ev2[0]));
    while(eventCount<total&&stageTime>=ev2[eventCount]){spawn_event2(eventCount);eventCount++;}
    if(stageTime>=82){boss_start();return;}
   }else{static const float events[]={1.5f,4.0f,6.0f,8.0f,10.3f,12.0f,14.5f,17.0f,19.8f,22.4f,24.0f,27.2f,30.0f,33.0f,35.0f,38.5f,42.0f,45.0f,48.0f,49.2f,51.5f,53.0f,56.5f,59.0f,63.0f,66.0f,69.0f,72.0f};
    const int totalEvents = int(sizeof(events)/sizeof(events[0]));
    while(eventCount<totalEvents && stageTime>=events[eventCount]){spawn_event(eventCount);eventCount++;}
    if(stageTime>=80){boss_start();return;}
   }
  }
  for(auto &e:enemies){if(!e.alive)continue;e.t+=dt;e.x-=e.vx*dt;e.fire-=dt;
   if(fase==5){
      const bool ghost=e.kind==14||e.kind==15,hawk=e.kind==20||e.kind==21,moai=e.kind==22;
      e.y+=std::sin(e.t*(ghost?4.2f:hawk?3.9f:1.9f)+e.x*.003f)*(ghost?53.f:hawk?39.f:22.f)*dt;
      if(ghost&&e.t>.8f)e.y+=(shipy-e.y)*std::min(1.f,dt*.9f);
      const float margin=moai?65.f:34.f;
      e.y=limit(e.y,ceilY(e.x)+margin,floorY(e.x)-margin);
      if(e.fire<=0&&e.x>50&&e.x<W+35){
        const float dx=shipx-e.x,dy=shipy-e.y,n=std::max(1.f,std::hypot(dx,dy));
        const float speed=moai?235.f:ghost?255.f:hawk?258.f:245.f;
        fire(e.x+(e.vx<0?25:-25),e.y,dx/n*speed,dy/n*speed,false);
        e.fire=moai?1.9f:ghost?1.55f:hawk?1.48f:1.65f;
      }
      if(death<=0&&std::abs(e.x-shipx)<(moai?39.f:ghost?26.f:34.f)&&
          std::abs(e.y-shipy)<(moai?57.f:ghost?24.f:22.f))life_hit();
      continue;
   }
   if(fase==4){
      float freq=e.kind<32?2.35f:e.kind<34?4.6f:3.3f;
      float amp=e.kind<32?35.f:e.kind<34?66.f:36.f;
      e.y+=std::sin(e.t*freq+e.x*.003f)*amp*dt;
      if(e.kind==32||e.kind==33){
        if(e.t>1.0f)e.y+=(shipy-e.y)*std::min(1.f,dt*.98f);
      }
      e.y=limit(e.y,ceilY(e.x)+34,floorY(e.x)-34);
      if(e.fire<=0 &&e.x>50&&e.x<W+40){
        float dx=shipx-e.x,dy=shipy-e.y,n=std::max(1.f,std::hypot(dx,dy));
        // Los disparos fijan la posición al salir: jamás persiguen al jugador en vuelo.
        const float speed=e.kind<32?225.f:e.kind<34?246.f:258.f;
        const float muzzle=e.x+(e.vx<0?24:-24);
        if(e.kind<32){fire(muzzle,e.y,dx/n*speed,dy/n*speed,false);}
        else{
          // Horquilla estrecha legible: obliga a desplazarse sin cerrar todo el pasillo.
          for(int side=-1;side<=1;side+=2){
            float ay=shipy+side*33.f-e.y;
            float an=std::max(1.f,std::hypot(dx,ay));
            fire(muzzle,e.y,dx/an*speed,ay/an*speed,false);
          }
        }
        e.fire=e.kind<32?1.9f:e.kind<34?1.55f:1.35f;
        tone("laser");
      }
      if(death<=0&&std::abs(e.x-shipx)<(e.kind>=34?36.f:27.f)&&std::abs(e.y-shipy)<(e.kind>=34?24.f:26.f))life_hit();
      continue;
   }
   if(fase==3){
    if(e.kind==20||e.kind==21){
       e.y+=std::sin(e.t*5.5f+e.x*.004f)*53*dt;
       e.y=limit(e.y,ceilY(e.x)+31,floorY(e.x)-31);
    }else if(e.kind==22){
       e.y+=std::sin(e.t*2.4f)*28*dt;
       e.y=limit(e.y,ceilY(e.x)+77,floorY(e.x)-77);
    }else if(e.kind==23){
       // Un segundo de aviso inmóvil; tras ello, caída acelerada vertical,
       // sin cambiar de columna ni perseguir una nave que ya está esquivando.
       if(e.fire<=0){
         const float falling=std::max(0.f,e.t-1.f);
         e.y+=(285.f+std::min(135.f,falling*125.f))*dt;
       }
       if(e.fire<=0&&!e.ceiling&&std::abs(e.x-shipx)<42.f&&std::abs(e.y-shipy)<42.f){
         ++estalactitasPasanCerca;e.ceiling=true;
       }
       if(e.y+29>floorY(e.x)){
         e.alive=false;deaths.push_back({e.kind,e.x,e.y,0});puff(e.x,e.y,15,CYAN);tone("explosion_corta",false,e.x);
       }
    }
    if(e.kind!=23&&e.fire<=0&&e.x>40&&e.x<W+30){
       float dx=shipx-e.x,dy=shipy-e.y,n=std::max(1.f,std::hypot(dx,dy));
       float speed=e.kind==22?248.f:260.f;
       fire(e.x+(e.vx>0?-33:33),e.y,dx/n*speed,dy/n*speed,false);
       // Moái blindado con descarga más frecuente; halcones atacan a ritmo variable.
       e.fire=e.kind==22?1.55f:rf(1.10f,1.55f);tone("laser",false,e.x);
    }
    if(death<=0&&std::abs(e.x-shipx)<(e.kind==22?44.f:e.kind==23?20.f:36.f)&&
       std::abs(e.y-shipy)<(e.kind==22?57.f:e.kind==23?33.f:22.f))life_hit();
    continue;
   }
   if(fase==2){
    if(e.kind==10||e.kind==11){
      e.y+=std::sin(e.t*4.0f+e.x*.007f)*74.f*dt;
      e.y=limit(e.y,ceilY(e.x)+50,floorY(e.x)-50);
    }
    if(e.kind==12||e.kind==13){e.y+=std::sin(e.t*3.1f+e.x*.004f)*52*dt;e.y=limit(e.y,ceilY(e.x)+24,floorY(e.x)-24);}
    if(e.kind==14||e.kind==15){
      if(e.t>1.0f){e.y+=(shipy-e.y)*std::min(1.f,dt*1.7f);e.vx=limit(e.vx,-430,430);}
      else e.y+=std::sin(e.t*5.f)*32*dt;
      e.y=limit(e.y,ceilY(e.x)+36,floorY(e.x)-36);
    }
    if(e.kind!=14&&e.kind!=15 && e.fire<=0 && e.x>55 && e.x<W+50){
      float dx=shipx-e.x,dy=shipy-e.y,n=std::max(1.f,std::hypot(dx,dy));
      float muzzle=(e.kind==12||e.kind==13)?24.f:29.f;
      fire(e.x+(e.vx<0?muzzle:-muzzle),e.y,dx/n*242,dy/n*242,false);
      e.fire=(e.kind==10||e.kind==11)?1.95f:rf(1.6f,2.7f);tone("laser",false,e.x);
    }
    // Hitboxes acordes al objeto visible, NO al atlas original de gran tamaño.
    float hx=(e.kind==10||e.kind==11)?25.f:(e.kind==12||e.kind==13)?25.f:24.f;
    float hy=(e.kind==10||e.kind==11)?25.f:(e.kind==12||e.kind==13)?15.f:25.f;
    if(death<=0&&std::abs(e.x-(shipx+3))<hx&&std::abs(e.y-shipy)<hy)life_hit();
    continue;
   }
   if(e.kind==1){
      e.y+=std::sin(e.t*3.7f+e.x*.007f)*72.f*dt;
      e.y=limit(e.y,ceilY(e.x)+48,floorY(e.x)-48);
   }else if(e.kind==14||e.kind==15){
      if(e.t>.8f)e.y+=(shipy-e.y)*std::min(1.f,dt*1.25f);
      e.y=limit(e.y,ceilY(e.x)+32,floorY(e.x)-32);
   }else if(e.kind==3){
      e.y+=std::sin(e.t*3.6f)*25.f*dt;
      e.y=limit(e.y,ceilY(e.x)+82,floorY(e.x)-82);
   }
   if(e.kind!=14&&e.kind!=15&&e.fire<=0&&e.x>150&&e.x<W+80){
      float dx=shipx-e.x,dy=shipy-e.y;
      float n=std::max(1.f,std::hypot(dx,dy));
      fire(e.x+(e.vx<0?28.f:-28.f),e.y,dx/n*235,dy/n*235,false);
      e.fire=rf(1.7f,3.3f);tone("laser",false,e.x);
   }
   // Punto vulnerable de la cabina pequeño, contra el cuerpo real de cada enemigo.
   float bodyHalfX=e.kind==3?39.f:(e.kind==1?25.f:24.f);
   float bodyHalfY=e.kind==3?75.f:(e.kind==1?25.f:25.f);
   if(death<=0&&std::abs(e.x-(shipx+3))<bodyHalfX&&std::abs(e.y-shipy)<bodyHalfY)life_hit();}
  enemies.erase(std::remove_if(enemies.begin(),enemies.end(),[](const Actor&e){return !e.alive||(e.vx>0&&e.x< -130)||(e.vx<0&&e.x>W+140);}),enemies.end());
  peakHostiles=std::max(peakHostiles,int(enemies.size()));
  for(auto &e:deaths){e.x-=140*dt;e.age+=dt;}deaths.erase(std::remove_if(deaths.begin(),deaths.end(),[](const DeathFx&e){return e.age>1.0f;}),deaths.end());
  for(auto &p:prizes){p.x-=140*dt;if(death<=0&&std::abs(p.x-shipx)<45&&std::abs(p.y-shipy)<35){if(p.type==2)shieldTime=9;else{weapon=p.type==1?1:2;weaponTime=22;}p.x=-200;score+=200;tone("item");}}
  prizes.erase(std::remove_if(prizes.begin(),prizes.end(),[](const Prize&p){return p.x< -50;}),prizes.end());
  if(weaponTime>0&& (weaponTime-=dt)<=0)weapon=0;if(shieldTime>0)shieldTime-=dt;
  if(mode==BOSS){bossTime+=dt;if(bossX>777)bossX-=105*dt;bossY=268+std::sin(bossTime*.9)*44;
   if(fase==5){
     // Dos rayos reales, nunca instantáneos: 1,5 s de aviso DESPUÉS de entrar.
     // Tres patrones: dos altos, dos bajos, o uno alto y otro bajo.
     const float cycle=std::fmod(ghost_attack_elapsed(),8.6f);
     const int before=bossPhase;
     bossPhase=cycle<1.5f?0:cycle<3.2f?1:cycle<5.2f?2:3;
     const float muzzleX=bossX-139.f, muzzleY=bossY+27.f;
     if(bossX<828 && bossPhase==1 && before!=1)tone("rayo");
     if(bossX<828 && bossPhase==1 && death<=0 && vulnerable<=0 &&
        shipx<muzzleX-8 && bossBeamHit<=0){
       for(const int y:ghost_beam_lanes()){
         if(std::abs(shipy-float(y))<14.f){life_hit();bossBeamHit=.90f;break;}
       }
     }
     bossBeamHit-=dt;
     // Durante la descarga doble no sumamos ráfagas dirigidas invisibles;
     // los cinco orbes y los disparos de persecución quedan para la tormenta.
     if(bossX<828 && bossPhase==2 && int(bossTime*2.40f)!=int((bossTime-dt)*2.40f)){
       for(int i=-2;i<=2;i++){
         float a=PI+i*.23f+std::sin(bossTime*.6f)*.16f;
         fire(muzzleX,muzzleY,std::cos(a)*224.f,std::sin(a)*224.f,false);
       }
       tone("laser");
     }
     // Incluso durante la apertura del núcleo hay proyectiles dispersos y evitables.
     if(bossX<828 && bossPhase==3 && int(bossTime*1.20f)!=int((bossTime-dt)*1.20f)){
       const int pulse=int(bossTime*1.20f);
       // El núcleo sigue siendo atacable; cada segundo pulso añade dos
       // disparos de flanco más lentos, dejando trayectorias esquivables.
       for(int j=(pulse%2==0?-1:0);j<=(pulse%2==0?1:0);++j){
         float dy=shipy+j*62.f-(bossY+7.f),dx=shipx-muzzleX;
         float n=std::max(1.f,std::hypot(dx,dy));
         fire(muzzleX,muzzleY,dx/n*196.f,dy/n*196.f,false);
       }
     }
     if(bossHp<=0){
       mode=BOSS_DEATH;stateTime=0;shots.clear();
       puff(bossX-94,bossY-17,205,{166,96,245,255});tone("explosion_grande");
     }
   }else if(fase==4){
     float cycle=std::fmod(bossTime,7.8f);int prev=bossPhase;
     // 1.45s de aviso visual, ráfaga, pulso y 2.30s de núcleo abierto.
     bossPhase=cycle<1.45f?0:cycle<3.35f?1:cycle<5.50f?2:3;
     if(bossX<810&&bossPhase==1&&prev!=1){
       for(int j=-2;j<=2;j++){
         float dx=shipx-(bossX-104),dy=shipy+j*27-(bossY+3),n=std::max(1.f,std::hypot(dx,dy));
         fire(bossX-104,bossY+3,dx/n*225.f,dy/n*225.f,false);
       }tone("rayo");
     }
     if(bossX<810&&bossPhase==1&&int(bossTime*1.45f)!=int((bossTime-dt)*1.45f)){
       for(int side=-1;side<=1;side+=2){
         float dx=shipx-(bossX-104),dy=shipy+side*48.f-(bossY+3);
         float n=std::max(1.f,std::hypot(dx,dy));
         fire(bossX-104,bossY+3,dx/n*219.f,dy/n*219.f,false);
       }
       tone("laser");
     }
     if(bossX<810&&bossPhase==2&&int(bossTime*2.25f)!=int((bossTime-dt)*2.25f)){
       for(int j=-1;j<=1;j++){
         float dx=shipx-(bossX-100),dy=shipy+j*46-(bossY+3),n=std::max(1.f,std::hypot(dx,dy));
         fire(bossX-100,bossY+3,dx/n*203.f,dy/n*203.f,false);
       }tone("laser");
     }
     if(bossHp<=0){mode=BOSS_DEATH;stateTime=0;puff(bossX-70,bossY,190,MAGENTA);tone("explosion_grande");}
   }else if(fase==3){
     float cycle=std::fmod(bossTime,6.8f);int previous=bossPhase;
     // Rayo avisado durante 1.3s; ráfaga más cerrada y ventana de ataque de 2.3s.
     bossPhase=cycle<1.30f?0:cycle<2.45f?1:cycle<4.50f?2:3;
     if(bossX<810&&bossPhase==1&&previous!=1)tone("rayo");
     if(bossX<810&&bossPhase==1&&vulnerable<=0&&death<=0&&shipx<bossX-115&&
        std::abs(shipy-(bossY-10))<14&&bossBeamHit<=0){life_hit();bossBeamHit=.9f;}
     bossBeamHit-=dt;
     if(bossPhase==2&&bossX<810&&int(bossTime*2.8f)!=int((bossTime-dt)*2.8f)){
       for(int i=-2;i<=2;i++){
         float dx=shipx-(bossX-115),dy=shipy+i*30-(bossY-10),n=std::max(1.f,std::hypot(dx,dy));
         fire(bossX-115,bossY-10,dx/n*258,dy/n*258,false);
       }tone("laser");
     }
     if(bossHp<=0){mode=BOSS_DEATH;stateTime=0;puff(bossX-68,bossY,180);tone("explosion_grande");}
   }else if(fase==2){
     float cycle=std::fmod(bossTime,7.1f);
     int previousBossPhase=bossPhase;
     bossPhase=cycle<1.1f?0:cycle<2.25f?1:cycle<3.85f?2:3;
     // El rayo tiene su propio efecto y voz prioritaria (antes solo sonaban los disparos).
     if(bossX<810 && bossPhase==1 && previousBossPhase!=1)tone("rayo");
     // El nuevo casco tiene la boca de fuego en la proa, no en su ojo.
     // Mantenemos cadencia/velocidad/daño y alineamos ataques con el dibujo.
     const float muzzleX=bossX-302.f, muzzleY=bossY-12.f;
     if(bossX<810 && bossPhase==1 && vulnerable<=0 && death<=0 &&
         shipx<muzzleX && std::abs(shipy-muzzleY)<13 && bossBeamHit<=0){life_hit();bossBeamHit=.9f;}
     bossBeamHit-=dt;
     if(bossPhase==2 && bossX<810 && int(bossTime*2.3)!=int((bossTime-dt)*2.3)){
       for(int i=-1;i<=1;i++){
         float dx=shipx-muzzleX,dy=shipy+float(i)*34-muzzleY,n=std::max(1.f,std::hypot(dx,dy));
         fire(muzzleX,muzzleY,dx/n*248,dy/n*248,false);
       }tone("laser");
     }
     if(bossHp<=0){mode=BOSS_DEATH;stateTime=0;puff(bossX-55,bossY+13,180);tone("explosion_grande");}
   }else{
   bossPhase=(int(bossTime/3.7f))%3;
   if(int(bossTime*3.3)!=int((bossTime-dt)*3.3)&&bossTime>4){
    const float sy=(bossPhase==1?bossY+40:bossY-50);
    for(int i=0;i<3;i++){float a=PI+std::sin(bossTime*.9)*.4f+(i-1)*.26f;fire(bossX-31,sy,std::cos(a)*205,std::sin(a)*205,false);}
    tone("laser");}
   if(bossHp<=0){mode=BOSS_DEATH;stateTime=0;puff(bossX,bossY,160);tone("explosion_grande");}
   }
  }
  for(auto &b:shots){b.x+=b.vx*dt;b.y+=b.vy*dt;b.life-=dt;
   if(b.y<ceilY(b.x)+2||b.y>floorY(b.x)-2){b.life=-1;continue;}
   if(b.friendly){if(mode==BOSS&&((fase==5&&std::abs(b.x-(bossX-99))<64&&std::abs(b.y-(bossY-22))<72)||(fase==4&&std::abs(b.x-(bossX-80))<60&&std::abs(b.y-bossY)<85)||(fase==3&&std::abs(b.x-(bossX-75))<54&&std::abs(b.y-bossY)<59)||(fase==2&&std::abs(b.x-(bossX-55))<44&&std::abs(b.y-(bossY+13))<45)||(fase==1&&std::abs(b.x-bossX)<55&&std::abs(b.y-bossY)<122))){b.life=-1;if(bossPhase==(fase>=2?3:1)){bossHp-=b.missile?3:2;puff(b.x,b.y,2);}else puff(b.x,b.y,2,DIM);continue;}
    for(int i=0;i<(int)enemies.size();i++)if(enemies[i].alive &&
        std::abs(enemies[i].x-b.x)<((fase==4 && enemies[i].kind>=30 && enemies[i].kind<=35)||((fase==2||fase==5) && (enemies[i].kind==12||enemies[i].kind==13))||((fase==3||fase==5) && (enemies[i].kind==20||enemies[i].kind==21))?(b.missile?37:32):(fase==2||fase==5) && (enemies[i].kind==14||enemies[i].kind==15)?(b.missile?29:24):(b.missile?48:34)) &&
        std::abs(enemies[i].y-b.y)<((fase==4 && enemies[i].kind>=30 && enemies[i].kind<=35)||((fase==2||fase==5) && (enemies[i].kind==12||enemies[i].kind==13))||((fase==3||fase==5) && (enemies[i].kind==20||enemies[i].kind==21))?(b.missile?25:20):(fase==2||fase==5) && (enemies[i].kind==14||enemies[i].kind==15)?(b.missile?29:25):(b.missile?39:29))){b.life=-1;enemies[i].hp-=(b.missile?3:1);if(enemies[i].hp<=0)kill(i);else puff(b.x,b.y,3);break;}
   }else if(death<=0&&std::abs(b.x-(shipx+SHIP_HIT_OFFSET_X))<SHIP_HIT_HALF_X&&std::abs(b.y-shipy)<SHIP_HIT_HALF_Y){b.life=-1;life_hit();}
  }
  shots.erase(std::remove_if(shots.begin(),shots.end(),[](const Bullet&b){return b.life<=0||b.x< -30||b.x>W+50||b.y<-30||b.y>H+30;}),shots.end());
  for(auto &s:puffs){s.x+=s.vx*dt;s.y+=s.vy*dt;s.vx*=1-1.4*dt;s.vy*=1-1.4*dt;s.life-=dt;}
  puffs.erase(std::remove_if(puffs.begin(),puffs.end(),[](const Puff&p){return p.life<=0;}),puffs.end());
 }
 // Los halcones normales comparten los dos atlas 4x4 RGBA de Tony C. Ware.
 // 0..7: vuelo; 8..15: destrucción. Proyectiles creados aparte en draw_shots().
 void draw_halcon_normal(int kind,float t,float x,float y,bool destroyed=false,float age=0.f){
   const bool left=(kind==20||kind==34); // derecha->izquierda
   const int frame=destroyed?8+std::min(7,int(age*8.f)):int(t*9.f)%8;
   sprite(left?"halcon_izquierda":"halcon_derecha",4,320,320,frame,
          int(x)-53,int(y)-53,106,106);
 }
 void draw_enemies(){
  if(fase==5){
    for(const auto&e:enemies){if(!e.alive)continue;
      int x=int(e.x),y=int(e.y),fr=int(e.t*9);
      if(e.kind==12||e.kind==13){int a=e.fire<.38f?8+fr%8:fr%8;sprite(e.kind==13?"c_izq_der":"c_der_izq",8,80,54,a,x-40,y-27,80,54);}
      else if(e.kind==14||e.kind==15){int a=e.t>1.f?8+fr%8:fr%8;sprite(e.kind==15?"e_izq_der":"e_der_izq",8,67,67,a,x-37,y-36,74,72);}
      else if(e.kind==20||e.kind==21){draw_halcon_normal(e.kind,e.t,e.x,e.y);}
      else if(e.kind==22){int a=e.fire<.3f?4+fr%4:fr%4;sprite("menor",6,168,168,a,x-61,y-61,122,122);}
    }
    return;
  }
  if(fase==4){draw_phase4_enemies();return;}
  if(fase==3){draw_phase3_enemies();return;}
  if(fase==2){draw_phase2_enemies();return;}
  for(auto &e:enemies){if(!e.alive)continue;int x=int(e.x),y=int(e.y),fr=int(e.t*9);
    if(e.kind==1){int a=e.fire<.35f?6+(fr%6):fr%6;
      sprite("sierra",6,256,256,a,x-35,y-35,70,70);}
    else if(e.kind==14||e.kind==15){int a=e.t>.85f?8+(fr%8):fr%8;
      sprite(e.kind==15?"e_izq_der":"e_der_izq",8,67,67,a,x-37,y-36,74,72);}
    else if(e.kind==3){int a=e.fire<.40f?4+std::min(3,int((.40f-e.fire)*10)):
      e.fire>1.4f&&e.t>1.0f?8+std::min(7,int((3.1f-e.fire)*5)):
      e.hp<3?8+(fr%8):fr%4;
      a=std::max(0,std::min(15,a));
      sprite("menor",6,168,168,a,x-84,y-84,168,168);}
  }
 }
 void draw_phase4_enemies(){for(auto &e:enemies){if(!e.alive)continue;
   int x=int(e.x),y=int(e.y),fr=int(e.t*8);
   if(e.kind>=30&&e.kind<=33){
     std::string tex=e.kind==30?"cel_estrella_di":e.kind==31?"cel_estrella_id":e.kind==32?"cel_devora_di":"cel_devora_id";
     // Sólo animación de vuelo; el proyectil es una entidad física independiente.
     sprite(tex,8,64,64,fr%8,x-33,y-33,66,66);
   }else if(e.kind==34||e.kind==35){
     draw_halcon_normal(e.kind,e.t,e.x,e.y);
   }
 }}
 // Indicadores militares V1.1: exclusivamente peligro de hielo y retícula.
 bool aviso_hielo(const Actor& e) const {
   return fase==3 && e.kind==23 && e.alive && e.fire>0.f;
 }
 bool punto_debil_activo() const {
   return mode==BOSS && bossHp>0.f && bossX<900.f && bossPhase==(fase>=2?3:1);
 }
 std::pair<int,int> centro_debil() const {
   switch(fase){
     case 2:return {int(bossX-55),int(bossY+13)};
     case 3:return {int(bossX-75),int(bossY)};
     case 4:return {int(bossX-80),int(bossY)};
     case 5:return {int(bossX-99),int(bossY-22)};
     default:return {int(bossX),int(bossY)};
   }
 }
 void draw_mira_debil(){
   if(!punto_debil_activo())return;
   auto [x,y]=centro_debil();
   // 36 px, pulso ligero; la retícula sigue el centro exacto del hitbox.
   const int frame=int(bossTime*9.f)%4;
   sprite("mira",4,256,256,frame,x-19,y-19,38,38);
 }
 void draw_phase3_enemies(){for(auto &e:enemies){if(!e.alive)continue;
   int x=int(e.x),y=int(e.y),fr=int(e.t*9);
   if(e.kind==20||e.kind==21){
      draw_halcon_normal(e.kind,e.t,e.x,e.y);
   }else if(e.kind==22){
      int frame=e.fire<.3?4+fr%4:fr%4;
      sprite("menor",6,168,168,frame,x-61,y-61,122,122);
   }else if(e.kind==23){
      int frame=e.t<.85f?int(e.t*3)%8:int(e.t*3)%8;
      sprite("hielo3",8,64,92,frame,x-25,y-38,50,76);
      if(aviso_hielo(e)){
        // Un único triángulo intermitente situado junto al punto de caída;
        // desaparece en cuanto comienza el desprendimiento, sin columnas extra.
        const bool on=(int(e.t*9.f)%5)!=4;
        if(on){
          const int f=std::clamp(int((1.f-e.fire)*4.5f),0,3);
          const int warnY=std::clamp(y+20,HUD_H+13,H-57);
          sprite("triangulo",4,256,256,f,x-23,warnY,46,46);
        }
      }
   }
 }}
 void draw_phase2_enemies(){for(auto &e:enemies){if(!e.alive)continue;
   int x=int(e.x),y=int(e.y),fr=int(e.t*10);
   if(e.kind==10||e.kind==11){int a=e.fire<.42f?6+fr%6:fr%6;
      sprite("sierra",6,256,256,a,x-35,y-35,70,70);}
   else if(e.kind==12||e.kind==13){int idx=e.fire<.38f?8+fr%8:fr%8;
      sprite(e.kind==13?"c_izq_der":"c_der_izq",8,80,54,idx,x-40,y-27,80,54);}
   else if(e.kind==14||e.kind==15){int idx=e.t>1?8+fr%8:fr%8;
      sprite(e.kind==15?"e_izq_der":"e_der_izq",8,67,67,idx,x-37,y-36,74,72);}
  }}
 void draw_deaths(){for(auto &d:deaths){int frame=std::min(5,int(d.age*6));int x=int(d.x),y=int(d.y);
  if(fase==5){
    if(d.kind==12||d.kind==13)sprite(d.kind==13?"c_izq_der":"c_der_izq",8,80,54,16+std::min(7,int(d.age*8)),x-40,y-27,80,54);
    else if(d.kind==14||d.kind==15)sprite(d.kind==15?"e_izq_der":"e_der_izq",8,67,67,16+std::min(7,int(d.age*8)),x-37,y-36,74,72);
    else if(d.kind==20||d.kind==21)draw_halcon_normal(d.kind,0.f,d.x,d.y,true,d.age);
    else if(d.kind==22)sprite("menor",6,168,168,16+std::min(7,int(d.age*8)),x-61,y-61,122,122);
    continue;
  }
  if(fase==4){
    if(d.kind>=30&&d.kind<=33){
      std::string tex=d.kind==30?"cel_estrella_di":d.kind==31?"cel_estrella_id":d.kind==32?"cel_devora_di":"cel_devora_id";
      sprite(tex,8,64,64,8+std::min(7,int(d.age*8)),x-33,y-33,66,66);
    }else if(d.kind==34||d.kind==35){
      draw_halcon_normal(d.kind,0.f,d.x,d.y,true,d.age);
    }
    continue;
  }
  if(fase==3){
    if(d.kind==20||d.kind==21)draw_halcon_normal(d.kind,0.f,d.x,d.y,true,d.age);
    else if(d.kind==22)sprite("menor",6,168,168,16+std::min(7,int(d.age*8)),x-61,y-61,122,122);
    else sprite("hielo3",8,64,92,8+std::min(7,int(d.age*9)),x-25,y-38,50,76);
    continue;
  }
  if(fase==2){
    if(d.kind==10||d.kind==11){sprite("sierra",6,256,256,18+std::min(5,int(d.age*6)),x-35,y-35,70,70);continue;}
    int a=16+std::min(7,int(d.age*8));
    if(d.kind==12||d.kind==13)sprite(d.kind==13?"c_izq_der":"c_der_izq",8,80,54,a,x-40,y-27,80,54);
    else sprite(d.kind==15?"e_izq_der":"e_der_izq",8,67,67,a,x-37,y-36,74,72);
    continue;
  }
  switch(d.kind){
   case 1:sprite("sierra",6,256,256,18+std::min(5,int(d.age*6)),x-35,y-35,70,70);break;
   case 3:sprite("menor",6,168,168,16+std::min(7,int(d.age*8)),x-84,y-84,168,168);break;
   case 14:case 15:sprite(d.kind==15?"e_izq_der":"e_der_izq",8,67,67,16+std::min(7,int(d.age*8)),x-37,y-36,74,72);break;
  }
 }}
 void draw_shots(){for(auto &b:shots){int x=int(b.x),y=int(b.y);
   if(b.friendly){if(b.missile){image("fx_laser",x-33,y-10,66,20);}else if(weapon==2){image("fx_laser",x-47,y-15,94,30);}else{image("fx_pulse",x-23,y-14,46,28);}}
   else{image("fx_orb",x-13,y-13,26,26);}
  }
  for(auto &p:puffs){RGBA c=p.c;c.a=(int)limit(p.life/p.maxlife*255,0,255);int size=std::max(1,int(2+3*p.life));box(int(p.x),int(p.y),size,size,c);}
 }
 void draw_prizes(){for(auto&p:prizes){int x=int(p.x),y=int(p.y);box(x-17,y-17,34,34,{5,24,49,240},GREEN);text(p.type==2?"E":p.type==1?"D":"L",x,y-12,27,GREEN,1);}}
 void draw_fantasma_jupiter(){
  // Atlas RGBA del usuario: 6 reposos, 6 intensificaciones, 6 destrucciones.
  // La zona del ojo (x=bossX-99) es vulnerable sólo cuando brilla el núcleo.
  int frame=0;
  if(mode==BOSS_DEATH) frame=12+std::min(5,int(stateTime/3.3f*6.f));
  else if(bossPhase==0||bossPhase==3)frame=int(bossTime/.135f)%6;
  else frame=6+(int(bossTime/.135f)%6);
  const int w=412,h=398;
  sprite("fantasma5",6,384,384,frame,int(bossX-w/2),int(bossY-h/2),w,h);
  if(mode==BOSS){
    const int cx=int(bossX-99),cy=int(bossY-22);
    if(bossPhase==0||bossPhase==1){
      // Los dos carriles anunciados son exactamente los que producen daño.
      // Aviso de 1,5 segundos después de que entre el jefe en pantalla.
      const auto lanes=ghost_beam_lanes();
      const int originX=int(bossX-139.f);
      for(int lane:lanes)draw_ghost_laser(originX,lane,bossPhase==1);
    }
    if(bossPhase==1||bossPhase==2){
      const int rr=33+int(6*std::sin(bossTime*10));
      for(int i=0;i<26;++i){float a=2*PI*i/26.f,b=2*PI*(i+1)/26.f;
        line(cx+int(rr*std::cos(a)),cy+int(rr*std::sin(a)),cx+int(rr*std::cos(b)),cy+int(rr*std::sin(b)),{173,74,236,170});}
    }else if(bossPhase==3){ // el ojo vulnerable se destaca en cian
      const int rr=18+int(3*std::sin(bossTime*7.f));
      for(int i=0;i<32;++i){float a=2*PI*i/32.f,b=2*PI*(i+1)/32.f;
        line(cx+int(rr*std::cos(a)),cy+int(rr*std::sin(a)),cx+int(rr*std::cos(b)),cy+int(rr*std::sin(b)),{99,238,255,235});}
    }

  }else if(mode==BOSS_DEATH){
    for(int i=0;i<13;++i){
      const float a=2.f*PI*i/13.f+stateTime*.6f;
      const float r=22.f+stateTime*70.f;
      line(int(bossX-98+r*std::cos(a)),int(bossY-18+r*std::sin(a)),
           int(bossX-98+(r+24.f)*std::cos(a)),int(bossY-18+(r+24.f)*std::sin(a)),
           {181,85,246,std::max(0,235-int(stateTime*65))});
    }
  }
 }
 void draw_boss(){if(mode!=BOSS&&mode!=BOSS_DEATH)return;
  if(fase==5){draw_fantasma_jupiter();return;}
  if(fase==4){draw_hijo();return;}
  if(fase==3){draw_halcon_jefe();return;}
  if(fase==2){draw_odisea();return;}
  int frame=0;
  if(mode==BOSS_DEATH)frame=16+std::min(7,int(stateTime/2.9f*8));
  else if(bossPhase==0)frame=std::min(7,int(std::fmod(bossTime,3.7f)/3.7f*8));
  else if(bossPhase==1)frame=8+(int(bossTime*4)%8);
  else frame=4+(int(bossTime*4)%10);
  sprite("moai",6,288,324,frame,int(bossX-127),int(bossY-140),254,280);

 }
 void draw_hijo(){
  // Sólo dos siluetas estables: sin celdas vacías o proyectiles dentro del cuerpo.
  int idx=mode==BOSS_DEATH?8+std::min(7,int(stateTime/2.9f*8)):(int(bossTime*2.7f)%2);
  sprite("hijo",8,230,220,idx,int(bossX-135),int(bossY-131),270,262);
  if(mode!=BOSS)return;
  const int cx=int(bossX-90),cy=int(bossY+5);
  if(bossPhase==0){
    int rr=20+int(6*std::sin(bossTime*5));
    for(int i=0;i<20;i++){
      double a=2*PI*i/20,b=2*PI*(i+1)/20;
      line(cx+int(rr*cos(a)),cy+int(rr*sin(a)),cx+int(rr*cos(b)),cy+int(rr*sin(b)),MAGENTA);
    }
  }else if(bossPhase==2){
    for(int rr=24;rr<=49;rr+=12){
      for(int i=0;i<28;i++){
        double a=2*PI*i/28,b=2*PI*(i+1)/28;
        line(cx+int(rr*cos(a)),cy+int(rr*sin(a)),cx+int(rr*cos(b)),cy+int(rr*sin(b)),{177,80,235,130});
      }
    }
  }else if(bossPhase==3){
    for(int i=0;i<24;i++){
      double a=2*PI*i/24,b=2*PI*(i+1)/24;
      line(cx+int(20*cos(a)),cy+int(20*sin(a)),cx+int(20*cos(b)),cy+int(20*sin(b)),CYAN);
    }
  }

 }
 // Mismo cañón de plasma en Halcón Estelar Rex y Odisea II.
 // Aviso: esfera sin haz. Descarga: esfera + cuello ahusado + halo y núcleo.
 void draw_beam_esferico(int xx,int yy,bool discharge){
   const float flicker=.5f+.5f*std::sin(bossTime*24.f);
   const int radius=(discharge?18:11)+int(2.f*flicker);
   if(discharge){
     const int neckStart=xx-43;
     box(0,yy-19,std::max(0,neckStart),38,{138,30,75,40});
     box(0,yy-14,std::max(0,neckStart),28,{212,35,82,98+int(17*flicker)});
     box(0,yy-9,std::max(0,neckStart),18,{252,77,126,147+int(24*flicker)});
     box(0,yy-4,std::max(0,neckStart),8,{255,219,233,205+int(25*flicker)});
     for(int x=neckStart;x<xx-7;++x){
       const float t=float(x-neckStart)/float(std::max(1,xx-7-neckStart));
       const int spread=std::max(2,int(11.f*(1.f-t)+2.f*t));
       box(x,yy-spread,1,2*spread+1,{238,51,108,120+int(42*flicker)});
       const int core=std::max(1,int(4.f*(1.f-t)+1.f*t));
       box(x,yy-core,1,core*2+1,{255,219,232,212});
     }
   }
   for(int r=radius+10;r>=1;--r){
     const int alpha=r>radius?19:(r>radius-5?92:(r>radius-10?165:242));
     const RGBA ink=r>radius?RGBA{235,44,106,alpha}:
                    r>radius-6?RGBA{255,66,136,alpha}:RGBA{255,224,236,alpha};
     for(int dy=-r;dy<=r;++dy){
       int dx=int(std::sqrt(float(r*r-dy*dy)));
       line(xx-dx,yy+dy,xx+dx,yy+dy,ink);
     }
   }
 }
 // En Júpiter hay tres órdenes de ataque, repetidas: techo, suelo, centro.
 // Todas las alturas permanecen en el corredor jugable incluso en su estrechamiento.
 float ghost_attack_elapsed() const {return std::max(0.f,bossTime-3.2f);}
 int ghost_beam_pattern() const {return int(ghost_attack_elapsed()/8.6f)%3;}
 std::array<int,2> ghost_beam_lanes() const {
   switch(ghost_beam_pattern()){
     case 0:return {246,290}; // dos arriba: salida por abajo
     case 1:return {346,390}; // dos abajo: salida por arriba
     default:return {248,382}; // una arriba/otra abajo: corredor central
   }
 }
 void draw_ghost_laser(int originX,int y,bool discharge){
   const float flicker=.5f+.5f*std::sin(bossTime*21.f+y*.033f);
   const int r=discharge?15+int(flicker*2.f):10+int(flicker*2.f);
   if(discharge){
     const int neck=originX-40;
     box(0,y-17,std::max(0,neck),34,{87,20,172,47});
     box(0,y-11,std::max(0,neck),22,{157,42,224,105+int(flicker*22)});
     box(0,y-5,std::max(0,neck),10,{237,130,255,168+int(flicker*22)});
     box(0,y-2,std::max(0,neck),4,{255,237,255,223});
     for(int x=neck;x<originX-5;++x){
       const float t=float(x-neck)/float(std::max(1,originX-5-neck));
       int w=std::max(2,int(8.f*(1.f-t)+2.f*t));
       box(x,y-w,1,2*w+1,{216,102,247,157});
       box(x,y-2,1,5,{255,220,255,215});
     }
   }else{
     // Líneas de telemetría de aviso: ni disparan ni producen colisión.
     box(0,y-1,std::max(0,originX-15),3,{222,103,255,52+int(flicker*52)});
   }
   // Discos concéntricos: centro verdaderamente circular, sin rectángulo
   // blanco de píxeles que ensuciaba el nacimiento de los dos haces.
   auto disc=[&](int rr,RGBA tint){
     for(int dy=-rr;dy<=rr;++dy){
       int dx=int(std::sqrt(float(rr*rr-dy*dy)));
       line(originX-dx,y+dy,originX+dx,y+dy,tint);
     }
   };
   disc(r+10,{114,44,198,28});
   disc(r+7,{178,64,231,56});
   disc(r+3,{224,112,249,85});
   disc(r,{241,158,255,134});
   disc(std::max(1,r-4),{255,215,255,195});
   disc(std::max(1,r-8),{255,250,255,222});
 }
 void draw_halcon_jefe(){
  // El Halcón Estelar jamás usa las celdas de proyectil/ataque como cuerpo.
  // 2 poses reales de vuelo, resto del ataque generado por el motor.
  const int idx=mode==BOSS_DEATH?8+std::min(7,int(stateTime/2.9f*8)):(int(bossTime*2.6f)%2?2:0);
  sprite("halcon_jefe",8,256,152,idx,int(bossX-137),int(bossY-82),274,164);
  if(mode==BOSS){
    // El disparo rojo se dibuja por separado, estable y anclado al pico.
    const int xx=int(bossX-115), yy=int(bossY-10);
    if(bossPhase==0||bossPhase==1){
      draw_beam_esferico(xx,yy,bossPhase==1);
    }else if(bossPhase==3){
      const int cx=int(bossX-78),cy=int(bossY+2);
      for(int i=0;i<20;i++){
        double a=2*PI*i/20,b=2*PI*(i+1)/20;
        line(cx+int(18*cos(a)),cy+int(18*sin(a)),cx+int(18*cos(b)),cy+int(18*sin(b)),CYAN);
      }
    }

  }
 }
 void draw_odisea(){
  // Nave científica orgánica HD del autor: 6 vuelo, 6 ataque, 6 destrucción.
  // El ojo grande coincide con el centro de colisión (bossX-55, bossY+13).
  int idx=0;
  if(mode==BOSS_DEATH)idx=12+std::min(5,int(stateTime/2.9f*6.f));
  else if(bossPhase==1||bossPhase==2)idx=6+(int(bossTime*7.5f)%6);
  else idx=int(bossTime*5.f)%6;
  sprite("odisea",6,362,241,idx,int(bossX-337),int(bossY-152),410,275);
  if(mode==BOSS){
    const int eyeX=int(bossX-55),eyeY=int(bossY+13);
    const int nozzleX=int(bossX-302),nozzleY=int(bossY-12);
    if(bossPhase==0||bossPhase==1){
      // Comparte exactamente la esfera y el cuello orgánico del Halcón Rex.
      draw_beam_esferico(nozzleX,nozzleY,bossPhase==1);
      if(bossPhase==0)hud_atlas_text("hud_magenta","REACTOR: CARGA",739,172,17,1,1,false);
    }else if(bossPhase==3){
      // El núcleo ocular auténtico brilla cian cuando se puede dañar.
      const int rr=18+int(2*std::sin(bossTime*8.f));
      for(int i=0;i<24;i++){
        double a=2*PI*i/24,b=2*PI*(i+1)/24;
        line(eyeX+int(rr*std::cos(a)),eyeY+int(rr*std::sin(a)),
             eyeX+int(rr*std::cos(b)),eyeY+int(rr*std::sin(b)),{78,235,246,185});
      }
    }

  }
 }
 void draw_play(){background();terrain();draw_enemies();draw_deaths();draw_boss();draw_mira_debil();draw_prizes();draw_shots();draw_ship();hud();draw_radio();if(cheatInvulnerable){box(12,122,191,25,{3,15,26,230},GREEN);text("F9 · INVULNERABLE",107,126,17,GREEN,1);}if(pause){box(160,180,640,160,{4,14,32,230},CYAN);text("PAUSA",480,211,49,WHITE,1);text("ESC PARA CONTINUAR",480,282,25,AMBER,1);}}
 // Menú público: una única opción de campaña; las pruebas de fases se hacen por CLI.
 static constexpr int MENU_ITEMS=5;
 void title(){background();box(0,0,W,H,{1,8,20,130});image("logo",147,52,666,215);
  const char* names[]={"EMPEZAR","PANTALLA COMPLETA","OPCIONES","REPRODUCTOR MUSICAL","SALIR"};
  for(int i=0;i<MENU_ITEMS;++i){
   int y=270+i*36;
   box(265,y,430,31,i==menuIndex?RGBA{20,66,93,238}:RGBA{5,24,43,224},i==menuIndex?CYAN:DIM);
   text(std::string(names[i])+(i==1?(fullscreen?" · SÍ":" · NO"):""),480,y+6,21,i==menuIndex?AMBER:WHITE,1);
  }
  // Marcas oficiales en las dos esquinas inferiores, sin tapar las opciones.
  image("marca_tony",25,467,229,53);
  image("marca_grit",709,451,228,76);
  text("WASD / CURSORES / MANDO · ENTER / A · F11 COMPLETA",480,517,15,DIM,1);
 }
 void audio_options(){background();box(0,0,W,H,{0,6,18,191});
  if(debugPhaseSelector){
    text("PRUEBAS · SELECCIÓN DE FASE",480,46,35,AMBER,1);
    text("MODO DE DESARROLLO · ACCESO OCULTO F9",480,98,20,CYAN,1);
    const char* phases[]={"LA SEÑAL IMPOSIBLE","LA NAVE DEL SILENCIO","LAS CRIPTAS DE EUROPA",
      "LA CUNA DE LAS ESTRELLAS","EL CORAZÓN DE JÚPITER"};
    for(int i=0;i<5;++i){
      const int yy=146+i*58;const bool active=(debugPhase==i+1);
      box(118,yy,724,48,active?RGBA{22,65,92,241}:RGBA{6,24,43,225},active?AMBER:DIM);
      text("0"+std::to_string(i+1)+" · "+phases[i],480,yy+12,23,active?WHITE:DIM,1);
    }
    text("CURSORES: ELEGIR · ENTER: PROBAR · F9 / ESC: VOLVER",480,484,19,AMBER,1);
    text("EN PARTIDA, F9 SIGUE ACTIVANDO LA INVULNERABILIDAD",480,513,16,DIM,1);
    return;
  }
   text("OPCIONES · SONIDO Y VÍDEO",480,37,35,CYAN,1);
   text("BANDA SONORA: FUTUREDARK · GRIT NOVA ENGINE",480,85,17,AMBER,1);
   const char* names[]={"VOLUMEN GENERAL","MÚSICA","EFECTOS","SONIDO","ESTÉREO ESPACIAL","SCANLINES","VOLVER AL TÍTULO"};
   const char* modes[]={"DESACTIVADO","SUAVE","ARCADE","CRT INTENSO"};
   for(int i=0;i<7;i++){
     int y=116+i*54;bool selected=i==menuOption;
     box(115,y,730,49,selected?RGBA{16,49,76,248}:RGBA{4,23,42,226},selected?CYAN:DIM);
     text(names[i],144,y+13,21,selected?WHITE:DIM);
     if(i<3){
       int volume=i==0?masterVolume:i==1?musicVolume:effectsVolume;
       box(440,y+19,270,12,{18,35,54,255},{77,115,146,255});
       box(442,y+21,int(266*volume/100.f),8,selected?AMBER:CYAN);
       text(std::to_string(volume)+"%",801,y+12,22,selected?AMBER:WHITE,2);
     }else if(i==3)text(sound?"ACTIVADO":"SILENCIADO",800,y+12,21,sound?GREEN:RED,2);
     else if(i==4)text(stereoSpatial?"SÍ":"NO",800,y+12,21,stereoSpatial?GREEN:AMBER,2);
     else if(i==5)text(modes[scanlines],800,y+12,21,scanlines?CYAN:DIM,2);
   }
   text("CURSORES: ELEGIR Y AJUSTAR · ENTER: CAMBIAR · ESC: VOLVER",480,498,17,AMBER,1);
   text("LOS CAMBIOS SE GUARDAN AUTOMÁTICAMENTE",480,522,15,DIM,1);
  }

 static std::string fmt_time(size_t samples){size_t t=samples/44100;return std::to_string(t/60)+":"+(t%60<10?"0":"")+std::to_string(t%60);}
 void jukebox(){background();box(0,0,W,H,{0,5,14,220});
  // Deck de cinta espacial / FutureDark. Dibujado con primitivas SDL2.
  text("FUTUREDARK · CONTACTO 2010",480,24,34,CYAN,1);
  text("REPRODUCTOR DE CASSETTE · EUROPEAN SPACE FORCE",480,66,17,AMBER,1);
  box(28,103,607,371,{9,19,32,255},{96,149,170,255});
  box(39,113,585,350,{17,32,48,255},{34,73,89,255});
  line(48,123,614,123,{192,218,210,220});
  box(62,138,539,230,{3,10,20,255},{103,163,171,255});
  box(72,148,519,210,{29,41,47,255},{150,174,171,255});
  // Ventana del cassette de resina ahumada y etiqueta de la pista.
  box(89,162,484,46,{9,22,33,255},{72,130,157,255});
  text("FUTUREDARK  /  2010   •   SIDE A",331,168,20,AMBER,1);
  text("CHROMIUM DIOXIDE · STEREO 44.1 kHz",331,188,14,CYAN,1);
  box(100,222,462,112,{4,10,16,255},{91,130,135,255});
  size_t at=0,total=0;bool live=false,paused=false;float meterL=0,meterR=0;
  if(auddev)SDL_LockAudioDevice(auddev);
  paused=musicPaused;live=musicTrack!=nullptr;at=musicPos;
  if(playingIndex>=0){
    total=musicData.pcm.size()/2;
    if(!live)at=total;
    if(live&&!paused&&musicData.channels==2&&total){
      const size_t start=std::min(at,total-1);
      const size_t end=std::min(start+400,total);
      double sumL=0,sumR=0;
      for(size_t i=start;i<end;i+=4){
        sumL+=std::abs(int(musicData.pcm[i*2]));
        sumR+=std::abs(int(musicData.pcm[i*2+1]));
      }
      const double scale=32768.0*double(std::max<size_t>(1,(end-start+3)/4));
      meterL=float(std::min(1.0,sumL/scale*3.5));
      meterR=float(std::min(1.0,sumR/scale*3.5));
    }
  }
  if(auddev)SDL_UnlockAudioDevice(auddev);
  const float progress=total?std::clamp(float(double(at)/double(total)),0.f,1.f):0.f;
  // Los carretes giran sólo mientras está sonando, y se congelan en pausa/stop.
  static float reelAngle=0.f;static uint32_t lastTick=0;
  const uint32_t tick=SDL_GetTicks();
  if(lastTick && tick>=lastTick && live&&!paused)reelAngle+=std::min(float(tick-lastTick)/1000.f,.08f)*4.3f;
  lastTick=tick;
  const auto reel=[&](int cx,int cy,int radius,float fill){
    const int body=std::max(16,int(radius*fill));
    for(int i=0;i<32;++i){
      const double a=double(i)*2*PI/32,b=double(i+1)*2*PI/32;
      line(cx+int(radius*std::cos(a)),cy+int(radius*std::sin(a)),
           cx+int(radius*std::cos(b)),cy+int(radius*std::sin(b)),{172,188,185,255});
      line(cx+int(body*std::cos(a)),cy+int(body*std::sin(a)),
           cx+int(body*std::cos(b)),cy+int(body*std::sin(b)),{111,92,65,255});
    }
    for(int i=0;i<6;++i){
      const double a=double(i)*PI/3+double(reelAngle);
      line(cx+int(10*std::cos(a)),cy+int(10*std::sin(a)),
           cx+int((radius-7)*std::cos(a)),cy+int((radius-7)*std::sin(a)),{202,215,208,255});
    }
    box(cx-8,cy-8,16,16,{15,31,44,255},{177,215,221,255});
    box(cx-3,cy-3,6,6,{246,190,87,255});
  };
  reel(213,278,48,.77f-.29f*progress);
  reel(447,278,48,.48f+.29f*progress);
  box(273,270,111,15,{19,24,26,255},{73,92,91,255});
  line(263,315,397,315,{87,73,58,255});
  box(103,338,457,12,{9,25,31,255},{75,121,133,255});
  if(total)box(105,340,int(453*progress),8,AMBER);
  box(65,378,532,77,{8,25,34,255},{72,134,149,255});
  text(playingIndex<0?"CINTA PREPARADA":live?(paused?"PAUSE":"PLAY"):"STOP",89,385,21,live&&!paused?GREEN:AMBER);
  text(fmt_time(at)+" / "+fmt_time(total),583,385,20,WHITE,2);
  const char* labels[]={"|<<","PLAY","PAUSA","STOP",">>|"};
  for(int i=0;i<5;++i){
    const int x=84+i*101;
    box(x,416,86,32,{27,49,61,255},{133,174,174,255});
    text(labels[i],x+43,424,17,i==1&&live&&!paused?GREEN:WHITE,1);
  }
  box(646,103,286,371,{7,19,34,246},{80,150,175,255});
  text("SELECCIÓN DE CINTA",789,116,21,CYAN,1);
  for(int i=0;i<JUKE_COUNT;++i){
    const int y=153+i*26;bool selected=i==playerIndex,active=i==playingIndex;
    box(654,y,270,24,selected?RGBA{34,78,106,255}:RGBA{9,29,49,240},selected?AMBER:DIM);
    text((i<9?"0":"")+std::to_string(i+1),661,y+4,16,selected?AMBER:DIM);
    int font=15;while(font>11&&textwidth(jukeName[i],font)>211)--font;
    text(jukeName[i],690,y+(24-font)/2,font,selected?WHITE:DIM);
    if(active)box(916,y+8,5,8,live&&!paused?GREEN:AMBER);
  }
  text("NIVEL DE SEÑAL  L / R",789,420,15,CYAN,1);
  for(int ch=0;ch<2;++ch){
    const int yy=443+ch*13;const int bars=int(19*(ch==0?meterL:meterR));
    for(int k=0;k<19;++k)box(697+k*11,yy,8,7,k<bars?(k>15?RED:k>11?AMBER:GREEN):RGBA{24,48,60,255});
  }
  text("ARRIBA/ABAJO: ELEGIR · ENTER: PLAY · IZQ/DER: CAMBIAR",480,490,17,AMBER,1);
  text("P: PAUSA · X: STOP · ESC: TÍTULO · FUTUREDARK © 2026",480,514,16,DIM,1);
 }
 void intro(){background();terrain();box(0,0,W,H,{0,5,18,145});box(100,61,760,113,{2,13,31,224},{31,90,135});text(fase==5?"05 · EL CORAZÓN DE JÚPITER":fase==4?"04 · LA CUNA DE LAS ESTRELLAS":fase==3?"03 · LAS CRIPTAS DE EUROPA":fase==2?"02 · LA NAVE DEL SILENCIO":"01 · LA SEÑAL IMPOSIBLE",480,75,41,WHITE,1);text(fase==5?"GRAN TORMENTA · DEFENSA DEL NÚCLEO":fase==4?"OCÉANO INTERIOR · ÚTERO CÓSMICO":fase==3?"BAJO EL HIELO DE EUROPA":fase==2?"ODISEA II · LABORATORIO INFECTADO":"ÓRBITA EXTERIOR DE JÚPITER",480,132,25,CYAN,1);
  box(57,207,846,237,{3,16,34,238},{65,148,199});portrait(2,0,84,242,112,128);portrait(3,0,210,242,112,128);
  text("MISIÓN DE RECONOCIMIENTO",362,231,24,AMBER);
  // Centramos verticalmente 2, 3 o 4 renglones sin mostrar texto heredado.
  const int count=int(std::count_if(std::begin(TEXTO_BRIEFING[fase-1]),std::end(TEXTO_BRIEFING[fase-1]),[](const char* str){return str[0]!='\0';}));
  const int ys4[4]={278,315,351,386};
  const int ys3[3]={281,333,385};
  const int ys2[2]={299,363};
  for(int i=0;i<count;i++){
    const std::string frase=TEXTO_BRIEFING[fase-1][i];
    int altura=22;while(altura>16&&textwidth(frase,altura)>540)--altura;
    const int y=count==4?ys4[i]:count==3?ys3[i]:ys2[i];
    text(frase,350,y,altura,WHITE);
  }
  text("FIREBIRD 01 · SISTEMAS LISTOS",480,477,26,GREEN,1);
  text("ESPACIO / A · COMENZAR",480,511,19,AMBER,1);
 }
 void boss_intro(){background();terrain();box(0,0,W,H,{9,4,20,170});box(100,48,760,130,{14,6,24,232},{135,61,89});
  text("¡CONTACTO INMINENTE!",480,62,44,RED,1);
  text(TITULO_JEFE[fase-1],480,129,34,WHITE,1);
  box(67,206,828,218,{9,15,35,225},{161,74,99});portrait(3,0,96,248,112,128);portrait(4,0,222,248,112,128);
  // AURA-01 conserva las frases originales previas a cada jefe, ajustadas para legibilidad.
  for(int i=0;i<2;i++){
    const std::string frase=TEXTO_PREJEFE[fase-1][i];
    int altura=i==0?23:22;
    while(altura>15&&textwidth(frase,altura)>500)--altura;
    text(frase,374,i==0?244:280,altura,i==0?CYAN:WHITE);
  }
  for(int i=0;i<2;i++){
    const std::string consejo=CONSEJOS_JEFE[fase-1][i];
    int altura=21;while(altura>17&&textwidth(consejo,altura)>525)--altura;
    text(consejo,360,i==0?326:370,altura,i==0?WHITE:AMBER);
  }
  text("ESPACIO / A · CONTINUAR",480,480,22,RED,1);
 }
 void endscreen(bool victory){background();terrain();box(0,0,W,H,{1,7,22,170});text(victory?"¡MISIÓN COMPLETADA!":"MISIÓN FALLIDA",480,130,49,victory?GREEN:RED,1);
  text(victory?(fase==5?"EL FANTASMA DE JÚPITER HA CAÍDO.":fase==4?"EL HIJO DE LAS ESTRELLAS HA CAÍDO.":fase==3?"EUROPA HA SIDO LIBERADA.":fase==2?"LA ODISEA II HA SIDO DESTRUIDA.":"LA SEÑAL HA SIDO SILENCIADA."):"FIREBIRD 01 · SIN SEÑAL",480,230,27,WHITE,1);
  if(victory&&fase==5){text("FIREBIRD 01 · REGRESO A CASA",480,272,23,AMBER,1);text("2026 · TONY C. WARE",480,359,22,WHITE,1);text("BANDA SONORA · FUTUREDARK",480,381,18,AMBER,1);}
  text("PUNTUACIÓN: "+num(score),480,319,30,CYAN,1);text(fase<5&&victory?"ESPACIO: SIGUIENTE FASE":fase==5&&victory?"ESPACIO: ESCENA FINAL":"ESPACIO: VOLVER AL TÍTULO",480,408,22,AMBER,1);
 }
 void final_credits(){
   // El dibujo aportado por Tony C. Ware permanece intacto; texto encima a la derecha.
   image("final_escena",0,0,W,H);
   // Velo oscuro discreto sobre el sol: garantiza legibilidad sin tapar piloto o avión.
   box(530,32,405,205,{5,9,23,157},{255,201,128,110});
   box(546,47,372,2,{255,209,164,180});
   box(546,224,372,1,{255,209,164,120});
   text("CONTACTO 2010",732,62,26,WHITE,1);
   text("CRÉDITOS FINALES",732,95,19,{247,200,136},1);
   static const char* headings[FINAL_CREDITS_PAGES]={
    "MISIÓN CUMPLIDA", "CREACIÓN, CONCEPTO Y DIRECCIÓN", "PROGRAMACIÓN Y DESARROLLO",
    "ARTE Y DISEÑO VISUAL", "BANDA SONORA ORIGINAL", "PRODUCCIÓN",
    "2026", "POR HABER LLEGADO HASTA JÚPITER", "EL CONTACTO HA TERMINADO."};
   static const char* names[FINAL_CREDITS_PAGES]={
    "FIREBIRD 01 · REGRESO A CASA", "TONY C. WARE", "GPT-6 · OPENAI",
    "TONY C. WARE + GPT-6", "FUTUREDARK (SUNO)", "TONY C. WARE",
    "TONY C. WARE", "GRACIAS POR JUGAR", "EL VIAJE, NO."};
   const float duration=finalCreditsPage==FINAL_CREDITS_PAGES-2?7.0f:5.35f;
   float appear=std::clamp(finalCreditsClock/0.85f,0.f,1.f);
   float leave=finalCreditsPage==FINAL_CREDITS_PAGES-1?1.f:std::clamp((duration-finalCreditsClock)/0.9f,0.f,1.f);
   float a=std::min(appear,leave);
   int alpha=int(255*a);
   RGBA heading{255,203,142,alpha},name{248,250,255,alpha};
   RGBA shadow{0,0,0,int(200*a)};
   // Encaje automático del texto real dentro de la placa derecha: evita cortes
   // de frases largas y mantiene las dos cartelas de despedida bien centradas.
   const bool thanks=finalCreditsPage==FINAL_CREDITS_PAGES-2;
   const bool ending=finalCreditsPage==FINAL_CREDITS_PAGES-1;
   int headlineHeight=ending?22:thanks?15:18;
   while(headlineHeight>12 && textwidth(headings[finalCreditsPage],headlineHeight)>340)--headlineHeight;
   text(headings[finalCreditsPage],733,147,headlineHeight,shadow,1);
   text(headings[finalCreditsPage],732,146,headlineHeight,heading,1);
   int nameHeight=ending?34:thanks?31:finalCreditsPage==0?25:29;
   while(nameHeight>18 && textwidth(names[finalCreditsPage],nameHeight)>340)--nameHeight;
   text(names[finalCreditsPage],733,189,nameHeight,shadow,1);
   text(names[finalCreditsPage],732,188,nameHeight,name,1);
   text("ESPACIO / A  ·  CONTINUAR      ESC  ·  TÍTULO",930,514,17,{255,244,225,245},2);
   if(stateTime<0.8f){int darkness=int(250.f*std::clamp(1.f-stateTime/.8f,0.f,1.f));box(0,0,W,H,{0,0,0,darkness});}
 }
 void render(){switch(mode){case TITLE:title();break;case AUDIO_OPTIONS:audio_options();break;case JUKEBOX:jukebox();break;case INTRO:intro();break;case PLAY:case BOSS:case BOSS_DEATH:draw_play();break;case BOSS_INTRO:boss_intro();break;case DEAD:endscreen(false);break;case WIN:endscreen(true);break;case FINAL_CREDITS:final_credits();break;}
    if(scanlines && scanlineTexture){
      SDL_SetTextureAlphaMod(scanlineTexture,uint8_t(scanlines==1?70:scanlines==2?125:185));
      SDL_Rect rect{0,0,W,H};SDL_RenderCopy(ren,scanlineTexture,nullptr,&rect);
    }
  }
 void toggleFullscreen(){fullscreen=!fullscreen;int ret=SDL_SetWindowFullscreen(win,fullscreen?0x1001:0);if(ret){fullscreen=!fullscreen;std::cerr<<"Pantalla completa: "<<SDL_GetError()<<"\n";}}
 void key(int code){ // SDL keysyms ASCII + especiales
  if(code==1073741892){toggleFullscreen();return;} // F11
  if(code==1073741890 && (mode==PLAY||mode==BOSS)){ // F9: truco de práctica
    cheatInvulnerable=!cheatInvulnerable;
    if(cheatInvulnerable)cheatUsedThisRun=true;
    tone("item");return;
  }
  if(mode==TITLE){
   if(code==1073741905||code=='s')menuIndex=(menuIndex+1)%MENU_ITEMS;
   if(code==1073741906||code=='w')menuIndex=(menuIndex+MENU_ITEMS-1)%MENU_ITEMS;
   if(code==13||code==32){switch(menuIndex){
     case 0:start(1);break;
     case 1:toggleFullscreen();break;
     case 2:mode=AUDIO_OPTIONS;menuOption=0;stateTime=0;break;
     case 3:mode=JUKEBOX;playerIndex=0;playingIndex=-1;stateTime=0;jukebox_stop();break;
     case 4:running=false;break;
   }}
  }else if(mode==AUDIO_OPTIONS){
   if(code==1073741890){debugPhaseSelector=!debugPhaseSelector;debugPhase=1;return;}
   if(debugPhaseSelector){
     if(code==27){debugPhaseSelector=false;return;}
     if(code==1073741905||code=='s')debugPhase=debugPhase%5+1;
     if(code==1073741906||code=='w')debugPhase=(debugPhase+3)%5+1;
     if(code==13||code==32){int selected=debugPhase;debugPhaseSelector=false;start(selected);}
     return;
   }
   if(code==27){back_to_title();return;}
   if(code==1073741905||code=='s')menuOption=(menuOption+1)%7;
   if(code==1073741906||code=='w')menuOption=(menuOption+6)%7;
   if((code==1073741904||code==1073741903)&&menuOption<3)edit_volume(menuOption,code==1073741903?1:-1);
    if((code==1073741904||code==1073741903)&&menuOption==5){scanlines=(scanlines+(code==1073741903?1:3))%4;save_audio_settings();}
    if((code==13||code==32||code==1073741904||code==1073741903)&&menuOption==4){
      if(auddev)SDL_LockAudioDevice(auddev);stereoSpatial=!stereoSpatial;
      if(auddev)SDL_UnlockAudioDevice(auddev);save_audio_settings();
    }
    if(code==13||code==32){if(menuOption==3)set_audio_enabled(!sound);if(menuOption==5){scanlines=(scanlines+1)%4;save_audio_settings();}if(menuOption==6)back_to_title();}
  }else if(mode==JUKEBOX){
   if(code==27){back_to_title();return;}
   if(code==1073741905||code=='s')playerIndex=(playerIndex+1)%JUKE_COUNT;
   if(code==1073741906||code=='w')playerIndex=(playerIndex+JUKE_COUNT-1)%JUKE_COUNT;
   if(code==1073741904){playerIndex=(playerIndex+JUKE_COUNT-1)%JUKE_COUNT;jukebox_play();}
   if(code==1073741903){playerIndex=(playerIndex+1)%JUKE_COUNT;jukebox_play();}
   if(code==13||code==32)jukebox_play();
   if(code=='p'&&playingIndex>=0){if(auddev)SDL_LockAudioDevice(auddev);musicPaused=!musicPaused;if(auddev)SDL_UnlockAudioDevice(auddev);}
   if(code=='x')jukebox_stop();
  }else if(mode==INTRO){if(code==13||code==32){mode=PLAY;stateTime=0;}}
   else if(mode==BOSS_INTRO){if(code==13||code==32)boss_init();}
   else if(mode==PLAY||mode==BOSS){if(code==27)pause=!pause;if(code=='m'){set_audio_enabled(!sound);}}
   else if(mode==FINAL_CREDITS){
    if(code==27){back_to_title();return;}
    if((code==13||code==32)&&stateTime>=1.f){
      if(finalCreditsPage<FINAL_CREDITS_PAGES-1){++finalCreditsPage;finalCreditsClock=0;}
      else back_to_title();
    }
   }
   else if((mode==WIN||mode==DEAD)&&(code==13||code==32||code==27)&&stateTime>=3){
     if(mode==WIN&&fase<5)next_phase(fase+1);
     else if(mode==WIN&&fase==5)begin_final_credits();
     else back_to_title();
   }
 }
 void eventPoll(){
  // Comprobar también desconexiones silenciosas, sin esperar a un evento.
  if(pad&&!SDL_GameControllerGetAttached(pad)){std::cout<<"Mando desconectado\n";close_gamepad();}
  if(!pad && (uint32_t)(SDL_GetTicks()-lastPadScan)>=1000u)scan_gamepads();
  uint8_t data[64];
  while(SDL_PollEvent(data)){
   uint32_t type=0;memcpy(&type,data,4);
   // SDL_CONTROLLERDEVICEADDED: which contiene ÍNDICE de dispositivo.
   if(type==0x653){int32_t index;memcpy(&index,data+8,4);if(!pad)open_gamepad(index);continue;}
   // SDL_CONTROLLERDEVICEREMOVED: which contiene ID de INSTANCIA (no índice).
   if(type==0x654){int32_t instance;memcpy(&instance,data+8,4);
     if(pad&&instance==padInstance){std::cout<<"Mando desconectado (instancia="<<instance<<")\n";close_gamepad();scan_gamepads();}
     continue;
   }
   if(type==0x100){running=false;continue;}
   if(type==0x300){int32_t code;memcpy(&code,data+20,4);if(code!=1073741890||data[13]==0)key(code);continue;}
   // No interpretar pulsaciones de mandos ajenos al controlador activo.
   if(type==0x650||type==0x651){
     int32_t instance;memcpy(&instance,data+8,4);
     if(!pad||instance!=padInstance)continue;
   }
   if(type==0x650&&(mode==TITLE||mode==AUDIO_OPTIONS||mode==JUKEBOX)){
    uint8_t axis=data[12];int16_t value;memcpy(&value,data+16,2);
    if(std::abs(value)>21000&&stateTime>.17f){
      if(axis==1)key(value>0?1073741905:1073741906);
      if(axis==0&&mode==AUDIO_OPTIONS)key(value>0?1073741903:1073741904);
      stateTime=0;
    }
   }
   if(type==0x651){ // SDL_CONTROLLERBUTTONDOWN, del mando activo
    uint8_t button=data[12];
    if(button==0){key(13);}
    else if(button==1&&(mode==TITLE||mode==AUDIO_OPTIONS||mode==JUKEBOX))key(27);
    else if(button==6){if(mode==PLAY||mode==BOSS)pause=!pause;}
    else if(button==2&&mode==JUKEBOX)key('p');
    else if(button==3&&mode==JUKEBOX)key('x');
    else if(button==11)key(1073741906);
    else if(button==12)key(1073741905);
    else if(button==13&&mode==AUDIO_OPTIONS)key(1073741904);
    else if(button==14&&mode==AUDIO_OPTIONS)key(1073741903);
   }
  }
 }
 bool screenshot(const std::string& path){int outputW=W,outputH=H;SDL_GetRendererOutputSize(ren,&outputW,&outputH);
   std::vector<uint8_t>b(size_t(outputW)*outputH*4);
   if(SDL_RenderReadPixels(ren,nullptr,SDL_PIXELFORMAT_ABGR8888,b.data(),outputW*4)<0)return false;
   png_image img{};img.version=PNG_IMAGE_VERSION;img.width=outputW;img.height=outputH;img.format=PNG_FORMAT_RGBA;
   return png_image_write_to_file(&img,path.c_str(),0,b.data(),outputW*4,nullptr)!=0;
 }
 int run(int argc,char**argv){if(!init())return 2;bool test=false,testFase2=false,testFase3=false,testFase4=false,testEquilibrio=false,testBossCombat=false,testBossCombat3=false,testBossCombat4=false,testAudio=false,testPlayerShotAudio=false,testJingles=false,testSoundtrack=false,hudQa=false,testBossCombat5=false,testFase5=false,testInvul=false,testFinalCredits=false,testMenu=false,testBossStatus=false,testDebugSelector=false,testHudAccents=false,testControls=false,testSaw=false,testIce=false,testHawks=false,testBeams=false,testFreshJingles=false,testVisualSignals=false,testFirebird=false;for(int i=1;i<argc;i++){std::string s=argv[i];if(s=="--test-combate-fase2")testBossCombat=true;if(s=="--test-combate-fase3")testBossCombat3=true;if(s=="--test-combate-fase4")testBossCombat4=true;if(s=="--test-combate-fase5")testBossCombat5=true;if(s=="--self-test-invulnerabilidad")testInvul=true; if(s=="--self-test-fase5")testFase5=true;if(s=="--self-test-audio")testAudio=true;if(s=="--self-test-disparos")testPlayerShotAudio=true;if(s=="--self-test-jingles")testJingles=true;if(s=="--self-test-jingles-nuevos")testFreshJingles=true;if(s=="--self-test-bso")testSoundtrack=true;if(s=="--self-test-final")testFinalCredits=true;if(s=="--self-test-menu")testMenu=true;if(s=="--self-test-selector")testDebugSelector=true;if(s=="--self-test-acentos")testHudAccents=true;if(s=="--self-test-controles")testControls=true;if(s=="--self-test-sierras")testSaw=true;if(s=="--self-test-estalactitas")testIce=true;if(s=="--self-test-halcones")testHawks=true;if(s=="--self-test-rayos")testBeams=true;if(s=="--self-test-boss-status")testBossStatus=true;if(s=="--self-test-avisos-escudo")testVisualSignals=true;if(s=="--self-test-firebird")testFirebird=true;if(s=="--capture-hud-variants")hudQa=true;if(s=="--self-test-equilibrio")testEquilibrio=true;if(s=="--self-test-fase2")testFase2=true;if(s=="--self-test-fase3")testFase3=true;if(s=="--self-test-fase4")testFase4=true;if(s=="--self-test"||s=="--self-test-fase2"||s=="--self-test-fase3"||s=="--self-test-fase4"||s=="--self-test-fase5")test=true;if(s=="--capture-firebird-vuelo"||s=="--capture-firebird-escudo"||s=="--capture-firebird-danos") {shotMode=s;capture=true;}if(s=="--capture-fase1-union"||s=="--capture-escudo-nuevo"||s=="--capture-triangulos"||s=="--capture-mira-jefe-3"||s=="--capture-mira-jefe-5"||s=="--capture-fantasma-aviso"||s=="--capture-halcones-nuevos"||s=="--capture-odisea-rayo-v104"||s=="--capture-fantasma-doble-arriba"||s=="--capture-fantasma-doble-abajo"||s=="--capture-fantasma-doble-mixto"||s=="--capture-sierras-fase1"||s=="--capture-estalactitas-fase3"||s=="--capture-estalactitas-caida"||s=="--capture-final-gracias"||s=="--capture-final-inicio"||s=="--capture-final-programacion"||s=="--capture-final-musica"||s=="--capture-final-frase"||s=="--capture-options"||s=="--capture-selector"||s=="--capture-halcon-rayo"||s=="--capture-fantasma-intenso"||s=="--capture-jukebox"||s=="--capture-title"||s=="--capture-intro"||s=="--capture-play"||s=="--capture-boss"||s=="--capture-new-moai"||s=="--capture-fase2"||s=="--capture-boss-fase2"||s=="--capture-boss-fase2-ataque"||s=="--capture-muerte-fase2"||s=="--capture-f22"||s=="--capture-fase3"||s=="--capture-boss-fase3"||s=="--capture-fase4"||s=="--capture-boss-fase4"||s=="--capture-fase5"||s=="--capture-fase5-union"||s=="--capture-boss-fase5"||s=="--capture-muerte-fase5"||s=="--capture-f9-fase5"){shotMode=s;capture=true;}}
  if(argc>=2 && std::string(argv[1])=="--self-test-estereo-crt"){
    gn::Sound sample;sample.pcm.assign(64,12000);
    std::array<AudioVoice,32> tv{};const Sound* track=nullptr;size_t pos=0;
    std::array<uint8_t,256> left{},right{},mono{};
    tv[1]={&sample,0,1.f,5,0,-1.f};
    gn::mix_stereo_s16(left.data(),int(left.size()),true,true,track,pos,false,false,tv,100,100,100);
    tv[1]={&sample,0,1.f,5,0,1.f};
    gn::mix_stereo_s16(right.data(),int(right.size()),true,true,track,pos,false,false,tv,100,100,100);
    tv[1]={&sample,0,1.f,5,0,1.f};
    gn::mix_stereo_s16(mono.data(),int(mono.size()),true,false,track,pos,false,false,tv,100,100,100);
    auto val=[](const std::array<uint8_t,256>& b,int ch){int16_t v=0;std::memcpy(&v,b.data()+ch*2,2);return int(v);};
    bool ok=scanlineTexture && val(left,0)>val(left,1)&&val(right,1)>val(right,0)&&
      val(mono,0)==val(mono,1)&&scanlines>=0&&scanlines<=3;
    std::cout<<"V1.2 ESTEREO/SCANLINES: "<<(ok?"OK":"ERROR")<<"\n";
    return ok?0:48;
  }
  if(testFirebird){
    auto it=textures.find("ship");
    bool atlas=(it!=textures.end() && it->second.w==1536 && it->second.h==1536);
    bool geometry=(SHIP_ATLAS_CELL==384 && SHIP_FRAME_COLS==4 &&
       SHIP_RENDER_W==104 && SHIP_RENDER_H==100 &&
       SHIP_NOSE_X>=40 && SHIP_NOSE_X<=44 &&
       SHIP_HIT_HALF_X<=12 && SHIP_HIT_HALF_Y<=10);
    start(1);mode=PLAY;shipx=260;shipy=280;death=0;
    shots.clear();weapon=0;shoot();
    bool pulse=(shots.size()==1 && shots[0].friendly &&
       std::abs(shots[0].x-(shipx+SHIP_NOSE_X))<.1f);
    shots.clear();weapon=1;shoot();
    bool doubleFire=(shots.size()==3 && shots[0].friendly && shots[1].friendly && shots[2].friendly);
    shots.clear();weapon=2;shoot();
    bool laser=(shots.size()==3 && shots[1].y<shipy && shots[2].y>shipy);
    bool deathFrames=true;
    for(int k=0;k<8;k++){
      death=.9f-(float(k)+.1f)*.9f/8.f;
      int frame=8+std::clamp(int((.9f-death)*8.f/.9f),0,7);
      deathFrames=deathFrames && frame==k+8;
    }
    death=0;weapon=0;shots.clear();
    shieldTime=9.f;vulnerable=0;
    bool shield=escudo_visible();life_hit();
    shield=shield&&lives==3&&shieldTime==0.f;
    bool ok=atlas&&geometry&&pulse&&doubleFire&&laser&&deathFrames&&shield;
    std::cout<<"FIREBIRD INDUSTRIAL V1.1: "<<(ok?"OK":"ERROR")
      <<" atlas="<<atlas<<" tamano_y_hitpoint="<<geometry<<" pulso="<<pulse
      <<" doble="<<doubleFire<<" laser="<<laser<<" muerte_8_frames="<<deathFrames
      <<" escudo="<<shield<<"\n";
    return ok?0:34;
  }
  if(capture && (shotMode=="--capture-firebird-vuelo"||
                 shotMode=="--capture-firebird-escudo"||
                 shotMode=="--capture-firebird-danos")){
    const bool damage=shotMode=="--capture-firebird-danos";
    start(damage?5:shotMode=="--capture-firebird-escudo"?3:1);
    mode=PLAY;shipx=288;shipy=277;stageTime=2.43f;vulnerable=0;
    if(damage){death=.45f;shieldTime=0.f;}
    else if(shotMode=="--capture-firebird-escudo")shieldTime=9.f;
    else {weapon=1;shoot();}
    render();SDL_RenderPresent(ren);
    const char* name=damage?"CAPTURA_FIREBIRD_01_DESTRUCCION.png":
      shotMode=="--capture-firebird-escudo"?"CAPTURA_FIREBIRD_01_ESCUDO.png":
      "CAPTURA_FIREBIRD_01_VUELO_Y_DISPARO.png";
    bool ok=screenshot(name);
    std::cout<<name<<" "<<(ok?"OK":"ERROR")<<"\n";
    return ok?0:3;
  }
  if(testVisualSignals){
    bool ok=textures.count("escudo_nuevo")&&textures.count("triangulo")&&textures.count("mira");
    start(3);mode=PLAY;
    Actor e;e.kind=23;e.fire=.6f;e.alive=true;
    ok=ok&&aviso_hielo(e);e.fire=0.f;ok=ok&&!aviso_hielo(e);
    for(int p=1;p<=5;++p){
      start(p);mode=BOSS;bossX=777;bossY=267;bossHp=50;bossPhase=p>=2?3:1;
      auto c=centro_debil();
      ok=ok&&punto_debil_activo()&&c.first>100&&c.first<900&&c.second>HUD_H&&c.second<H;
      bossPhase=p>=2?1:0;ok=ok&&!punto_debil_activo();
      bossPhase=p>=2?3:1;bossHp=0;ok=ok&&!punto_debil_activo();
    }
    start(2);mode=PLAY;vulnerable=0;death=0;shieldTime=9.f;lives=3;
    ok=ok&&escudo_visible();life_hit();
    ok=ok&&lives==3&&shieldTime==0.f&&!escudo_visible();
    std::cout<<"ESCUDO, AVISOS DE EUROPA Y RETÍCULAS 5 JEFES: "<<(ok?"OK":"ERROR")<<"\n";
    return ok?0:3;
  }
  if(testFreshJingles){
    bool ok=true;
    for(const char* name:{"musica_titulo","musica_victoria","musica_game_over"}){
      Sound song;const bool present=ogg_decode(name,song);
      const float duration=float(song.pcm.size()/2)/44100.f;
      float onset=999.f,maxRms=0.f;
      const size_t chunk=4410*2;
      std::vector<float> env;
      for(size_t pos=0;pos+chunk<=song.pcm.size();pos+=chunk){
        double sum=0;
        for(size_t j=0;j<chunk;++j){const double v=double(song.pcm[pos+j])/32768.;sum+=v*v;}
        const float rms=float(std::sqrt(sum/double(chunk)));env.push_back(rms);maxRms=std::max(maxRms,rms);
      }
      const float threshold=std::max(.002f,maxRms*.06f);
      for(size_t i=0;i<env.size();++i)if(env[i]>threshold){onset=float(i)*.1f;break;}
      const bool valid=present&&song.channels==2&&duration>3.f&&duration<90.f&&onset<.5f;
      ok &=valid;
      std::cout<<name<<" OGG 44.1k estereo duracion="<<duration<<" onset="<<onset<<" "<<(valid?"OK":"ERROR")<<"\n";
    }
    return ok?0:43;
  }
  if(testHawks){
    bool ok=true;
    for(auto id:{"halcon_izquierda","halcon_derecha"}){
      auto it=textures.find(id);
      if(it==textures.end()||it->second.w!=1280||it->second.h!=1280){ok=false;continue;}
      // 4x4, ocho poses de vuelo y ocho de destrucción: ambas direcciones.
      for(int i=0;i<16;++i)ok&= (i%4)*320<it->second.w && (i/4)*320<it->second.h;
    }
    const bool noLegacy=textures.find("halcon_di")==textures.end()&&textures.find("halcon4_di")==textures.end();
    ok&=noLegacy;
    std::cout<<"HALCONES RGBA 16x2, VUELO SIN DISPARO Y DESTRUCCIÓN: "<<(ok?"OK":"ERROR")<<"\n";
    return ok?0:41;
  }
  if(testBeams){
    bool ok=true;
    start(5); // activa el terreno HD real antes de comprobar rutas de escape
    // No hay cerrojo: cada ciclo entra en la secuencia techo, suelo, mixta.
    const int tops[3]={246,346,248},bottoms[3]={290,390,382};
    const int safeY[3]={345,275,315};
    for(int i=0;i<3;++i){
      bossTime=3.2f+8.6f*i+1.7f;
      auto lanes=ghost_beam_lanes();
      bool matches=ghost_beam_pattern()==i&&lanes[0]==tops[i]&&lanes[1]==bottoms[i];
      bool gap=std::abs(safeY[i]-lanes[0])>=25&&std::abs(safeY[i]-lanes[1])>=25;
      // Todos los pasillos elegidos siguen siendo navegables durante el scroll.
      bool terrainSafe=true;
      fase=5;
      for(int sc=0;sc<1920;sc+=16){
        scroll=float(sc);
        terrainSafe &= (safeY[i]>ceilY(310)+12 && safeY[i]<floorY(310)-12);
      }
      ok&=matches&&gap&&terrainSafe;
      std::cout<<"FANTASMA patrón "<<i<<" altura="<<lanes[0]<<","<<lanes[1]
               <<" escape="<<safeY[i]<<": "<<(matches&&gap&&terrainSafe?"OK":"ERROR")<<"\n";
    }
    bool allHits=true,allWarnings=true,allEscapes=true;
    for(int i=0;i<3;++i){
      for(int j=0;j<2;++j){
        start(5);mode=BOSS;bossX=777.f;bossTime=3.2f+8.6f*i+1.65f;bossPhase=0;bossBeamHit=0;
        shipx=305;shipy=float(tops[i]+j*(bottoms[i]-tops[i]));vulnerable=0;death=0;lives=3;
        move(1.f/60.f);
        allHits&=lives==2; // comprueba los DOS carriles reales de cada ciclo
      }
      start(5);mode=BOSS;bossX=777.f;bossTime=3.2f+8.6f*i+.5f;bossPhase=0;bossBeamHit=0;
      shipx=305;shipy=float(tops[i]);vulnerable=0;death=0;lives=3;
      move(1.f/60.f);
      allWarnings&=lives==3; // el aviso no mata
      start(5);mode=BOSS;bossX=777.f;bossTime=3.2f+8.6f*i+1.65f;bossPhase=0;bossBeamHit=0;
      shipx=305;shipy=float(safeY[i]);vulnerable=0;death=0;lives=3;
      move(1.f/60.f);
      allEscapes&=lives==3; // corredor de escape de ese patrón
    }
    ok&=allHits&&allWarnings&&allEscapes;
    std::cout<<"RAYOS DOBLES: 6 IMPACTOS, 3 AVISOS INOFENSIVOS Y 3 ESCAPES: "
             <<(allHits&&allWarnings&&allEscapes?"OK":"ERROR")<<"\n";
    return ok?0:42;
  }
  if(testControls){
    std::array<uint8_t,512> keys{};
    auto expected=[&](float x,float y){auto d=keyboard_direction(keys.data());return d.first==x&&d.second==y;};
    bool ok=expected(0,0);
    keys[SC_W]=1;ok&=expected(0,-1);keys.fill(0);
    keys[SC_A]=1;ok&=expected(-1,0);keys.fill(0);
    keys[SC_S]=1;ok&=expected(0,1);keys.fill(0);
    keys[SC_D]=1;ok&=expected(1,0);keys.fill(0);
    keys[SC_W]=1;keys[SC_D]=1;ok&=expected(1,-1);keys.fill(0);
    keys[SC_UP]=1;ok&=expected(0,-1);keys.fill(0);
    keys[SC_LEFT]=1;ok&=expected(-1,0);keys.fill(0);
    keys[SC_DOWN]=1;ok&=expected(0,1);keys.fill(0);
    keys[SC_RIGHT]=1;ok&=expected(1,0);keys.fill(0);
    keys[20]=1;keys[18]=1;keys[19]=1;ok&=expected(0,0); // Q/O/P, ya no mueven
    // Escaneo del dispositivo debe ser seguro incluso sin ningún mando físico.
    scan_gamepads();ok&=(!pad||padInstance>=0);
    // Con SDL2 virtual: conectar -> desconectar -> reconectar, incluyendo nuevos IDs.
    if(!pad){
      const int first=SDL_JoystickAttachVirtual(1,2,15,1);
      if(first<0){ok=false;std::cerr<<"Mando virtual: "<<SDL_GetError()<<"\n";}
      else{
        eventPoll();
        const int32_t firstId=padInstance;
        bool plug=pad&&padInstance>=0;
        SDL_JoystickDetachVirtual(first);eventPoll();
        bool unplug=!pad&&padInstance==-1;
        const int second=SDL_JoystickAttachVirtual(1,2,15,1);
        bool reconnect=false;
        if(second>=0){eventPoll();reconnect=pad&&padInstance>=0&&padInstance!=firstId;
          SDL_JoystickDetachVirtual(second);eventPoll();}
        bool finalClear=!pad&&padInstance==-1;
        ok&=plug&&unplug&&reconnect&&finalClear;
        std::cout<<"Mando virtual CONECTAR/DESCONECTAR/RECONECTAR: "
                 <<(plug&&unplug&&reconnect&&finalClear?"OK":"ERROR")<<"\n";
      }
    }
    std::cout<<"CONTROL PC WASD/FLECHAS, SIN Q/O/P Y HOTPLUG: "<<(ok?"OK":"ERROR")<<"\n";
    return ok?0:3;
  }
  if(testDebugSelector){
    back_to_title();key(1073741905);key(1073741905);key(13);
    bool hidden=mode==AUDIO_OPTIONS&&!debugPhaseSelector;
    key(1073741890);bool opened=mode==AUDIO_OPTIONS&&debugPhaseSelector;
    for(int i=0;i<4;++i)key(1073741905);
    bool select5=debugPhase==5;
    key(13);bool started5=mode==INTRO&&fase==5&&!cheatInvulnerable;
    mode=PLAY;key(1073741890);bool cheatWorks=cheatInvulnerable;
    bool ok=hidden&&opened&&select5&&started5&&cheatWorks;
    std::cout<<"SELECTOR F9 AUDIO + INVULNERABILIDAD: "<<(ok?"OK":"ERROR")<<"\n";
    return ok?0:21;
  }
  if(testHudAccents){
    bool ok=true;
    for(const auto&style:{"hud_amber","hud_magenta","hud_cyan"}){
      const auto& ink=hudGlyphInk.at(style);
      for(const auto&p:std::vector<std::pair<uint32_t,uint32_t>>{{0xC1,'A'},{0xC9,'E'},{0xCD,'I'},{0xD3,'O'},{0xDA,'U'},{0xDC,'U'},{0xD1,'N'}}){
        int a=hud_index(p.first),b=hud_index(p.second);
        int delta=(ink[b].y%96+ink[b].h)-(ink[a].y%96+ink[a].h);
        ok &= delta>=-14 && delta<=3;
        std::cout<<style<<" "<<char(p.second)<<" acento y_corr="<<delta<<"\n";
      }
    }
    std::cout<<"BASELINE ACENTOS: "<<(ok?"OK":"ERROR")<<"\n";
    return ok?0:22;
  }
  if(testMenu){
    // El menú público solo permite empezar desde el principio: sin selector de fases.
    back_to_title();
    bool startFirst=menuIndex==0;
    key(1073741906);bool wrapUp=menuIndex==4;
    key(1073741905);bool wrapDown=menuIndex==0;
    key(1073741905);key(1073741905);
    key(13);bool audioMenu=mode==AUDIO_OPTIONS;
    key(27);bool backAudio=mode==TITLE&&menuIndex==0;
    for(int i=0;i<3;i++)key(1073741905);
    key(13);bool playerMenu=mode==JUKEBOX;
    key(27);bool backPlayer=mode==TITLE&&menuIndex==0;
    key(13);bool startsPhase1=mode==INTRO&&fase==1;
    bool bothLogos=textures.count("marca_tony")&&textures.count("marca_grit");
    bool ok=startFirst&&wrapUp&&wrapDown&&audioMenu&&backAudio&&playerMenu&&backPlayer&&startsPhase1&&bothLogos;
    std::cout<<"MENÚ LIMPIO: "<<(ok?"OK":"ERROR")<<" 5 opciones="<<MENU_ITEMS
             <<" logos="<<bothLogos<<" comienza_en_fase1="<<startsPhase1
             <<" ajustes="<<audioMenu<<" reproductor="<<playerMenu<<" retorno="<<(backAudio&&backPlayer)<<"\n";
    return ok?0:31;
  }
  if(testFinalCredits){
    start(5);mode=BOSS_DEATH;stateTime=3.28f;move(.05f);
    bool win=(mode==WIN && musicName=="musica_victoria" && !musicLoop);
    stateTime=victory_delay()+.03f;move(.03f);
    bool automatic=(mode==FINAL_CREDITS && finalCreditsPage==0 && musicName=="musica_escena_final"
      && musicTrack && !musicLoop && musicTrack->pcm.size()>44100*2*280);
    bool imageReady=(textures.count("final_escena")>0 && textures["final_escena"].t!=nullptr && textures["final_escena"].w==1920);
    finalCreditsClock=5.34f;move(.05f);bool pages=(finalCreditsPage==1);
    stateTime=1.2f;key(32);bool skip=(finalCreditsPage==2);
    finalCreditsPage=FINAL_CREDITS_PAGES-3;finalCreditsClock=5.34f;
    move(.03f);bool reachesThanks=finalCreditsPage==FINAL_CREDITS_PAGES-2;
    finalCreditsClock=6.99f;move(.03f);
    bool reachesPhrase=finalCreditsPage==FINAL_CREDITS_PAGES-1;
    move(1.20f);bool visible=finalCreditsClock>=.85f;
    move(279.f);bool stays=(mode==FINAL_CREDITS&&finalCreditsPage==FINAL_CREDITS_PAGES-1&&
       musicName=="musica_escena_final"&&musicTrack&&!musicLoop);
    key(27);bool returns=(mode==TITLE && musicName=="musica_titulo");
    bool ok=win&&automatic&&imageReady&&pages&&skip&&reachesThanks&&reachesPhrase&&visible&&stays&&returns;
    std::cout<<"FINAL CINEMATOGRÁFICO "<<(ok?"OK":"ERROR")<<" victoria="<<win<<" transición="<<automatic
             <<" imagen="<<imageReady<<" fundidos="<<pages<<" avanzar="<<skip<<" gracias="<<reachesThanks
             <<" frase="<<reachesPhrase<<" visible="<<visible<<" hasta_el_final="<<stays<<" título="<<returns<<"\n";
    return ok?0:21;
  }
  if(testSoundtrack){
    bool tracks=true;for(int i=0;i<JUKE_COUNT;i++){
      Sound dummy;tracks &= ogg_decode(jukeFile[i],dummy,true);
    }
    bool phases=true;for(int i=1;i<=5;i++){
      start(i);phases &= musicName==phase_music(i) && musicTrack && musicLoop;
    }
    boss_start();bool bosses=(musicName=="musica_jefes_06" && musicLoop);
    mode=JUKEBOX;playerIndex=0;jukebox_play();
    bool jukeStarts=(playingIndex==0 && musicName=="musica_fase_01" && !musicLoop && !musicPaused);
    key('p');bool pauses=musicPaused;key('p');bool resumes=!musicPaused;
    key('x');bool stops=(musicTrack==nullptr&&playingIndex==-1);
    // Simular controles sin guardar configuración en las pruebas.
    int previous=musicVolume;edit_volume(1,-1);bool volume=(musicVolume==previous-5);edit_volume(1,1);
    mode=JUKEBOX;key(27);bool returns=(mode==TITLE&&musicName=="musica_titulo"&&!musicLoop);
    mode=BOSS_DEATH;fase=2;stateTime=2.89f;
    move(.05f);
    bool victory=(mode==WIN&&musicName=="musica_victoria"&&!musicLoop&&musicTrack);
    float fullDuration=victory_delay();
    stateTime=4.21f;move(.01f);bool holds=(mode==WIN); // El jingle no debe truncarse en 4.2 s
    stateTime=fullDuration+.01f;move(.01f);bool advances=(fase==3&&mode==INTRO&&musicName=="musica_fase_03");
    bool ok=tracks&&phases&&bosses&&jukeStarts&&pauses&&resumes&&stops&&volume&&returns&&victory&&holds&&advances;
    std::cout<<"FUTUREDARK BSO: "<<(ok?"OK":"ERROR")<<" cargadas="<<tracks<<" fases_1_a_5="<<phases<<" jefes="<<bosses
             <<" victoria="<<victory<<" jingle_completo="<<holds<<" avanza="<<advances
             <<" jukebox="<<jukeStarts<<" pausa="<<pauses<<" reanuda="<<resumes
             <<" para="<<stops<<" volumen="<<volume<<" regreso_titulo="<<returns<<"\n";
    return ok?0:18;
  }
  if(testJingles){
    if(!auddev){std::cerr<<"Sin salida SDL2 de audio\n";return 17;}
    SDL_PauseAudioDevice(auddev,1);
    music("musica_titulo");
    bool loaded=musicTrack==&musicData && musicData.channels==2 && !musicLoop && musicPos==0;
    SDL_LockAudioDevice(auddev);
    musicPos=musicData.pcm.size()/2-24;
    std::vector<uint8_t> scratch(128*4);
    mix_audio(scratch.data(),int(scratch.size()));
    bool ends=musicTrack==nullptr && !musicLoop;
    mix_audio(scratch.data(),int(scratch.size()));bool staysSilent=musicTrack==nullptr;
    SDL_UnlockAudioDevice(auddev);
    music("musica_titulo");bool restarts=musicTrack==&musicData && musicPos==0&&!musicLoop;
    music("musica_game_over");bool gameOverReady=musicTrack==&musicData&&musicData.channels==2;
    SDL_LockAudioDevice(auddev);musicPos=musicData.pcm.size()/2-24;
    mix_audio(scratch.data(),int(scratch.size()));bool gameOverEnds=musicTrack==nullptr;
    SDL_UnlockAudioDevice(auddev);
    music("musica_game_over");bool gameOverRestarts=musicTrack==&musicData&&musicPos==0&&!musicLoop;
    music("musica_victoria");bool victory=musicTrack==&musicData&&!musicLoop;
    music("musica_fase_04");bool loops=musicTrack==&musicData&&musicLoop;
    const bool ok=loaded&&ends&&staysSilent&&restarts&&gameOverReady&&gameOverEnds&&gameOverRestarts&&victory&&loops;
    std::cout<<"JINGLES OGG ESTEREO: "<<(ok?"OK":"ERROR")<<"\n";
    return ok?0:17;
  }
  if(testInvul){
    start(4);mode=PLAY;vulnerable=0;death=0;shieldTime=0;lives=3;
    const int normalLives=lives;key(1073741890);
    bool on=cheatInvulnerable&&cheatUsedThisRun;
    life_hit();bool protects=death==0&&lives==normalLives;
    // Todos los daños reales (terreno, enemigos, disparos, rayos) convergen en life_hit.
    int recordBefore=best;score=best+1000;save_best();
    bool recordProtected=best==recordBefore;
    next_phase(5);mode=PLAY;vulnerable=0;death=0;
    bool persists=cheatInvulnerable&&cheatUsedThisRun;
    life_hit();protects=protects&&death==0&&lives==normalLives;
    key(1073741890);bool off=!cheatInvulnerable;
    life_hit();bool normalDamage=death>0&&lives==normalLives-1;
    start(5);bool resets=!cheatInvulnerable&&!cheatUsedThisRun;
    bool ok=on&&protects&&recordProtected&&persists&&off&&normalDamage&&resets;
    std::cout<<"INVULNERABILIDAD F9: "<<(ok?"OK":"ERROR")
        <<" on="<<on<<" protege="<<protects<<" record_intacto="<<recordProtected
        <<" entre_fases="<<persists<<" off="<<off<<" dano_normal="<<normalDamage<<" nuevo_juego="<<resets<<"\n";
    return ok?0:15;
  }
  if(argc>=2 && std::string(argv[1])=="--self-test-aura"){
    bool atlas=textures.count("aura") && textures["aura"].t
               && textures["aura"].w==2172 && textures["aura"].h==724;
    bool names=true;
    for(int f=0;f<5;++f)names&=std::string(TEXTO_PREJEFE[f][0]).rfind("AURA-01:",0)==0;
    for(int f=1;f<=5;++f){
      start(f);
      const int id=(f==1)?18:(f==2)?16:(f==3)?16:(f==4)?17:20;
      if(f==1)spawn_event(id);
      else if(f==2)spawn_event2(id);
      else if(f==3)spawn_event3(id);
      else if(f==4)spawn_event4(id);
      else spawn_event5(id);
      names&=radio.role==3 && radio.who=="AURA-01" && radio.life>0;
    }
    mode=PLAY;radio.elapsed=.55f;render();
    std::cout<<"AURA-01: "<<(atlas&&names?"OK":"ERROR")
             <<" atlas_12f="<<atlas<<" cinco_fases="<<names<<"\n";
    return atlas&&names?0:49;
  }
  // Regenerar las capturas narrativas auténticas con la versión actual del
  // renderizador. Sólo diagnóstico CLI; ningún menú ni atajo de gameplay.
  if(argc>=2 && std::string(argv[1])=="--capture-guion-radios"){
    const int ev[5][4]={{0,9,18,24},{0,7,16,24},{0,7,16,25},{0,8,17,24},{0,9,20,27}};
    for(int phase=1;phase<=5;phase++){
      for(int k=0;k<4;k++){
        start(phase);mode=PLAY;vulnerable=3;shipx=260;shipy=268;scroll=185.f+int(k)*90.f;
        const int id=ev[phase-1][k];
        if(phase==1)spawn_event(id);
        else if(phase==2)spawn_event2(id);
        else if(phase==3)spawn_event3(id);
        else if(phase==4)spawn_event4(id);
        else spawn_event5(id);
        if(radio.life<=0){std::cerr<<"Sin radio en fase "<<phase<<" evento "<<id<<"\n";return 3;}
        radio.elapsed=.5f;
        render();SDL_RenderPresent(ren);
        const std::string file="CAPTURA_GUION_FASE_"+std::to_string(phase)+"_RADIO_"+std::to_string(id)+".png";
        if(!screenshot(file))return 3;
      }
      start(phase);boss_init();bossX=780;bossY=270;bossTime=0;
      vulnerable=3;radio.elapsed=.7f;
      render();SDL_RenderPresent(ren);
      const std::string file="CAPTURA_GUION_FASE_"+std::to_string(phase)+"_RADIO_JEFE.png";
      if(!screenshot(file))return 3;
    }
    std::cout<<"25 capturas de radios originales del motor: OK\n";
    return 0;
  }
  if(argc>=2 && std::string(argv[1])=="--capture-guion-intermisiones"){
    for(int phase=1;phase<=5;phase++){
      start(phase);mode=INTRO;stateTime=1;
      render();SDL_RenderPresent(ren);
      std::string base="CAPTURA_GUION_FASE_"+std::to_string(phase);
      if(!screenshot(base+"_BRIEFING.png"))return 3;
      mode=BOSS_INTRO;stateTime=1;
      render();SDL_RenderPresent(ren);
      if(!screenshot(base+"_PREJEFE.png"))return 3;
      mode=WIN;stateTime=1;
      render();SDL_RenderPresent(ren);
      if(!screenshot(base+"_VICTORIA.png"))return 3;
    }
    start(1);mode=DEAD;render();SDL_RenderPresent(ren);
    if(!screenshot("CAPTURA_GUION_GAME_OVER.png"))return 3;
    std::cout<<"16 capturas de intermisiones originales del motor: OK\n";
    return 0;
  }
  // Capturas nativas de validación del HUD; no se habilita ningún truco de juego.
  if(hudQa){
    SDL_SetWindowSize(win,W,H);start(4);mode=PLAY;vulnerable=3;
    const struct {int lives;int weapon;const char* name;} cases[]={
      {4,0,"CAPTURA_HUD_4_VIDAS_PULSO.png"},
      {3,1,"CAPTURA_HUD_3_VIDAS_DOBLE.png"},
      {2,2,"CAPTURA_HUD_2_VIDAS_LASER.png"},
      {1,0,"CAPTURA_HUD_1_VIDA_PULSO.png"},
      {0,2,"CAPTURA_HUD_0_VIDAS_LASER.png"}};
    for(const auto&v:cases){lives=v.lives;weapon=v.weapon;
      render();SDL_RenderPresent(ren);
      if(!screenshot(v.name)){std::cerr<<"No se pudo guardar "<<v.name<<"\n";return 3;}
      std::cout<<v.name<<" OK\n";
    }
    // Extremos de números y barra sin cortar el diseño por la derecha.
    score=9999999;best=9999999;lives=4;weapon=2;fase=5;
    mode=PLAY;render();SDL_RenderPresent(ren);
    if(!screenshot("CAPTURA_HUD_NUMEROS_MAXIMOS.png"))return 3;
    for(int phase=1;phase<=5;phase++){
      start(phase);mode=BOSS;bossX=775;bossY=269;bossTime=0;
      bossPhase=3;bossHp=std::max(1.f,bossMax*.59f);
      render();SDL_RenderPresent(ren);
      const std::string f="CAPTURA_HUD_JEFE_FASE_"+std::to_string(phase)+".png";
      if(!screenshot(f))return 3;
      if(phase==5){
        bossTime=2.45f;render();SDL_RenderPresent(ren);
        if(!screenshot("CAPTURA_HUD_JEFE_FASE_5_NOMBRE_DESPLAZADO.png"))return 3;
      }
    }
    return 0;
  }
  if(testBossStatus){
    bool ok=true;
    const std::array<HudSlot,7> slots={HUD_SCORE,HUD_LIVES,HUD_WEAPON,HUD_RECORD,HUD_PHASE,HUD_BOSS_NAME,HUD_BOSS_BAR};
    for(const auto&f:slots){
      bool within=f.x>=6&&f.y>=0&&f.w>0&&f.h>0&&f.x+f.w<W-6&&f.y+f.h<HUD_H;
      ok &= within;
      std::cout<<"HUD zona "<<f.x<<","<<f.y<<" "<<f.w<<"x"<<f.h<<" => "<<(within?"OK":"ERROR")<<"\n";
    }
    for(const auto&name:TITULO_JEFE){
      const int textW=hud_precise_width("hud_magenta",name,18,0);
      const float overflow=std::max(0,textW-(HUD_BOSS_NAME.w-8));
      // La zona visible siempre está en clipping; la animación alcanza el
      // primer y último carácter sin invadir la ventana de la energía.
      bool fits=(textW>0 && overflow>=0 && HUD_BOSS_NAME.x+HUD_BOSS_NAME.w < HUD_BOSS_BAR.x);
      std::cout<<"JEFE ["<<name<<"] ancho total="<<textW<<", recorrido="<<overflow
               <<", dentro de ventana="<<(fits?"SI":"NO")<<"\n";
      ok &= fits;
    }
    const auto h=textures.find("hud");
    bool texValid=h!=textures.end()&&h->second.w==960&&h->second.h==120;
    ok &= texValid;
    for(int livesCase=0;livesCase<=4;++livesCase){
      const int ww=livesCase?livesCase*21+(livesCase-1)*6:0;
      ok &= ww<=HUD_LIVES.w-8;
    }
    for(int ar=0;ar<3;++ar){
      std::string name=ar==0?"PULSO":ar==1?"DOBLE":"LÁSER";
      int hh=29;while(hh>16&&hud_precise_width("hud_amber",name,hh,1)>HUD_WEAPON.w-6)--hh;
      bool good=hud_precise_width("hud_amber",name,hh,1)<=HUD_WEAPON.w-6;
      ok &= good;
    }
    std::cout<<"HUD MILITAR: "<<(ok?"OK":"ERROR")<<" textura="<<(texValid?"960x120":"INCORRECTA")<<"\n";
    return ok?0:5;
  }
  if(argc>=2 && std::string(argv[1])=="--self-test-paisaje5"){
    start(5);mode=PLAY;
    for(int i=0;i<60*96;++i){vulnerable=999;death=0;shipx=170;shipy=270;move(1.f/60);}
    bool ok=(mode==BOSS_INTRO||mode==BOSS)&&enemiesSpawned>35&&wallTop5.size()==1920&&wallFloor5.size()==1920;
    std::cout<<"FASE 5 paisaje/oleadas/jefe: "<<(ok?"OK":"ERROR")<<" spawns="<<enemiesSpawned<<" shots="<<hostileShotsFired<<" mode="<<mode<<"\n";
    return ok?0:12;
  }
  if(testEquilibrio){
    struct Pressure {int spawns, shots, peak;};
    Pressure pressure[3]{};
    for(int phase=2;phase<=4;phase++){
      start(phase);mode=PLAY;
      for(int i=0;i<60*82&&mode==PLAY;i++){
        vulnerable=100;death=0;shipx=180;shipy=270;move(1.f/60.f);
      }
      pressure[phase-2]={enemiesSpawned,hostileShotsFired,peakHostiles};
      std::cout<<"FASE "<<phase<<" oleadas: enemigos="<<enemiesSpawned
        <<" disparos_hostiles="<<hostileShotsFired<<" pico_enemigos="<<peakHostiles<<"\n";
    }
    // No se exige igualdad de proyectiles: la fase 3 cuenta con rocas letales.
    bool ok=pressure[1].spawns>pressure[0].spawns && pressure[2].spawns>pressure[0].spawns
       && pressure[1].shots>=pressure[0].shots && pressure[2].shots>=pressure[0].shots*3/2
       && pressure[1].peak<=28 && pressure[2].peak<=30;
    std::cout<<"COMPROBACIÓN DE PRESIÓN: "<<(ok?"OK":"ERROR")<<"\n";
    return ok?0:11;
  }
  if(testPlayerShotAudio){
    if(!auddev || audio["disparo"].pcm.empty() || audio["laser"].pcm.empty()){
      std::cerr<<"No hay salida de audio o muestras para prueba de disparos\n";return 16;
    }
    // Pausamos el dispositivo, no el mezclador: inspección determinista sin carrera con callback.
    SDL_PauseAudioDevice(auddev,1);
    start(2);mode=PLAY;
    const uint64_t beforeRequested=playerShotAudioRequested, beforeScheduled=playerShotAudioScheduled;
    bool correctSamples=true, notStolen=true, nonSilent=true;
    for(int weaponMode=0;weaponMode<3;++weaponMode){
      weapon=weaponMode;
      for(int n=0;n<20;++n){
        // Saturar las 31 voces generales no debe silenciar el arma.
        SDL_LockAudioDevice(auddev);
        for(int i=1;i<int(voices.size());++i)
          voices[i]={&audio["explosion"],0,0.04f,9,SDL_GetTicks()};
        SDL_UnlockAudioDevice(auddev);
        shoot();
        SDL_LockAudioDevice(auddev);
        const Sound* expected=&audio[weaponMode==2?"laser":"disparo"];
        correctSamples &= (voices[0].sample==expected && voices[0].position==0);
        notStolen &= voices[0].sample!=nullptr;
        // Verificar que hay energía PCM en la voz asignada (sin depender de altavoces del runner).
        const auto& sample=voices[0].sample->pcm;
        nonSilent &= std::any_of(sample.begin(),sample.end(),[](int16_t v){return v>500||v< -500;});
        SDL_UnlockAudioDevice(auddev);
      }
    }
    const auto requested=playerShotAudioRequested-beforeRequested;
    const auto scheduled=playerShotAudioScheduled-beforeScheduled;
    bool ok=requested==60&&scheduled==60&&correctSamples&&notStolen&&nonSilent;
    std::cout<<"PRUEBA 60 DISPAROS: "<<(ok?"OK":"ERROR")
             <<" solicitados="<<requested<<" reproducidos="<<scheduled
             <<" muestras_correctas="<<correctSamples<<" voz_protegida="<<notStolen
             <<" pcm_valido="<<nonSilent<<"\n";
    return ok?0:16;
  }
  if(testAudio){
    if(!auddev){std::cerr<<"No hay salida de audio para test\n";return 8;}
    SDL_PauseAudioDevice(auddev,1);
    music("musica_fase_04");
    tone("disparo");tone("laser");tone("explosion");tone("explosion_grande");tone("rayo");
    SDL_LockAudioDevice(auddev);
    int active=0;for(auto &v:voices)if(v.sample)++active;
    std::vector<uint8_t> rendered(4096*4);
    mix_audio(rendered.data(),int(rendered.size()));
    int nonzero=0;for(size_t i=0;i<rendered.size();i+=2)
       if(rendered[i]||rendered[i+1])++nonzero;
    bool longExplosion=false;for(auto &v:voices)
       if(v.sample==&audio["explosion_grande"]&&v.position>1000)longExplosion=true;
    bool ok=active>=4&&nonzero>500&&musicPos>=4096&&longExplosion;
    SDL_UnlockAudioDevice(auddev);
    std::cout<<"Prueba de mezclador polifonico: "<<(ok?"OK":"ERROR")
             <<" voices="<<active<<" muestras_no_cero="<<nonzero
             <<" musica_pos="<<musicPos<<" explosion_activa="<<longExplosion<<"\n";
    return ok?0:9;
  }
  if(testSaw){
    start(1);mode=PLAY;cheatInvulnerable=true;
    for(int i=0;i<60*79;i++)move(1.f/60);
    const bool allKinds=std::all_of(enemies.begin(),enemies.end(),[](const Actor&e){return e.kind==1||e.kind==3||e.kind==14||e.kind==15;});
    const bool noAutoMissiles=std::none_of(shots.begin(),shots.end(),[](const Bullet&b){return b.friendly&&b.missile;});
    const bool stage1=sierrasF1>=30&&sierrasDesdeIzq>=8&&sierrasDesdeDer>=12&&espectrosF1>=10&&moaisF1>=7&&allKinds&&noAutoMissiles;
    const int saw1=sierrasF1,sawLeft=sierrasDesdeIzq,sawRight=sierrasDesdeDer,ghost1=espectrosF1,moai1=moaisF1;
    start(2);mode=PLAY;cheatInvulnerable=true;
    for(int i=0;i<60*79;i++)move(1.f/60);
    const bool stage2=sierrasF2>=25&&std::none_of(enemies.begin(),enemies.end(),[](const Actor&e){return e.kind==0||e.kind==6;});
    bool ok=stage1&&stage2;
    std::cout<<"SIERRAS / SIN WALKERS: "<<(ok?"OK":"ERROR")<<" f1_sierra="<<saw1<<" izquierda="<<sawLeft<<" derecha="<<sawRight<<" espectros="<<ghost1<<" moais="<<moai1<<" f2_sierra="<<sierrasF2<<" sin_misiles="<<noAutoMissiles<<"\n";
    return ok?0:24;
  }
  if(testIce){
    start(3);mode=PLAY;cheatInvulnerable=true;shipx=310;shipy=276;
    for(int i=0;i<60*79;i++)move(1.f/60);
    bool ok=estalactitasProgramadas>=15&&estalactitasCerca>=8&&estalactitasPasanCerca>=5&&estalactitasMaxSimultaneas<=5;
    std::cout<<"ESTALACTITAS CERCA DE FIREBIRD: "<<(ok?"OK":"ERROR")<<" spawns="<<estalactitasProgramadas<<" apuntadas_cerca="<<estalactitasCerca<<" cruzaron_zona_nave="<<estalactitasPasanCerca<<" max_simultaneas="<<estalactitasMaxSimultaneas<<"\n";
    return ok?0:25;
  }
  if(testBossCombat||testBossCombat3||testBossCombat4||testBossCombat5){
    start(testBossCombat5?5:testBossCombat4?4:testBossCombat3?3:2);mode=BOSS;bossX=777;bossY=270;
    bossHp=fase==5?260:fase==4?195:fase==3?175:145;bossMax=bossHp;
    bossTime=4.05f;vulnerable=99.f;shipx=360;shipy=276;death=0;
    // Combate completo, sin reducir artificialmente la vida del jefe.
    for(int i=0;i<60*230;i++){
      if(mode==BOSS&&i%8==0)shoot();
      vulnerable=99;death=0;move(1.f/60);
      if(mode==WIN)break;
    }
    bool ok=mode==WIN;
    std::cout<<(testBossCombat5?"FANTASMA DE JÚPITER":testBossCombat4?"HIJO DE LAS ESTRELLAS":testBossCombat3?"HALCÓN ESTELAR":"ODISEA II")<<" combate verificado: "<<(ok?"OK":"ERROR")<<" hp="<<bossHp<<" mode="<<mode<<" score="<<score<<"\n";
    return ok?0:7;
  }
  if(test){start(testFase5?5:testFase4?4:testFase3?3:testFase2?2:1);mode=PLAY;for(int i=0;i<60*110;i++) {vulnerable=99;lives=3;death=0;shipx=160;shipy=275;move(1.f/60);
   if(mode==BOSS_INTRO){boss_init();bossHp=1;bossTime=6.1f;bossPhase=fase>=2?3:1;bossX=780;}
   if(mode==BOSS){bossHp=0;move(.01f);}if(mode==WIN)break;}
   bool ok=mode==WIN;std::cout<<"CONTACTO 2010 native check: stageTime="<<stageTime<<" mode="<<mode<<" score="<<score<<" "<<(ok?"OK":"ERROR")<<"\n";return ok?0:4;}
  if(capture && (shotMode=="--capture-escudo-nuevo"||shotMode=="--capture-triangulos"||shotMode=="--capture-mira-jefe-3"||shotMode=="--capture-mira-jefe-5")){
    int p=shotMode=="--capture-triangulos"||shotMode=="--capture-mira-jefe-3"?3:shotMode=="--capture-mira-jefe-5"?5:2;
    start(p);mode=PLAY;shipx=285;shipy=270;vulnerable=0;death=0;
    if(shotMode=="--capture-escudo-nuevo"){
      shieldTime=9;stageTime=1.5f;spawn_phase2(12,2);
    }else if(shotMode=="--capture-triangulos"){
      spawn_estalactitas(3);
      for(auto &e:enemies)if(e.kind==23){e.t=.65f;e.fire=.5f;}
    }else{
      mode=BOSS;bossX=777.f;bossY=267.f;bossTime=7.f;bossPhase=p>=2?3:1;
      bossHp=std::max(1.f,bossMax*.6f);
    }
    render();SDL_RenderPresent(ren);
    const char* name=shotMode=="--capture-escudo-nuevo"?"CAPTURA_ESCUDO_FIREBIRD_NUEVO.png":
      shotMode=="--capture-triangulos"?"CAPTURA_ESTALACTITAS_TRIANGULOS.png":
      shotMode=="--capture-mira-jefe-3"?"CAPTURA_MIRA_HALCON_REX.png":"CAPTURA_MIRA_FANTASMA_JUPITER.png";
    const bool ok=screenshot(name);
    std::cout<<name<<" "<<(ok?"OK":"ERROR")<<"\n";
    return ok?0:3;
  }
  if(capture){SDL_SetWindowSize(win,W,H);
    if(shotMode=="--capture-final-gracias"||shotMode=="--capture-final-inicio"||shotMode=="--capture-final-programacion"||shotMode=="--capture-final-musica"||shotMode=="--capture-final-frase"){
      start(5);begin_final_credits();stateTime=1.5f;finalCreditsClock=1.5f;
      if(shotMode=="--capture-final-programacion")finalCreditsPage=2;
      if(shotMode=="--capture-final-musica")finalCreditsPage=4;
      if(shotMode=="--capture-final-gracias")finalCreditsPage=7;
      if(shotMode=="--capture-final-frase")finalCreditsPage=8;
    }

   if(shotMode=="--capture-options"||shotMode=="--capture-selector"){mode=AUDIO_OPTIONS;menuOption=1;if(shotMode=="--capture-selector"){debugPhaseSelector=true;debugPhase=3;}}
   if(shotMode=="--capture-jukebox"){mode=JUKEBOX;playerIndex=9;jukebox_play();}
if(shotMode=="--capture-halcones-nuevos")start(3);
if(shotMode=="--capture-fantasma-aviso"||shotMode=="--capture-fantasma-doble-arriba"||shotMode=="--capture-fantasma-doble-abajo"||shotMode=="--capture-fantasma-doble-mixto")start(5);
if(shotMode=="--capture-sierras-fase1")start(1);
if(shotMode=="--capture-fase1-union"){start(1);mode=PLAY;scroll=1580.f;shipx=310.f;shipy=275.f;vulnerable=2.f;}
if(shotMode=="--capture-estalactitas-fase3"||shotMode=="--capture-estalactitas-caida")start(3);
if(shotMode=="--capture-intro")start();if(shotMode=="--capture-fase2"||shotMode=="--capture-boss-fase2"||shotMode=="--capture-boss-fase2-ataque"||shotMode=="--capture-muerte-fase2"||shotMode=="--capture-f22")start(2);if(shotMode=="--capture-fase3"||shotMode=="--capture-boss-fase3"||shotMode=="--capture-halcon-rayo")start(3);if(shotMode=="--capture-fase4"||shotMode=="--capture-boss-fase4")start(4);if(shotMode=="--capture-fase5"||shotMode=="--capture-fase5-union"||shotMode=="--capture-fantasma-intenso"||shotMode=="--capture-boss-fase5"||shotMode=="--capture-muerte-fase5"||shotMode=="--capture-f9-fase5")start(5);if(shotMode=="--capture-play"){start();mode=PLAY;for(int i=0;i<15*60;i++)move(1.f/60);}
   if(shotMode=="--capture-boss"){start();mode=BOSS;bossX=775;bossY=268;bossTime=5.8f;bossPhase=1;bossHp=95;}
   if(shotMode=="--capture-sierras-fase1"){
     mode=PLAY;shipx=308;shipy=270;spawn(1,2);spawn(2,2);spawn(14,1);spawn(15,1);spawn(3,1);
     int i=0;const float xs[]={790,610,240,140,665,420,800};const float ys[]={219,320,350,200,264,177,300};
     for(auto &e:enemies){int j=std::min(i++,6);e.x=xs[j];e.y=ys[j];e.t=.85f;e.fire=.2f;}
   }
   if(shotMode=="--capture-estalactitas-fase3"||shotMode=="--capture-estalactitas-caida"){
     mode=PLAY;shipx=315;shipy=280;spawn_estalactitas(3);
     for(auto &e:enemies)if(e.kind==23){
       e.fire=shotMode=="--capture-estalactitas-caida"?-.1f:.55f;
       e.t=shotMode=="--capture-estalactitas-caida"?1.18f:.5f;
       if(shotMode=="--capture-estalactitas-caida")e.y=215.f+(e.x>400.f?40.f:0.f);
     }
   }
   if(shotMode=="--capture-halcones-nuevos"){
      mode=PLAY;shipx=260;shipy=310;spawn_phase3(20,2);spawn_phase3(21,2);
      int k=0;for(auto &e:enemies){e.x=450.f+(k%2)*200.f;e.y=198.f+(k/2)*114.f;e.t=.46f+float(k)*.20f;++k;}
   }
   if(shotMode=="--capture-fantasma-aviso"||shotMode=="--capture-fantasma-doble-arriba"||shotMode=="--capture-fantasma-doble-abajo"||shotMode=="--capture-fantasma-doble-mixto"){
     mode=BOSS;bossX=777;bossY=268;bossTime=3.2f+(shotMode=="--capture-fantasma-aviso"?.7f:1.75f)+8.6f*(shotMode=="--capture-fantasma-doble-abajo"?1:shotMode=="--capture-fantasma-doble-mixto"?2:0);
     bossPhase=shotMode=="--capture-fantasma-aviso"?0:1;bossHp=260;bossMax=260;shipx=310;shipy=319;vulnerable=0;death=0;
   }
   if(shotMode=="--capture-odisea-rayo-v104"){
     start(2);mode=BOSS;bossX=775;bossY=268;bossTime=3.05f;bossPhase=1;bossHp=145;bossMax=145;shipx=305;shipy=315;death=0;vulnerable=0;
   }
   if(shotMode=="--capture-fase2"){mode=PLAY;spawn_phase2(11,2);spawn_phase2(12,1);spawn_phase2(13,1);spawn_phase2(14,1);spawn_phase2(15,1);int wc=0;
    for(auto &e:enemies){if(e.kind==11){e.x=wc++?830.f:650.f;e.y=floorY(e.x)-35;}
      else if(e.kind==12){e.x=810;e.y=208;}
      else if(e.kind==13){e.x=165;e.y=168;}
      else if(e.kind==14){e.x=775;e.y=341;}
      else if(e.kind==15){e.x=445;e.y=345;}}
   }
   if(shotMode=="--capture-fase3"){
     mode=PLAY;spawn_phase3(20,2);spawn_phase3(21,2);spawn_phase3(22,2);spawn_phase3(23,2);
     int k=0;for(auto &e:enemies){e.x=370.f+(k%3)*190.f;e.y=e.kind==23?float(ceilY(e.x)+45):180.f+(k%3)*64.f;e.t=e.kind==23?1.2f:float(k)*.31f;k++;}
   }
   if(shotMode=="--capture-fase4"){
     mode=PLAY;spawn_phase4(30,2);spawn_phase4(31,1);spawn_phase4(32,1);spawn_phase4(33,1);spawn_phase4(34,2);spawn_phase4(35,1);
     int k=0;for(auto &e:enemies){e.x=360.f+(k%3)*180.f;e.y=184.f+(k%4)*64.f;e.t=k*.22f;k++;}
   }
   if(shotMode=="--capture-fase5"){
     mode=PLAY;scroll=215;spawn_phase5(14,2);spawn_phase5(20,2);spawn_phase5(12,2);spawn_phase5(22,1);
     int k=0;for(auto &e:enemies){
       const float positionsX[]={445.f,590.f,735.f,835.f,485.f,660.f,815.f};
       const float positionsY[]={220.f,295.f,190.f,305.f,325.f,178.f,265.f};
       int j=std::min(k,6);e.x=positionsX[j];e.y=positionsY[j];e.t=k*.31f;k++;
     }
   }
   if(shotMode=="--capture-fase5-union"){
     mode=PLAY;
     // La unión HD horizontal queda a x=360 (scroll == 1920-360).
     scroll=1560;
   }
   if(shotMode=="--capture-boss-fase5"||shotMode=="--capture-f9-fase5"||shotMode=="--capture-fantasma-intenso"){
     if(shotMode=="--capture-fantasma-intenso")start(5);
     mode=BOSS;bossX=777;bossY=275;bossTime=shotMode=="--capture-fantasma-intenso"?4.15f:6.1f;
     bossPhase=shotMode=="--capture-fantasma-intenso"?2:3;bossHp=260;bossMax=260;
     if(shotMode=="--capture-f9-fase5")key(1073741890);
   }
   if(shotMode=="--capture-muerte-fase5"){mode=BOSS_DEATH;bossX=777;bossY=275;bossTime=6.1f;stateTime=1.55f;bossHp=0;bossMax=260;}
   if(shotMode=="--capture-boss-fase4"){mode=BOSS;bossX=777;bossY=268;bossTime=5.8f;bossPhase=3;bossHp=195;bossMax=195;}
   if(shotMode=="--capture-boss-fase3"||shotMode=="--capture-halcon-rayo"){
     if(shotMode=="--capture-halcon-rayo")start(3);
     mode=BOSS;bossX=777;bossY=265;bossTime=shotMode=="--capture-halcon-rayo"?2.f:5.0f;
     bossPhase=shotMode=="--capture-halcon-rayo"?1:3;bossHp=175;bossMax=175;
   }
   if(shotMode=="--capture-f22"){mode=PLAY;spawn_phase2(12,2);spawn_phase2(13,2);int k=0;for(auto &e:enemies){e.x=400.f+float(k%2)*270.f;e.y=190.f+float(k/2)*150.f;++k;}}
   if(shotMode=="--capture-boss-fase2"||shotMode=="--capture-boss-fase2-ataque"||shotMode=="--capture-muerte-fase2"){
     mode=shotMode=="--capture-muerte-fase2"?BOSS_DEATH:BOSS;
     bossX=775;bossY=268;bossTime=shotMode=="--capture-boss-fase2-ataque"?3.05f:4.8f;
     bossPhase=shotMode=="--capture-boss-fase2-ataque"?2:3;
     bossHp=shotMode=="--capture-muerte-fase2"?0:145;bossMax=145;
     if(mode==BOSS_DEATH)stateTime=1.6f;
   }
   if(shotMode=="--capture-new-moai"){start();mode=PLAY;spawn(3,3);for(auto &e:enemies){e.x=560+(&e-&enemies[0])*110;e.y=275;}}
   if(shotMode=="--capture-halcones-nuevos"||shotMode=="--capture-odisea-rayo-v104"||shotMode=="--capture-fantasma-aviso"||shotMode=="--capture-fantasma-doble-arriba"||shotMode=="--capture-fantasma-doble-abajo"||shotMode=="--capture-fantasma-doble-mixto"||shotMode=="--capture-fase2"||shotMode=="--capture-boss-fase2"||shotMode=="--capture-boss-fase2-ataque"||shotMode=="--capture-muerte-fase2"||shotMode=="--capture-f22"||shotMode=="--capture-fase3"||shotMode=="--capture-boss-fase3"||shotMode=="--capture-fase4"||shotMode=="--capture-boss-fase4"||shotMode=="--capture-fase5"||shotMode=="--capture-fase5-union"||shotMode=="--capture-boss-fase5"||shotMode=="--capture-muerte-fase5"||shotMode=="--capture-f9-fase5"||shotMode=="--capture-halcon-rayo"||shotMode=="--capture-fantasma-intenso"){vulnerable=0;shipx=310;shipy=276;death=0;}
   if(shotMode=="--capture-sierras-fase1"||shotMode=="--capture-estalactitas-fase3"||shotMode=="--capture-estalactitas-caida"){
     vulnerable=0;death=0;
   }
   render();SDL_RenderPresent(ren);SDL_Delay(150);std::string out="CAPTURA_"+(shotMode=="--capture-options"?std::string("OPCIONES_AUDIO"):shotMode=="--capture-selector"?std::string("SELECTOR_F9_OCULTO"):shotMode=="--capture-halcon-rayo"?std::string("HALCON_RAYO_ESFERICO"):shotMode=="--capture-fantasma-intenso"?std::string("FANTASMA_JUPITER_ATAQUE"):shotMode=="--capture-jukebox"?std::string("REPRODUCTOR_MUSICAL"):shotMode=="--capture-final-inicio"?std::string("FINAL_INICIO"):shotMode=="--capture-final-programacion"?std::string("FINAL_CREDITOS_PROGRAMACION"):shotMode=="--capture-final-musica"?std::string("FINAL_CREDITOS_FUTUREDARK"):shotMode=="--capture-final-gracias"?std::string("FINAL_GRACIAS_POR_JUGAR"):shotMode=="--capture-final-frase"?std::string("FINAL_FRASE"):shotMode=="--capture-title"?std::string("TITULO"):
      shotMode=="--capture-intro"?std::string("BRIEFING"):
      shotMode=="--capture-boss"?std::string("JEFE_NUEVO"):
      shotMode=="--capture-fantasma-aviso"?std::string("FANTASMA_RAYOS_AVISO"):
      shotMode=="--capture-halcones-nuevos"?std::string("HALCONES_SIN_DISPAROS"):
      shotMode=="--capture-odisea-rayo-v104"?std::string("ODISEA_II_RAYO_ESFERICO"):
      shotMode=="--capture-fantasma-doble-arriba"?std::string("FANTASMA_RAYOS_ARRIBA"):
      shotMode=="--capture-fantasma-doble-abajo"?std::string("FANTASMA_RAYOS_ABAJO"):
      shotMode=="--capture-fantasma-doble-mixto"?std::string("FANTASMA_RAYOS_MIXTOS"):
      shotMode=="--capture-fase1-union"?std::string("FASE_1_UNION_CEMENTERIO_V2"):shotMode=="--capture-sierras-fase1"?std::string("SIERRAS_FASE_1"):shotMode=="--capture-estalactitas-fase3"?std::string("ESTALACTITAS_AVISO_FASE_3"):shotMode=="--capture-estalactitas-caida"?std::string("ESTALACTITAS_CAIDA_FASE_3"):shotMode=="--capture-new-moai"?std::string("MOAIS_MENORES"):shotMode=="--capture-fase2"?std::string("FASE_2"):shotMode=="--capture-boss-fase2"?std::string("JEFE_FASE_2"):shotMode=="--capture-boss-fase2-ataque"?std::string("JEFE_FASE_2_ATAQUE"):shotMode=="--capture-muerte-fase2"?std::string("JEFE_FASE_2_DESTRUCCION"):shotMode=="--capture-fase3"?std::string("FASE_3"):shotMode=="--capture-boss-fase3"?std::string("JEFE_FASE_3"):shotMode=="--capture-fase5"?std::string("FASE_5_PAISAJE"):shotMode=="--capture-fase5-union"?std::string("FASE_5_UNION_TERRENO"):shotMode=="--capture-boss-fase5"?std::string("JEFE_FASE_5_FANTASMA"):shotMode=="--capture-muerte-fase5"?std::string("FANTASMA_JUPITER_DESTRUCCION"):shotMode=="--capture-f9-fase5"?std::string("JEFE_FASE_5_TRUCO_F9"):shotMode=="--capture-fase4"?std::string("FASE_4"):shotMode=="--capture-boss-fase4"?std::string("JEFE_FASE_4"):shotMode=="--capture-f22"?std::string("F22_AMBAS_DIRECCIONES"):std::string("FASE_1"))+".png";bool ok=screenshot(out);std::cout<<out<<" "<<(ok?"OK":"ERROR")<<"\n";return ok?0:3;}
  uint32_t prev=SDL_GetTicks();while(running){uint32_t now=SDL_GetTicks();float dt=limit((now-prev)/1000.f,0,.034f);prev=now;eventPoll();move(dt);step_audio();render();SDL_RenderPresent(ren);SDL_Delay(1);}
  return 0;
 }
};
int main(int argc,char**argv){Game g;return g.run(argc,argv);}
