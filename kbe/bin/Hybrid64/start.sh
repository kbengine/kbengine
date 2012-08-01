#!/bin/sh

./kbmachine&
sleep 1s
./dbmgr&
sleep 1s
./baseappmgr&
./cellappmgr&
sleep 1s

