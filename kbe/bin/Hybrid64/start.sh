#!/bin/sh

./kbmachine&
sleep 1s
./dbmgr&
sleep 2s
./baseappmgr&
sleep 1s
./cellappmgr&
sleep 1s
./loginapp&
sleep 1s
./baseapp&
sleep 1s
./cellapp&


