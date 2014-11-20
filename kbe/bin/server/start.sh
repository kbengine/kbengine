#!/bin/sh

echo KBE_ROOT = \"${KBE_ROOT}\"
echo KBE_RES_PATH = \"${KBE_RES_PATH}\"
echo KBE_BIN_PATH = \"${KBE_BIN_PATH}\"

sh ./kill.sh

./kbmachine&
sleep 1s
./billingsystem&
sleep 1s
./messagelog&
sleep 1s
./dbmgr&
sleep 2s
./baseappmgr&
sleep 1s
./cellappmgr&
sleep 1s
./baseapp&
sleep 1s
./baseapp&
sleep 1s
./baseapp&
sleep 1s
./cellapp&
sleep 1s
./cellapp&
sleep 1s
./cellapp&
sleep 1s
./loginapp&

