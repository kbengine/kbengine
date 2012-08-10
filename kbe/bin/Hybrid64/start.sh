#!/bin/sh
export KBE_ROOT='/root/kbe/'
export KBE_RES_PATH='/root/kbe/kbe/res/;/root/kbe/demo/;/root/kbe/demo/res/'
export KBE_HYBRID_PATH='/root/kbe/kbe/bin/Hybrid64/'

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


