#!/bin/sh

echo KBE_ROOT = \"${KBE_ROOT}\"
echo KBE_RES_PATH = \"${KBE_RES_PATH}\"
echo KBE_BIN_PATH = \"${KBE_BIN_PATH}\"

sleep 1s
./bots&
sleep 1s
./bots&
sleep 1s
./bots&
