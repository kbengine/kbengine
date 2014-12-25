#!/bin/sh

export KBE_ROOT="../"
export KBE_RES_PATH="$KBE_ROOT/kbe/res/:$KBE_ROOT/demo:$KBE_ROOT/demo/res/:$KBE_ROOT/demo/scripts/"
export KBE_BIN_PATH="$KBE_ROOT/kbe/bin/server/"

echo KBE_ROOT = \"${KBE_ROOT}\"
echo KBE_RES_PATH = \"${KBE_RES_PATH}\"
echo KBE_BIN_PATH = \"${KBE_BIN_PATH}\"

sh ./kill_server.sh

$KBE_BIN_PATH/kbmachine --cid=2129652375332859700 --grouporder=1  --globalorder=1&
$KBE_BIN_PATH/messagelog --cid=1129653375331859700 --grouporder=1 --globalorder=2&
$KBE_BIN_PATH/billingsystem --cid=1129652375332859700 --grouporder=1 --globalorder=3&
$KBE_BIN_PATH/dbmgr --cid=3129652375332859700 --grouporder=1 --globalorder=4&
$KBE_BIN_PATH/baseappmgr --cid=4129652375332859700 --grouporder=1  --globalorder=5&
$KBE_BIN_PATH/cellappmgr --cid=5129652375332859700 --grouporder=1  --globalorder=6&
$KBE_BIN_PATH/baseapp --cid=6129652375332859700 --grouporder=1  --globalorder=7&
$KBE_BIN_PATH/cellapp --cid=7129652375332859700 --grouporder=1  --globalorder=8&
$KBE_BIN_PATH/loginapp --cid=8129652375332859700 --grouporder=1  --globalorder=9&

