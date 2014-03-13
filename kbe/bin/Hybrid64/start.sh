#!/bin/sh

echo KBE_ROOT = \${KBE_ROOT}\"
echo KBE_ROOT = \${KBE_RES_PATH}\"
echo KBE_ROOT = \${KBE_HYBRID_PATH}\"

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

