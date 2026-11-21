#!/usr/bin/env python3
"""CONTACTO 2010 V1.1, dossier editorial revisado. ReportLab + recursos reales, sin spoilers.
Generación íntegra y determinista, con comprobación estricta de medidas de texto.
"""
from pathlib import Path
import math, re, hashlib, html, io
from PIL import Image
from reportlab.pdfgen import canvas
from reportlab.lib.colors import HexColor, Color
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus import Paragraph
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.utils import ImageReader

R=Path(__file__).resolve().parents[2]
OUT=R/'documentacion/ATLAS_MEMORIA_MANUAL_GRIT_NOVA_CONTACTO_2010_V1.1.pdf'
CPT=R/'capturas'
FNT=Path('/usr/share/fonts/truetype/dejavu')
fontfiles={'Regular':'DejaVuSans.ttf','Bold':'DejaVuSans-Bold.ttf','Cond':'DejaVuSansCondensed.ttf','CondBold':'DejaVuSansCondensed-Bold.ttf','Mono':'DejaVuSansMono.ttf'}
for name,file in fontfiles.items():pdfmetrics.registerFont(TTFont(name,str(FNT/file)))
pdfmetrics.registerFontFamily('Regular',normal='Regular',bold='Bold',italic='Regular',boldItalic='Bold')
PW,PH=1280,800
NAV=HexColor('#071320');MID=HexColor('#0d2134');CARD=HexColor('#122a40');STROKE=HexColor('#28435b')
WHITE=HexColor('#f7fafc');SUB=HexColor('#c2d6e5');SOFT=HexColor('#91b0c4')
CYAN=HexColor('#73d6f4');AMBER=HexColor('#ffbf6e');PINK=HexColor('#f5a7e8')
C=canvas.Canvas(str(OUT),pagesize=(PW,PH),pageCompression=1,invariant=1)
C.setTitle('CONTACTO 2010 V1.1 | Atlas, memoria, manual y GRIT NOVA ENGINE | Edición revisada')
C.setAuthor('Tony C. Ware · GPT-6 OpenAI')
C.setSubject('Guía ilustrada del juego y documentación técnica del motor C++17 / SDL2')
num=0
pagetitles=[]
issues=[]
import tempfile
cache=Path(tempfile.gettempdir())/'contacto_2010_dossier_images';cache.mkdir(exist_ok=True)

def jpg(path,maxpix=4000):
    path=Path(path)
    if not path.exists():raise FileNotFoundError(path)
    fingerprint=hashlib.sha256((str(path)+str(path.stat().st_mtime_ns)).encode()).hexdigest()[:20]
    target=cache/(fingerprint+'.jpg')
    if not target.exists():
        with Image.open(path) as im:
            im=im.convert('RGBA')
            back=Image.new('RGBA',im.size,(7,19,32,255));back.alpha_composite(im)
            rgb=back.convert('RGB')
            if max(rgb.size)>maxpix:rgb.thumbnail((maxpix,maxpix),Image.Resampling.LANCZOS)
            rgb.save(target,quality=95,subsampling=0,optimize=True)
    return target

def draw_img(path,x,y,w,h,mode='contain',edge=True):
    p=jpg(path)
    with Image.open(p) as im:iW,iH=im.size
    C.setFillColor(HexColor('#050f19'));C.roundRect(x-4,y-4,w+8,h+8,8,fill=1,stroke=0)
    sc=min(w/iW,h/iH) if mode=='contain' else max(w/iW,h/iH)
    dw,dh=iW*sc,iH*sc
    if mode=='cover':
        C.saveState();c=C.beginPath();c.roundRect(x,y,w,h,7);C.clipPath(c,stroke=0,fill=0)
    C.drawImage(str(p),x+(w-dw)/2,y+(h-dh)/2,width=dw,height=dh,mask='auto')
    if mode=='cover':C.restoreState()
    if edge:
        C.setStrokeColor(STROKE);C.roundRect(x-4,y-4,w+8,h+8,8,fill=0,stroke=1)

def text(s,x,y,size=15,color=WHITE,font='Regular',align='left'):
    C.setFillColor(color);C.setFont(font,size)
    if align=='center': C.drawCentredString(x,y,s)
    elif align=='right': C.drawRightString(x,y,s)
    else: C.drawString(x,y,s)

