#!/bin/sh

export KBE_ROOT="../"
export KBE_RES_PATH="$KBE_ROOT/kbe/res/:$KBE_ROOT/demo:$KBE_ROOT/demo/res/:$KBE_ROOT/demo/scripts/"
export KBE_BIN_PATH="$KBE_ROOT/kbe/bin/server/"

echo KBE_ROOT = \"${KBE_ROOT}\"
echo KBE_RES_PATH = \"${KBE_RES_PATH}\"
echo KBE_BIN_PATH = \"${KBE_BIN_PATH}\"

sh ./kill_server.sh

$KBE_BIN_PATH/machine&
sleep 1s
$KBE_BIN_PATH/interfaces&
sleep 1s
$KBE_BIN_PATH/messagelog&
sleep 1s
$KBE_BIN_PATH/dbmgr&
sleep 2s
$KBE_BIN_PATH/baseappmgr&
sleep 1s
$KBE_BIN_PATH/cellappmgr&
sleep 1s
$KBE_BIN_PATH/baseapp&
sleep 1s
$KBE_BIN_PATH/baseapp&
sleep 1s
$KBE_BIN_PATH/baseapp&
sleep 1s
$KBE_BIN_PATH/cellapp&
sleep 1s
$KBE_BIN_PATH/cellapp&
sleep 1s
$KBE_BIN_PATH/cellapp&
sleep 1s
$KBE_BIN_PATH/loginapp&

