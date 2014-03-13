#!/bin/sh

echo KBE_ROOT = \${KBE_ROOT}\"
echo KBE_ROOT = \${KBE_RES_PATH}\"
echo KBE_ROOT = \${KBE_HYBRID_PATH}\"

sleep 1s
./bots&
sleep 1s
./bots&
sleep 1s
./bots&