def rich(s,x,top,w,h,size=16,leading=None,color=SUB,font='Regular',minsize=12):
    # Measure in a bounded region; never paint beyond the box.
    leading=leading or size*1.44
    body=html.escape(s).replace('\n','<br/>')
    body=re.sub(r'&lt;b&gt;(.*?)&lt;/b&gt;',r'<b>\1</b>',body)
    best=None
    for sz in [size-i*.5 for i in range(int((size-minsize)*2)+1)]:
        sty=ParagraphStyle('p',fontName=font,fontSize=sz,leading=sz*leading/size,textColor=color,allowWidows=0,allowOrphans=0)
        p=Paragraph(body,sty);_,hh=p.wrap(w,9999)
        if hh<=h-3:
            best=p;bh=hh;break
    if best is None:
        raise ValueError(f'El párrafo no cabe ({h}px, {w}px): {s[:65]}...')
    best.drawOn(C,x,top-bh)
    return bh

def base():
    C.setFillColor(NAV);C.rect(0,0,PW,PH,fill=1,stroke=0)
    C.setFillColor(MID);C.rect(0,0,PW,61,fill=1,stroke=0)
    C.setStrokeColor(STROKE);C.line(55,62,PW-55,62)
    C.setStrokeColor(STROKE);C.line(55,739,PW-55,739)
    text('CONTACTO 2010   /   GRIT NOVA ENGINE',58,754,13,AMBER,'CondBold')
    text('DOSSIER V1.1  ·  EDICIÓN REVISADA',PW-58,754,11,SOFT,'Bold','right')

def page(title,section,subtitle=''):
    global num
    num+=1;pagetitles.append(title)
    C.setPageSize((PW,PH));base()
    text(section.upper(),64,705,13,CYAN,'CondBold')
    text(title,61,642,40,WHITE,'CondBold')
    if subtitle:rich(subtitle,65,611,PW-130,54,15,21,SOFT)
    text('TONY C. WARE   ·   FUTUREDARK   ·   GRIT NOVA',58,32,11,SOFT,'Cond')
    text(f'V1.1     /     {num:02d}',PW-59,32,11,CYAN,'Bold','right')
    C.bookmarkPage(f'p{num}');C.addOutlineEntry(f'{num:02d} · {title}',f'p{num}',level=0,closed=False)

def done():C.showPage()

def panel(x,y,w,h,title,body,accent=CYAN,sz=16):
    C.setFillColor(CARD);C.roundRect(x,y,w,h,11,fill=1,stroke=0)
    C.setStrokeColor(STROKE);C.roundRect(x,y,w,h,11,fill=0,stroke=1)
    C.setFillColor(accent);C.roundRect(x+17,y+h-35,5,17,2,fill=1,stroke=0)
    text(title.upper(),x+35,y+h-31,15,accent,'CondBold')
    rich(body,x+21,y+h-54,w-42,h-71,size=sz,color=SUB)

def cards_page(title,sec,desc,blocks):
    page(title,sec,desc)
    # Four or six bounded panels; no overlap with title or footer.
    if len(blocks)<=4:
        coords=[(61,323,557,220),(662,323,557,220),(61,87,557,220),(662,87,557,220)]
    else:
        coords=[(61,380,557,164),(662,380,557,164),(61,226,557,140),(662,226,557,140),(61,80,557,132),(662,80,557,132)]
    for i,(tit,body) in enumerate(blocks):
        x,y,w,h=coords[i];panel(x,y,w,h,tit,body,CYAN if i%2==0 else AMBER,15 if len(blocks)<=4 else 13.5)
    done()

def shot(title,sec,path,lead,blocks=()):
    page(title,sec,lead)
    draw_img(path,62,107,778,441)
    if blocks:
        h= (441 - (len(blocks)-1)*11) /len(blocks)
        for i,(tit,body) in enumerate(blocks):
            panel(872,107 + (len(blocks)-1-i)*(h+11),348,h,tit,body,AMBER if i==0 else CYAN,14)
    done()

def pair(title,sec,a,b,lead,notes=('','')):
    page(title,sec,lead)
    draw_img(a,62,202,560,315)
    draw_img(b,657,202,560,315)
    if notes[0]:panel(62,83,560,92,notes[0],'Captura tomada directamente del ejecutable.',CYAN,13)
    if notes[1]:panel(657,83,560,92,notes[1],'Revisión visual sobre la V1.1.',AMBER,13)
    done()

def full_image(path,outline_name):
    global num
    p=jpg(path,maxpix=4100)
    with Image.open(p) as im:iw,ih=im.size
    width=1020 if ih>iw else 1320
    height=width*ih/iw
    num+=1;pagetitles.append(outline_name)
    C.setPageSize((width,height));C.drawImage(str(p),0,0,width=width,height=height)
    C.bookmarkPage(f'p{num}');C.addOutlineEntry(f'{num:02d} · {outline_name}',f'p{num}',0,False)
    C.showPage()

