#!/usr/bin/env python3
"""Auditoría reproducible de la textura, costura y hitbox de la fase 1 V1.1."""
from pathlib import Path
import struct,json
import numpy as np
from PIL import Image
root=Path(__file__).resolve().parents[2]
asset=root/'assets/fase1_cementerio_v2_1920x540.png'
arr=np.asarray(Image.open(asset).convert('RGBA'))
assert arr.shape==(540,1920,4)
assert np.array_equal(arr[:,0],arr[:,-1]),'extremos distintos'
for y in (125,135,507,526,539):
    assert arr[y,0,3]>=200,('ranura visible en la unión',y,arr[y,0,3])
assert arr[270:390,:,3].max()==0,'alfa del corredor central no limpio'
assert np.min(arr[-1,:,3])>=230,'franja transparente en el borde inferior'
assert np.min(arr[120,:,3])>=200,'franja transparente tras HUD en el techo'
assert not (root/'assets/desguace_mapa_loop.png').exists(),'terreno viejo sigue activo'
raw=(root/'assets/terreno.bin').read_bytes()
assert struct.unpack_from('<I',raw,0)[0]==1920 and len(raw)==4+1920*4
geometry=np.frombuffer(raw[4:],dtype='<u2').reshape((-1,2)).astype(int)
meta=json.loads((root/'desarrollo/documentacion/fase1_cementerio_v2_geometria.json').read_text())
assert np.array_equal(geometry[:,0],meta['top'])
assert np.array_equal(geometry[:,1],meta['floor'])
assert np.all(geometry[:,1]-geometry[:,0]>=240)
assert np.max(geometry[:,0])<=230 and np.min(geometry[:,1])>=415
assert np.array_equal(geometry[0],geometry[-1])
print('CEMENTERIO V2: loop sin ranura, transparencia real, 1920×540, BIN/JSON concordantes')
print('Corredor mínimo:',min(geometry[:,1]-geometry[:,0]), 'techo máximo:',max(geometry[:,0]), 'suelo mínimo:',min(geometry[:,1]))
