#!/usr/bin/env python3
"""Integra los dos paneles ORIGINALES 1920x320 en el corredor 1920x540 de la Fase 1.
No redibuja la obra: redimensiona verticalmente, recoloca y calcula colisión conservadora.
"""
from pathlib import Path
import json,struct,sys,shutil
import numpy as np
from PIL import Image
from scipy.ndimage import median_filter

root=Path(__file__).resolve().parents[2]
master=root/'desarrollo/material_original/arte/FASE1_CEMENTERIO_V2'
assets=root/'assets'
TOP_H=150
BOTTOM_H=150
# Ajuste de pantalla: techo 3 px arriba, suelo 9 px abajo.
# El último píxel visible del suelo queda cubierto por arte opaco; sin banda inferior.
TOP_Y=109
BOTTOM_Y=399
SCREEN_H=540
TEX_W=1920

def resize(file):
    src=Image.open(master/file).convert('RGBA')
    assert src.size==(1920,320),str(src.size)
    return src.resize((TEX_W,TOP_H),Image.Resampling.LANCZOS)

top=resize('TECHO_LOOP_1920x320_RGBA.png')
floor=resize('SUELO_LOOP_1920x320_RGBA.png')
canvas=Image.new('RGBA',(TEX_W,SCREEN_H),(0,0,0,0))
canvas.alpha_composite(top,(0,TOP_Y))
canvas.alpha_composite(floor,(0,BOTTOM_Y))
# La fuente trae extremos pixel-a-pixel idénticos, pero TRANSPARENTES:
# produce una ranura negra visible (aprox. 24 px) cuando se repite el suelo.
# Reparar solo 16 px de cada extremo con interpolación periódica de los
# dos bordes interiores. Se conserva intacto el diseño fuera de los dos bordes de 16 px.
a=np.asarray(canvas).copy()
# Extensión de los grabados metálicos próximos a cada borde (no colores planos).
BRIDGE=16
original=a.copy()
for x in range(BRIDGE):
    a[:,x]=original[:,2*BRIDGE-1-x]
    a[:,TEX_W-1-x]=original[:,TEX_W-2*BRIDGE+x]
# Solo la costura mínima exacta que cruzan la unión se fusionan suavemente, con
# muestras del propio arte, evitando la franja plana de un promedio ancho.
SMOOTH=5
right_edge=a[:,TEX_W-SMOOTH-1].astype(np.float32)
left_edge=a[:,SMOOTH].astype(np.float32)
for i in range(2*SMOOTH):
    x=(TEX_W-SMOOTH+i)%TEX_W
    t=(i+1)/(2*SMOOTH+1)
    t=t*t*(3-2*t)
    a[:,x]=np.rint(right_edge*(1-t)+left_edge*t).clip(0,255).astype(np.uint8)
a[:,-1]=a[:,0]
canvas=Image.fromarray(a,'RGBA')
texture=assets/'fase1_cementerio_v2_1920x540.png'
canvas.save(texture,optimize=True)

# Silueta alpha: la máscara de los sólidos grandes es una aproximación arcade
# que ignora pelillos/cables transparentes y concede 5 px al jugador.
alpha=a[:,:,3]
def ceiling(x):
    y=np.where(alpha[120:330,x]>110)[0]
    return int(120+y[-1]) if len(y) else 120

def ground(x):
    y=np.where(alpha[330:540,x]>110)[0]
    return int(330+y[0]) if len(y) else 510

top_raw=np.array([ceiling(x) for x in range(TEX_W)],dtype=np.int32)
floor_raw=np.array([ground(x) for x in range(TEX_W)],dtype=np.int32)
# mediana periódica para evitar que filamentos minúsculos creen picos de colisión.
periodic=lambda v: median_filter(np.r_[v[-15:],v,v[:15]],size=27)[15:-15]
top_collision=np.clip(periodic(top_raw)-7,120,230).astype(np.uint16)
floor_collision=np.clip(periodic(floor_raw)+8,415,520).astype(np.uint16)
assert np.min(floor_collision.astype(int)-top_collision.astype(int))>=180
# En la unión ambos extremos deben coincidir para geometría y gráfico.
top_collision[-1]=top_collision[0];floor_collision[-1]=floor_collision[0]
with (assets/'terreno.bin').open('wb') as out:
    out.write(struct.pack('<I',TEX_W))
    for t,f in zip(top_collision,floor_collision):out.write(struct.pack('<HH',int(t),int(f)))
info={'titulo':'Cementerio de escombros V2','imagen_activa':'assets/'+texture.name,
      'ancho':TEX_W,'alto':SCREEN_H,'techo_origen_y':TOP_Y,'techo_altura':TOP_H,
      'suelo_origen_y':BOTTOM_Y,'suelo_altura':BOTTOM_H,
      'referencia_colision':'alpha de arte RGBA suavizada, generosa para el jugador',
      'ancho_corredor_minimo':int(np.min(floor_collision.astype(int)-top_collision.astype(int))),
      'techo_maximo':int(top_collision.max()),'suelo_minimo':int(floor_collision.min()),
      'top':top_collision.astype(int).tolist(),'floor':floor_collision.astype(int).tolist()}
metadata=root/'desarrollo/documentacion/fase1_cementerio_v2_geometria.json'
metadata.write_text(json.dumps(info,ensure_ascii=False,indent=1)+'\n',encoding='utf-8')
assert np.array_equal(a[:,0],a[:,-1])
assert np.all(alpha[270:390,:]==0), 'El corredor central debe quedar limpio'
assert np.min(floor_collision-top_collision)>180
if (assets/'desguace_mapa_loop.png').exists():
    (assets/'desguace_mapa_loop.png').unlink()   # borrar terreno activo antiguo
print('CEMENTERIO V2: 1920x540, loop 1:1; corredor mínimo',info['ancho_corredor_minimo'],
      'techo máximo',info['techo_maximo'],'suelo mínimo',info['suelo_minimo'])