def divider(title,tag,subtitle):
    page(title,tag)
    C.setFillColor(HexColor('#092238'));C.roundRect(62,130,1156,412,24,stroke=0,fill=1)
    C.setFillColor(CYAN);C.rect(90,169,9,323,stroke=0,fill=1)
    text(title,124,415,54,WHITE,'CondBold')
    rich(subtitle,125,364,1018,165,24,32,SUB)
    done()

# PLIEGOS PRINCIPALES. Portada original HQ a sangre, sin caja ni recorte.
full_image(R/'documentacion/caratulas/CONTACTO_2010_CARATULA_PRINCIPAL_HD.png','Carátula principal · alta resolución')

cards_page('Una misión, cinco mundos','01 / Presentación',
    'CONTACTO 2010 es un matamarcianos horizontal nativo para PC Linux. Este dossier reúne el manual, el atlas visual y la documentación de su tecnología.',[
      ('Edición jugable','Cinco etapas enlazadas con enemigos y jefes propios, terrenos animados, efectos y un relato contado mediante comunicaciones.'),
      ('La Firebird-01','Interceptora militar experimental cuyo diseño industrial y sprites de 16 fotogramas se han revisado para esta versión.'),
      ('Criterio editorial','Las imágenes de juego que se muestran proceden del ejecutable real. Las láminas artísticas se presentan como ilustraciones, no como capturas.'),
      ('Sin desvelar el desenlace','El dossier evita mostrar al adversario definitivo, las escenas conclusivas y las opciones reservadas a pruebas de desarrollo.')])

cards_page('Cómo leer este dossier','02 / Orientación','Un volumen para jugar, consultar y reutilizar el trabajo de programación y diseño.',[
  ('Primera parte · juego','Argumento, controles, armas, HUD, banda sonora, fases y peligros. Los nombres y secuencias se documentan sin revelar el desenlace.'),
  ('Segunda parte · tecnología','Arquitectura real de GRIT NOVA ENGINE 1.1, funciones extraídas a biblioteca y lógica que continúa en el código del videojuego.'),
  ('Tercera parte · producción','Herramientas, activos, formato de terrenos, compilación, comprobaciones y estado de la edición.'),
  ('Archivo visual','La lámina industrial se reproduce íntegra. El cierre del volumen ofrece una galería sin duplicados de las carátulas aportadas.')])

shot('La señal imposible','03 / Universo',CPT/'CAPTURA_FASE_1.png',
    'Año 2010. Tras perder contacto con la nave científica Odisea II, una señal inexplicable conduce a la Firebird-01 hacia Júpiter.',[
    ('Escenario','Ruinas de órbita, laboratorios abandonados, el hielo de Europa y fenómenos cósmicos que no responden a ninguna explicación convencional.'),
    ('Misión','Sobrevivir a las oleadas, encontrar rutas de paso y neutralizar las defensas que impiden seguir avanzando.'),
    ('Narrador','Operadora, piloto, científica y comandante intervienen con mensajes de radio dentro de la acción.')])

cards_page('Mandos de la Firebird-01','04 / Manual','Control rápido, sin complicaciones: el movimiento y el disparo son el centro de la experiencia.',[
 ('Movimiento','W / A / S / D: arriba, izquierda, abajo y derecha. También funcionan las flechas de dirección.'),
 ('Combate','ESPACIO: disparar. Mantén la posición y busca los huecos cuando el escenario se llena de enemigos y proyectiles.'),
 ('Mando compatible','Admite mando reconocido por SDL2 GameController, con detección de conexión, desconexión y reconexión durante la ejecución.'),
 ('Pantalla y pausa','F11: alterna pantalla completa. ESC: pausa o regreso al menú, según la pantalla actual.')])

shot('El HUD militar','05 / Manual',CPT/'CAPTURA_HUD_3_VIDAS_DOBLE.png',
    'Instrumental integrado: el juego reserva una franja superior de 120 píxeles para ofrecer toda la información sin reducir la zona de combate.',[
 ('Instrumentos','PUNTOS, VIDAS, ARMA, RÉCORD y FASE se mantienen dentro de los espacios reales de su panel.'),
 ('Combate contra jefes','JEFE y su barra energética ocupan un canal específico del HUD; las cifras y los nombres largos respetan los límites de cada ventana.'),
 ('Lectura','Los valores utilizan atlas tipográficos propios y los iconos de vidas permanecen agrupados y centrados.')])

