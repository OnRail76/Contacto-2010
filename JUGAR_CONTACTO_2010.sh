#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"
export LD_LIBRARY_PATH="$ROOT/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
if [[ ! -x "$ROOT/CONTACTO_2010_LINUX" ]]; then
  echo 'ERROR: No se encuentra el binario CONTACTO_2010_LINUX.' >&2;exit 1
fi
exec "$ROOT/CONTACTO_2010_LINUX" "$@"
