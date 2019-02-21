#!/bin/sh

currPath=$(pwd)
keyStr="/kbengine"

bcontain=`echo $currPath|grep $keyStr|wc -l`


if [ $bcontain = 0 ]
then
	export KBE_ROOT="$(pwd)"
else
	export KBE_ROOT="$(pwd | awk -F "/kbengine" '{print $1}')/kbengine"
fi

export KBE_RES_PATH="$KBE_ROOT/kbe/res"
export KBE_BIN_PATH="$KBE_ROOT/kbe/bin/server"

echo KBE_ROOT = \"${KBE_ROOT}\"
echo KBE_RES_PATH = \"${KBE_RES_PATH}\"
echo KBE_BIN_PATH = \"${KBE_BIN_PATH}\"

"$KBE_BIN_PATH/kbcmd" --newassets=python --outpath="$currPath/server_assets"