shot('Armas y supervivencia','06 / Manual',CPT/'CAPTURA_HUD_2_VIDAS_LASER.png',
    'La Firebird-01 cambia de armamento según los recursos obtenidos durante la misión.',[
 ('Pulso','Configuración inicial. Disparo ágil y reconocible con su propia respuesta sonora.'),
 ('Doble','Amplía la cobertura y permite enfrentarse a formaciones más densas.'),
 ('Láser','Armamento concentrado con grafismo diferenciado; el HUD informa en todo momento de cuál está equipado.')])

shot('Escudo energético','07 / Sistema defensivo',CPT/'CAPTURA_FIREBIRD_01_ESCUDO.png',
    'Un campo ovalado rodea la nueva Firebird-01 sin tapar su silueta industrial.',[
 ('Protección','Al activarse, la nave puede resistir temporalmente el contacto o un proyectil. El impacto consume la protección antes que una vida.'),
 ('Presentación','Cuatro intensidades visuales RGBA acompañan la actividad del escudo. Su centro transparente mantiene visible el fuselaje.'),
 ('Precisión','El dibujo del escudo y la hitbox están ajustados al mismo punto de referencia de la nave.')])

shot('La Firebird-01 revisada','08 / Diseño jugable',CPT/'CAPTURA_FIREBIRD_01_VUELO_Y_DISPARO.png',
    'El sprite de la nave parte de una hoja industrial de 16 fotogramas: ocho de vuelo y ocho de daño y destrucción.',[
 ('Proporciones','La silueta visible se ajusta a las celdas RGBA sin deformar su diseño de alas y motores.'),
 ('Impacto justo','La colisión es más reducida que la silueta completa: se concentra en el fuselaje para mantener el estilo arcade.'),
 ('Disparo independiente','Las bocas del armamento están alineadas con el morro; los proyectiles pertenecen al motor y no vienen pintados en el sprite.')])

# LÁMINA INDUSTRIAL A PÁGINA COMPLETA, SIN CAPTURAS NI MARCOS.
full_image(R/'desarrollo/material_original/arte/FIREBIRD_01_DEFINITIVA/LAMINA_INDUSTRIAL_FIREBIRD_V3.png','Lámina industrial Firebird-01 · íntegra')

shot('01 · La señal imposible','09 / Atlas de campaña',CPT/'CAPTURA_FASE_1.png',
    'El Cementerio de Escombros V2 sustituye al terreno antiguo con un entorno legible de chatarra orbital y un amplio corredor de vuelo.',[
 ('Oleadas','Sierras Rodantes entrando por ambos lados, espectros y moáis menores.'),
 ('Terreno V2','Techo y suelo independientes adaptados a 1920 × 540, desplazamiento horizontal continuo y colisiones reconstruidas.'),
 ('Pacing','El entorno enmarca Júpiter y los asteroides sin llenar de obstáculos el centro de la pantalla.')])

shot('Cementerio V2 · unión real','10 / Geometría y scroll',CPT/'CAPTURA_FASE_1_UNION_CEMENTERIO_V2.png',
    'Captura obtenida con el scroll cruzando la costura del mapa. El borde inferior y el inicio bajo el HUD permanecen cubiertos.',[
 ('Loop verificado','Los extremos del PNG coinciden y la franja transparente inferior de la versión previa queda fuera del encuadre.'),
 ('Colisiones','El BIN contiene una pareja de límites techo/suelo por cada columna: 1920 muestras.'),
 ('Zona libre','Corredor mínimo de 267 píxeles tras recolocar el suelo hacia abajo y el techo hacia arriba.')])

shot('El primer guardián','11 / Atlas de campaña',CPT/'CAPTURA_JEFE_FASE_1_MOAI.png',
    'Los encuentros de jefe tienen patrones reconocibles y una ventana de daño indicada por el sistema de adquisición de objetivos.',[
 ('Lectura de combate','Observar al enemigo, detectar el momento seguro y evitar descargas simultáneas.'),
 ('Mira dinámica','La retícula aparece sobre el punto débil solamente cuando la lógica acepta impactos.'),
 ('Información','La energía del enfrentamiento se muestra en el panel, no sobre la ilustración del enemigo.')])

shot('02 · La nave del silencio','12 / Atlas de campaña',CPT/'CAPTURA_FASE_2.png',
    'El laboratorio perdido de la Odisea II mezcla maquinaria militar con formas de vida que han alterado su estructura.',[
 ('Enemigos','F-22 espaciales, espectros y Sierras Rodantes en lugar de los caminantes antiguos.'),
 ('Atmósfera','La arquitectura científica está invadida por elementos alienígenas.'),
 ('Diseño','Se conserva un número limitado de familias por escenario para dar identidad propia a cada fase.')])

