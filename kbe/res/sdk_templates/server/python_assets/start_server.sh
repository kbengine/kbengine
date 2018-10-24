#!/bin/sh

currPath=$(pwd)
keyStr="/kbengine/"

bcontain=`echo $currPath|grep $keyStr|wc -l`


if [ $bcontain = 0 ]
then
	export KBE_ROOT=$(cd ../; pwd)
else
	export KBE_ROOT="$(pwd | awk -F "/kbengine/" '{print $1}')/kbengine"
fi



export KBE_RES_PATH="$KBE_ROOT/kbe/res/:$(pwd):$(pwd)/res:$(pwd)/scripts/"
export KBE_BIN_PATH="$KBE_ROOT/kbe/bin/server/"

echo KBE_ROOT = \"${KBE_ROOT}\"
echo KBE_RES_PATH = \"${KBE_RES_PATH}\"
echo KBE_BIN_PATH = \"${KBE_BIN_PATH}\"

sh ./kill_server.sh

$KBE_BIN_PATH/machine --cid=2129652375332859700 --gus=1&
$KBE_BIN_PATH/logger --cid=1129653375331859700 --gus=2&
$KBE_BIN_PATH/interfaces --cid=1129652375332859700 --gus=3&
$KBE_BIN_PATH/dbmgr --cid=3129652375332859700 --gus=4&
$KBE_BIN_PATH/baseappmgr --cid=4129652375332859700 --gus=5&
$KBE_BIN_PATH/cellappmgr --cid=5129652375332859700 --gus=6&
$KBE_BIN_PATH/baseapp --cid=6129652375332859700 --gus=7&
$KBE_BIN_PATH/cellapp --cid=7129652375332859700 --gus=8&
$KBE_BIN_PATH/loginapp --cid=8129652375332859700 --gus=9&
