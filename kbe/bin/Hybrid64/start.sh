#!/bin/sh
export KBE_ROOT='/root/kbengine/'
export KBE_RES_PATH='/root/kbengine/kbe/res/;/root/kbengine/demo/;/root/kbengine/demo/res/'
export KBE_HYBRID_PATH='/root/kbengine/kbe/bin/Hybrid64/'

./kbmachine&
sleep 1s
./dbmgr&
sleep 1s
./baseappmgr&
sleep 1s
./cellappmgr&
sleep 1s
./loginapp&
sleep 1s
./baseapp&
sleep 1s
./cellapp&