shot('La Odisea II asimilada','13 / Encuentro',CPT/'CAPTURA_ODISEA_II_RAYO_ESFERICO.png',
    'El rayo enemigo utiliza esfera de energía, cuello ahusado y haz continuo; el efecto es compartido con el Halcón Estelar Rex.',[
 ('Preparación','La animación diferencia la carga del ataque y su descarga para dejar margen de maniobra.'),
 ('Objetivo','El núcleo del adversario abre ventanas para contraatacar.'),
 ('Realización','Efectos de proyectiles y rayos independientes de los fotogramas de la nave enemiga.')])

shot('03 · Las criptas de Europa','14 / Atlas de campaña',CPT/'CAPTURA_FASE_3.png',
    'Bajo las capas de hielo, el espacio se estrecha y los peligros dejan de venir únicamente del frente.',[
 ('Criaturas','Halcones Estelares y moáis menores se combinan con amenazas del entorno.'),
 ('Riesgo ambiental','Las estalactitas caen desde el techo y obligan a maniobrar cerca de su punto de impacto.'),
 ('Animación','Fondo, techo y suelo conviven con alertas breves que no saturan la pantalla.')])

shot('El aviso de las estalactitas','15 / Mecánica de riesgo',CPT/'CAPTURA_ESTALACTITAS_TRIANGULOS.png',
    'Los triángulos parpadean antes de la caída. El hielo permanece inmóvil durante la advertencia y después desciende verticalmente.',[
 ('Señal mínima','Solo se utiliza el triángulo del atlas de peligro, sin flechas ni anillos adicionales.'),
 ('Reacción','El jugador puede apartarse durante el aviso: el objeto no sigue el movimiento de la Firebird-01.'),
 ('Ritmo','Caídas más frecuentes y próximas a la trayectoria de vuelo, con control del número simultáneo.')])

shot('El Halcón Estelar Rex','16 / Encuentro',CPT/'CAPTURA_JEFE_FASE_3_HALCON.png',
    'Una descarga roja de gran alcance convierte al desplazamiento vertical en una decisión esencial.',[
 ('Rayo esférico','El efecto reúne núcleo, esfera y haz en una transición continua.'),
 ('Contrataque','La vulnerabilidad se comunica visualmente al terminar el ciclo ofensivo.'),
 ('Legibilidad','Las señales se concentran en el punto débil y no ocultan el arte del jefe.')])

shot('04 · La cuna de las estrellas','17 / Atlas de campaña',CPT/'CAPTURA_FASE_4.png',
    'Un paisaje biológico de colores eléctricos cambia el tono de la misión hacia la ciencia ficción orgánica.',[
 ('Formas vivas','Células estelares, células devoradoras y amenazas aéreas componen sus oleadas.'),
 ('Supervivencia','El terreno animado exige distinguir obstáculos y proyectiles entre estructuras vivientes.'),
 ('Identidad','La fase conserva su naturaleza orgánica: no reutiliza maquinaria enemiga donde visualmente desentonaría.')])

shot('La vida que protege su núcleo','18 / Encuentro',CPT/'CAPTURA_JEFE_FASE_4_HIJO_ESTRELLAS.png',
    'La criatura del cuarto encuentro alterna oleadas, pulsos y ventanas de ataque reconocibles.',[
 ('Punto vulnerable','La mira aparece sincronizada con la lógica de daño, nunca durante el cierre del núcleo.'),
 ('Ataques','Orbes y anomalías gravitatorias exigen buscar posiciones libres.'),
 ('Intención','La dificultad se basa en anticipar y esquivar, no en impactos inevitablemente encadenados.')])

shot('05 · El corazón de Júpiter','19 / Atlas de campaña',CPT/'CAPTURA_FASE_5_PAISAJE.png',
    'Los remolinos y las almas jovianas forman el paisaje de la última etapa. El recorrido y el desenlace se reservan para quien juegue.',[
 ('Escenario','Techo y suelo RGBA continuos de 1920 × 540 y fondo de tormentas atmosféricas.'),
 ('Diseño','La decoración de los extremos deja espacio central para moverse y leer los ataques.'),
 ('Sin anticipos','Este atlas muestra el paisaje, no la confrontación definitiva ni las escenas posteriores.')])

cards_page('La narración en combate','20 / Guion aprobado','Los cuatro portavoces hacen avanzar el relato sin detener el scroll.',[
 ('Operadora','Presenta el estado de la misión y las nuevas amenazas mediante mensajes breves al empezar las fases.'),
 ('Piloto','Reacciona a lo que aparece en pantalla. Su identidad permanece abierta para mantener la atmósfera del juego.'),
 ('Científica','Aporta información sobre la naturaleza de las señales y los organismos encontrados.'),
 ('Comandante','Marca objetivos de avance y activa la preparación ante encuentros importantes.')])

