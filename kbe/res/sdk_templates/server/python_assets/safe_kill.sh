#!/bin/sh

currPath=$(pwd)
keyStr="/kbengine/"

bcontain=`echo $currPath|grep $keyStr|wc -l`


if [ $bcontain = 0 ]
then
	export KBE_ROOT="$(cd ../; pwd)"
else
	export KBE_ROOT="$(pwd | awk -F "/kbengine/" '{print $1}')/kbengine"
fi



export KBE_RES_PATH="$KBE_ROOT/kbe/res/:$(pwd):$(pwd)/res:$(pwd)/scripts/"
export KBE_BIN_PATH="$KBE_ROOT/kbe/bin/server/"

userid=`id -u $1 &>/dev/null`
XUID=$?

if [ $XUID = 0 ]
then
	KBCMD_PATH="$KBE_BIN_PATH/kbcmd"
	XUID=`"$KBCMD_PATH" --getuid`
	echo UID=$XUID
fi

python "$KBE_ROOT/kbe/tools/server/pycluster/cluster_controller.py" stop $XUID
