
    INTRODUCTION

Caelum is an open-source sky rendering library for OGRE. For more information
see the big Caelum thread on the ogre forum:
    http://www.ogre3d.org/phpBB2/viewtopic.php?t=24961
The thread is very long; you should try reading it backwards.

There is also a page on the ogre wiki (might be out of date):
    http://www.ogre3d.org/wiki/index.php/Caelum

Caelum is distributed under the LGPL. What this basically means is that you
can do what you please but you should send any changes back. The forum is the
best place to do this.

    COMPILING

Compiling on windows is easiest. You need Visual Studio 2005 or 2008 and the
Ogre 1.4 SDK installed. Just open the project files and click build; it
should work.

A SConstruct file is provided for linux. Making it work required writing
ogre.cfg files and such and is harder than on windows. You should be able to
get it working if you have some experience building software for linux.

    USAGE

See CaelumDemo's source code; it should contain some minimal initialisation
and tweaking. CaelumLab is more complicated and can be used to visually tweak
different sky elements.

There is no Caelum "SDK"; the recommended way to use it is to build it
yourself.

    DOCUMENTATION

Caelum's code has plenty of comments in doxygen format. It is not known if
building doxygen documentation actually works; but you can at least read the
comments.

/* vim: set nocin noai tw=78 spell: */