pair('Briefings y comunicaciones','21 / Guion en pantalla',
     CPT/'CAPTURA_GUION_FASE_1_BRIEFING.png',CPT/'CAPTURA_GUION_FASE_2_RADIO_7.png',
     'Las secuencias reproducen el guion aprobado por Tony C. Ware y se muestran directamente desde el ejecutable.',
     ('BRIEFING DE ETAPA','RADIO EN PLENO COMBATE'))

pair('Avisos y presentación','22 / Guion en pantalla',
     CPT/'CAPTURA_GUION_FASE_3_PREJEFE.png',CPT/'CAPTURA_GUION_FASE_4_RADIO_8.png',
     'Las transmisiones de entrada se activan al comenzar realmente el combate para evitar que el mensaje expire detrás de una intermisión.',
     ('PRESENTACIÓN DEL ENCUENTRO','TRANSMISIÓN BREVE'))

shot('Sistemas de adquisición','23 / Señales de combate',CPT/'CAPTURA_MIRA_HALCON_REX.png',
    'Una pequeña retícula identifica el punto vulnerable. Su aparición debe coincidir siempre con un instante en que el arma causa daño.',[
 ('Solo dos familias','Triángulos para caída de hielo y retículas para puntos débiles. El resto del atlas no se utiliza.'),
 ('Coordenadas reales','La retícula sigue el objetivo cuando se desplaza por el escenario.'),
 ('Diseño ético','No se indican blancos invulnerables ni zonas de ataque ficticias.')])

cards_page('Un motor real, no una promesa','24 / GRIT NOVA ENGINE','La versión 1.1 separa un núcleo de software reutilizable y conserva la lógica del juego donde corresponde.',[
 ('Motor independiente','motor/grit_nova_engine.hpp y .cpp compilan una biblioteca estática libgritnova.a y exponen servicios de entrada, PNG, audio y render.'),
 ('Código del juego','fuentes/contacto_native.cpp contiene la campaña, los estados, la IA, los enemigos, las armas, el terreno, las comunicaciones y la interfaz.'),
 ('Entorno técnico','C++17, SDL2 y libpng. El motor está pensado para aplicaciones 2D nativas en Linux.'),
 ('Límite declarado','Todavía no es un editor visual ni una plataforma de creación sin código. Es una biblioteca funcional de bajo nivel utilizada por este juego.')])

cards_page('API del núcleo','25 / GRIT NOVA ENGINE','Las funciones públicas pueden revisarse directamente en grit_nova_engine.hpp.',[
 ('Imagen y render','load_png_texture() decodifica PNG RGBA. draw_box() y draw_line() dibujan primitivas sencillas sobre SDL_Renderer.'),
 ('Control','keyboard_direction() traduce WASD y flechas a dos ejes. open_controller(), close_controller() y scan_controllers() atienden cambios del mando.'),
 ('Audio','mix_mono_s16() mezcla música PCM y hasta 32 voces de efectos, con controles independientes de volumen.'),
 ('Texto','uppercase_utf8() separa puntos de código Unicode para la renderización con los atlas de glifos del juego.')])

cards_page('Tubería gráfica','26 / GRIT NOVA ENGINE','Desde los materiales originales hasta cada fotograma en pantalla.',[
 ('Carga de recursos','El PNG se decodifica con libpng, se convierte a una textura SDL2 y se conserva su anchura y altura reales.'),
 ('Coordenadas','La resolución lógica de 960 × 540 separa la franja de instrumentos de la zona jugable, sin depender de la resolución física del monitor.'),
 ('Composición','Fondo, terreno, entidades, efectos, nave, interfaz y transmisiones se ordenan de forma consistente.'),
 ('Máscaras','Las transparencias RGBA permiten efectos independientes y una superposición suave con la ilustración de los escenarios.')])

cards_page('Modelo de sonido','27 / GRIT NOVA ENGINE','El audio original se conserva y el motor mezcla recursos preparados para reproducirse con baja latencia.',[
 ('Originales','Diez temas OGG de FutureDark, alias musical de Tony C. Ware, guardados sin sustituir los masters.'),
 ('Música del motor','Los audios activos utilizan PCM mono 16-bit a 22.050 Hz. La mezcla maneja reproducción continua o única según contexto.'),
 ('Efectos simultáneos','Mezclador de 32 voces para impactos, disparos, alertas y explosiones, con volúmenes independientes.'),
 ('Jingles','Título, Game Over y Victoria se asignan a su pantalla correspondiente, con entradas ajustadas para minimizar silencio inicial.')])

cards_page('Entrada y sesiones','28 / GRIT NOVA ENGINE','El teclado y los mandos SDL2 comparten el objetivo de ofrecer movimientos consistentes.',[
 ('Teclas','El controlador devuelve un vector de movimiento en dos ejes, con WASD y flechas como entradas equivalentes.'),
 ('Hotplug','El identificador de instancia permite detectar qué mando se ha desconectado y gestionar su reconexión.'),
 ('Continuidad','Se conserva el control por teclado aunque no haya mando conectado o el dispositivo desaparezca.'),
 ('Prueba aislada','El núcleo cuenta con un programa de prueba que no depende de la campaña ni de sus enemigos.')])

cards_page('Estados y simulación','29 / Arquitectura del juego','La capa específica del videojuego coordina el tiempo, la progresión y las reglas de enfrentamiento.',[
 ('Estados','Pantalla de título, briefings, combate, presentación de enemigos importantes, pausa y resultados tienen transición controlada.'),
 ('Cronómetros','Los contadores de estado, etapa y combate mantienen separados los ritmos de la narrativa y de la acción.'),
 ('Entidades','Estructuras compactas para enemigos, proyectiles, premios, efectos y comunicaciones.'),
 ('Ataque y defensa','Colisiones y ventanas de daño gobiernan energía, escudo, vidas, puntos y desaparición de enemigos.')])

cards_page('Terrenos y colisiones','30 / Arquitectura del juego','El escenario gráfico y la geometría de colisión siguen relacionados pero no son idénticos.',[
 ('BIN por fase','Los límites verticales se almacenan como columnas de techo y suelo, con su anchura declarada en cabecera.'),
 ('Precisión artística','El alfa de los PNG se suaviza y se aplica un margen para que cables o fragmentos diminutos no formen trampas.'),
 ('Costura horizontal','Los paneles de 1920 píxeles permiten desplazar bloques sucesivos de textura sin un salto visual en el borde.'),
 ('Cementerio V2 revisado','Techo elevado 3 px, suelo rebajado 9 px, sin banda transparente visible. Corredor mínimo calculado: 267 píxeles.')])

cards_page('Organización del proyecto','31 / Fuentes y recursos','El paquete integra código modular, recursos activos y originales conservados por separado.',[
 ('motor/','Biblioteca GRIT NOVA, cabeceras, pruebas unitarias y script de construcción independiente.'),
 ('fuentes/','Código nativo específico de CONTACTO 2010: fases, combatientes, lógica de oleadas, interfaz y sucesos.'),
 ('assets/','Gráficos PNG RGBA, BIN de terreno, WAV PCM, fuente visual y atlas de sprites utilizados por el ejecutable.'),
 ('desarrollo/ y documentacion/','Masters originales, herramientas de transformación, verificación, historial técnico y este dossier.')])

cards_page('Construir y ejecutar','32 / Compilación','El proyecto entrega un ejecutable nativo y permite reconstruirlo desde fuentes.',[
 ('Preparación','En Linux x86-64, contar con g++, ar, libpng y una biblioteca SDL2 compatible. No depende de Python ni de Pygame para ejecutar.'),
 ('Compilación principal','Desde la carpeta raíz: ./COMPILAR_LINUX.sh. Se genera motor/libgritnova.a y el ejecutable CONTACTO_2010_LINUX.'),
 ('Comprobación del motor','Ejecutar ./motor/PROBAR_MOTOR.sh para validar funciones de teclado, PNG, dibujo y audio fuera de la clase del juego.'),
 ('Inicio de partida','Ejecutar ./JUGAR_CONTACTO_2010.sh y seleccionar EMPEZAR. Guardados de preferencias y récord se crean localmente.')])

cards_page('Pruebas y evidencias','33 / Control de calidad','La versión preparada superó 28 verificaciones automatizadas antes de esta corrección de terreno.',[
 ('Campaña','Carga, oleadas y sistemas principales de las cinco fases; comprobaciones de combate contra los jefes.'),
 ('Sistemas','Mandos, escudo, avisos, sprites de la Firebird revisada, armas, música, efectos y transiciones.'),
 ('Validación geométrica','Se cotejan textura, extremos de scroll, BIN, JSON y corredor mínimo. La edición revisada exige cobertura opaca en ambos bordes de pantalla.'),
 ('Capturas auténticas','Todos los fotogramas de juego del dossier se obtienen desde el ejecutable; ningún mockup sustituye una prueba.')])

cards_page('Alcance de las verificaciones','34 / Control de calidad','Las pruebas automáticas aportan confianza técnica sin sustituir una campaña jugada por una persona.',[
 ('Cobertura obtenida','Carga y coherencia de recursos, respuestas esperadas de mecánicas, fases, controles, jefes y música.'),
 ('A comprobar en equipo','Sensación del combate completo, tamaño de lectura del HUD en monitor, fluidez en oleadas densas y respuesta del mando físico.'),
 ('Nuevo borde corregido','El alfa del primer píxel bajo el HUD y de la última fila de la fase 1 supera el umbral de cobertura en sus 1920 columnas.'),
 ('Publicación','El paquete está preparado para una revisión final local. No se afirma que exista ya una Release publicada en GitHub.')])

cards_page('Memoria de desarrollo I','35 / Evolución','Las decisiones de diseño se fueron consolidando desde los primeros escenarios hasta una campaña reconocible.',[
 ('Campaña completa','Se construyeron cinco escenarios, cada uno con fondo, peligro, combinaciones de enemigos y enfrentamiento propios.'),
 ('Más coherencia','Se abandonaron enemigos redundantes, se incorporaron sierras reutilizables y se ajustaron las formaciones para evitar saturación.'),
 ('Narrativa','Los briefings y transmisiones se revisaron con un guion final autorizado por el creador y se corrigió su visibilidad.'),
 ('Música y finalización','La banda sonora recibió normalización, asignación correcta por escena y comprobaciones de reproducción.')])

cards_page('Memoria de desarrollo II','36 / Evolución','La revisión V1.1 concentra el trabajo en estabilidad, calidad visual y separación técnica.',[
 ('HUD militar','El panel metálico sustituyó al marcador antiguo. Se centraron ventanas, números, vidas, armas y energía de los jefes.'),
 ('Peligros legibles','Triángulos de alerta para hielo, retículas sobre blancos vulnerables y escudo independiente de la nave.'),
 ('Nave revisada','Firebird-01 de 16 fotogramas con sprite nuevo, disparos alineados y hitbox arcade centrada en el fuselaje.'),
 ('Cementerio V2','Nuevo techo y suelo, costura horizontal reparada y borde inferior cubierto sin reescalar ni alterar los demás niveles.')])

cards_page('Lo que puede ser GRIT NOVA','37 / Hoja de ruta','Proyecciones honestas: posibilidades técnicas, no funciones ya entregadas.',[
 ('Arquitectura de escenas','Extraer la máquina de estados a una interfaz intercambiable para que varios juegos aprovechen el mismo sistema.'),
 ('Datos declarativos','Definir enemigos, patrones, zonas de daño y guiones en archivos editables sin recompilar la campaña.'),
 ('Física y colisiones','Generalizar actores y grupos de impacto con espacios, terrenos y proyectiles reutilizables.'),
 ('Herramientas y portabilidad','Crear comprobadores gráficos, empaquetadores de assets y probar compilaciones para otros sistemas con dependencias adecuadas.')])

cards_page('Créditos y licencias','38 / Créditos','El proyecto identifica por separado el código, la música, los originales gráficos y el material editorial.',[
 ('Creación y dirección','Tony C. Ware · concepto, historia, dirección artística, diseño, producción e identidad visual.'),
 ('Programación y desarrollo','GPT-6 · OpenAI, colaboración técnica con el autor.'),
 ('Banda sonora original','FutureDark, alias musical de Tony C. Ware; originales OGG conservados en el paquete.'),
 ('Condiciones','Código fuente bajo licencia MIT. La ilustración, la música y las marcas no quedan automáticamente amparadas por la licencia del código.')])

# Archivo fotográfico de cubiertas. A página completa, sin ponerlas en un recuadro.
divider('Galería de carátulas','39 / Archivo artístico',
        'Cuatro composiciones distintas, sin copias repetidas ni diseños que anuncien tecnologías ajenas a esta edición. Todas ocupan su página completa y conservan sus proporciones originales.')
full_image(R/'documentacion/caratulas/CONTACTO_2010_CARATULA_ALTERNATIVA_CLASICA.png','Carátula alternativa · composición clásica')
full_image(R/'documentacion/caratulas/CONTACTO_2010_CARATULA_ALTERNATIVA_METALICA.webp','Carátula alternativa · título metálico')
full_image(R/'documentacion/caratulas/CONTACTO_2010_CARATULA_ALTERNATIVA_JUPITER.webp','Carátula alternativa · Júpiter')
# La portada HQ se incluyó ya a sangre como primera página; no se repite.

C.save()
print('Dossier:',OUT,'| páginas:',num)
