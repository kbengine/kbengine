use IO::File;
use File::Find;

if ($ARGV[0] eq '-6') {
    find(\&tovc6, '.');
}
elsif ($ARGV[0] eq '-5') {
    find(\&tovc5, '.');
}
elsif ($ARGV[0] eq '-2005') {
    find(\&tovc2005, '.');
}
elsif ($ARGV[0] eq '-w3') {
    find(\&tow3, '.');
}
elsif ($ARGV[0] eq '-w4') {
    find(\&tow4, '.');
}
elsif ($ARGV[0] eq '-ia64') {
    find(\&tovc64, '.');
}
elsif ($ARGV[0] eq '-d') {
    find(\&todebugpools, '.');
}
elsif ($ARGV[0] eq '-b') {
    find(\&tobrowse, '.');
}
elsif ($ARGV[0] eq '-mt') {
    find(\&addmt, '.');
}
elsif ($ARGV[0] eq '-m') {
        ## 0 - conapp, 1 - dll lib, 2 - static lib
        $dsptype = 2;
        $name = "apr";
        onemake();
}
else {
    print "Specify -5 or -6 for Visual Studio 5 or 6 (98) .dsp format\n";
    print "Specify -w3 or -w4 for .dsp build with warning level 3 or 4 (strict)\n\n";
    print "Specify -ia64 for build targeted at Itanium (req's psdk tools)\n\n";
    print "Specify -p for extreme pool debugging\n\n";
    print "Specify -mt to add .manifest embedding\n\n";
    die "Missing argument";
}

sub addmt { 
    my $outpath, $outtype;

    if (m|\.dsp$|) {
        $oname = $_;
        $tname = '.#' . $_;
        $verchg = 0;
        $srcfl = new IO::File $oname, "r" || die;
        $dstfl = new IO::File $tname, "w" || die;
        while ($src = <$srcfl>) {
            if ($src =~ m|^# TARGTYPE .+ Application|) {
                $outtype = ".exe"
            }
            if ($src =~ m|^# TARGTYPE .+ Dynamic-Link|) {
                $outtype = ".dll"
            }
            if ($src =~ m|^# PROP Output_Dir "(.+)"|) {
                $outdir = $1;
                $outpath = $oname;
                $outpath =~ s|\.dsp||;
                $outpath =  ".\\" . $outdir . "\\" . $outpath . $outtype;
            }
            if ($src =~ m|^# ADD (BASE )?LINK32 .+ /out:"([^"]+)"|) {
                $outpath = $2;
                $outpath =~ s|/|\\|;
                $outpath = ".\\" . $outpath if (!($outpath =~ m|^\.|));
                $src =~ s|/out:"([^"]+)"|/out:"$outpath"|;
            }
            if (defined($outpath) && ($src =~ m|^# Begin Special Build Tool|)) {
                undef $outpath;
            }
            if (defined($outpath) && defined($outtype) && ($src =~ m|^\s*$|)) {
                print $dstfl '# Begin Special Build Tool' . "\n";
                print $dstfl 'TargetPath=' . $outpath . "\n";
                print $dstfl 'SOURCE="$(InputPath)"' . "\n";
                print $dstfl 'PostBuild_Desc=Embed .manifest' . "\n";
                print $dstfl 'PostBuild_Cmds=if exist $(TargetPath).manifest mt.exe -manifest $(TargetPath).manifest -outputresource:$(TargetPath);2' . "\n";
                print $dstfl '# End Special Build Tool' . "\n";
                $verchg = -1;
                undef $outpath;
            }
            print $dstfl $src;
        }
        undef $outtype if (defined($outtype));
        undef $outpath if (defined($outpath));
        undef $srcfl;
        undef $dstfl;
        if ($verchg) {
            unlink $oname || die;
            rename $tname, $oname || die;
            print "Added manifest to " . $oname . " in " . $File::Find::dir . "\n"; 
        }
        else {
            unlink $tname;
        }
    }
}
sub tovc5 { 

    if (m|\.dsp$|) {
        $oname = $_;
        $tname = '.#' . $_;
        $verchg = 0;
        $srcfl = new IO::File $oname, "r" || die;
        $dstfl = new IO::File $tname, "w" || die;
        while ($src = <$srcfl>) {
            if ($src =~ s|Format Version 6\.00|Format Version 5\.00|) {
                $verchg = -1;
            }
            if ($src =~ s|^(# ADD CPP .*)/ZI (.*)|$1/Zi $2|) {
                $verchg = -1;
            }
            if ($src =~ s|^(# ADD BASE CPP .*)/ZI (.*)|$1/Zi $2|) {
                $verchg = -1;
            }
            if ($src =~ s|^(# ADD CPP .*)/EHsc (.*)|$1/GX $2|) {
                $verchg = -1;
            }
            if ($src =~ s|^(# ADD BASE CPP .*)/EHsc (.*)|$1/GX $2|) {
                $verchg = -1;
            }
            while ($src =~ s|^(# ADD RSC .*)/d "([^ ="]+)=([^"]+)"|$1/d $2="$3"|) {
                $verchg = -1;
            }
            if ($src !~ m|^# PROP AllowPerConfigDependencies|) {
                print $dstfl $src; }
            else {
                $verchg = -1;
            }
        }
        undef $srcfl;
        undef $dstfl;
        if ($verchg) {
            unlink $oname || die;
            rename $tname, $oname || die;
            print "Converted VC6 project " . $oname . " to VC5 in " . $File::Find::dir . "\n"; 
        }
        else {
            unlink $tname;
        }
    }
}

sub tovc6 { 

    if (m|\.dsp$|) {
        $oname = $_;
        $tname = '.#' . $_;
        $verchg = 0;
        $srcfl = new IO::File $_, "r" || die;
        $dstfl = new IO::File $tname, "w" || die;
        while ($src = <$srcfl>) {
            if ($src =~ s|Format Version 5\.00|Format Version 6\.00|) {
                $verchg = -1;
            }
            if ($src =~ s|^(!MESSAGE .*)\\\n|$1|) {
                $cont = <$srcfl>;
                $src = $src . $cont;
                $verchg = -1;
            }
            if ($src =~ s|^(# ADD CPP .*)/GX (.*)|$1/EHsc $2|) {
                $verchg = -1;
            }
            if ($src =~ s|^(# ADD BASE CPP .*)/GX (.*)|$1/EHsc $2|) {
                $verchg = -1;
            }
            while ($src =~ s|^(# ADD RSC .*)/d "([^ ="]+)=([^"]+)"|$1/d $2="$3"|) {
                $verchg = -1;
            }
            print $dstfl $src; 
            if ($verchg && $src =~ m|^# Begin Project|) {
                print $dstfl "# PROP AllowPerConfigDependencies 0\n"; 
            }
        }
        undef $srcfl;
        undef $dstfl;
        if ($verchg) {
            unlink $oname || die;
            rename $tname, $oname || die;
            print "Converted VC5 project " . $oname . " to VC6 in " . $File::Find::dir . "\n"; 
        }
        else {
            unlink $tname;
        }
    }
}

sub tovc2005 { 

    if (m|\.dsp$| || m|\.mak$|) {
        $oname = $_;
        $tname = '.#' . $_;
        $verchg = 0;
        $srcfl = new IO::File $_, "r" || die;
        $dstfl = new IO::File $tname, "w" || die;
        while ($src = <$srcfl>) {
            if ($src =~ s|(\bCPP.*) /GX(.*)|$1 /EHsc$2|) {
                $verchg = -1;
            }
            if ($src =~ s|(\bLINK32.*) /machine:I386(.*)|$1$2|) {
                $verchg = -1;
            }
            while ($src =~ s|^(# ADD RSC .*)/d ([^ ="]+)="([^"]+)"|$1/d "$2=$3"|) {
                $verchg = -1;
            }
            print $dstfl $src; 
        }
        undef $srcfl;
        undef $dstfl;
        if ($verchg) {
            unlink $oname || die;
            rename $tname, $oname || die;
            print "Converted project " . $oname . " to 2005 in " . $File::Find::dir . "\n"; 
        }
        else {
            unlink $tname;
        }
    }
}

sub tow3 { 

    if (m|\.dsp$| || m|\.mak$|) {
        $oname = $_;
        $tname = '.#' . $_;
        $verchg = 0;
        $srcfl = new IO::File $_, "r" || die;
        $dstfl = new IO::File $tname, "w" || die;
        while ($src = <$srcfl>) {
            while ($src =~ m|\\\n$|) {
                $src = $src . <$srcfl>
            }
            if ($src =~ s|(\bCPP.*) /W4(.*)|$1 /W3$2|) {
                $verchg = -1;
            }
            print $dstfl $src; 
        }
        undef $srcfl;
        undef $dstfl;
        if ($verchg) {
            unlink $oname || die;
            rename $tname, $oname || die;
            print "Converted project " . $oname . " to warn:3 in " . $File::Find::dir . "\n"; 
        }
        else {
            unlink $tname;
        }
    }
}

sub tow4 { 

    if (m|\.dsp$| || m|\.mak$|) {
        $oname = $_;
        $tname = '.#' . $_;
        $verchg = 0;
        $srcfl = new IO::File $_, "r" || die;
        $dstfl = new IO::File $tname, "w" || die;
        while ($src = <$srcfl>) {
            while ($src =~ m|\\\n$|) {
                $src = $src . <$srcfl>
            }
            if ($src =~ s|(\bCPP.*) /W3(.*)|$1 /W4$2|) {
                $verchg = -1;
            }
            print $dstfl $src; 
        }
        undef $srcfl;
        undef $dstfl;
        if ($verchg) {
            unlink $oname || die;
            rename $tname, $oname || die;
            print "Converted project " . $oname . " to warn:4 " . $File::Find::dir . "\n"; 
        }
        else {
            unlink $tname;
        }
    }
}

sub tovc64 { 

    if (m|\.dsp$| || m|\.mak$|) {
        $oname = $_;
        $tname = '.#' . $_;
        $verchg = 0;
        $srcfl = new IO::File $_, "r" || die;
        $dstfl = new IO::File $tname, "w" || die;
        while ($src = <$srcfl>) {
            while ($src =~ m|\\\n$|) {
                $src = $src . <$srcfl>
            }
            if ($src =~ s|Win32 \(x86\) (Release)|Win32 (IA64) $1|s) {
                $verchg = -1;
            }
            if ($src =~ s|Win32 \(x86\) (Debug)|Win32 (IA64) $1|s) {
                $verchg = -1;
            }
            if ($src =~ s| - Win32 (Release)| - Win32 (IA64) $1|s) {
                $verchg = -1;
            }
            if ($src =~ s| - Win32 (Debug)| - Win32 (IA64) $1|s) {
                $verchg = -1;
            }
            # Cross compilation exceptions
            if (!(m|gen[^/]*$| || m|dftables[^/]*$|)) {
                if ($src =~ s|(\bCPP.* /W3)(.*) /FD(.*)|$1 /As64 /Wp64$2$3|s) {
                    $verchg = -1;
                }
                if ($src =~ s|(\bLINK.*/machine):I386(.*)|$1:IA64$2|s) {
                    $verchg = -1;
                }
            }
            else {
                if ($src =~ s|(\bCPP.* /W3)(.*) /FD(.*)|$1 /As32 /Wp64$2$3|s) {
                    $verchg = -1;
                }
            }
            print $dstfl $src; 
        }
        undef $srcfl;
        undef $dstfl;
        if ($verchg) {
            unlink $oname || die;
            rename $tname, $oname || die;
            print "Converted build file " . $oname . " to Win64 in " . $File::Find::dir . "\n"; 
        }
        else {
            unlink $tname;
        }
    }
}

sub todebugpools { 

    if (m|\.dsp$|) {
        $oname = $_;
        $tname = '.#' . $_;
        $verchg = 0;
        $srcfl = new IO::File $oname, "r" || die;
        $dstfl = new IO::File $tname, "w" || die;
        while ($src = <$srcfl>) {
            if ($src =~ s|^(# ADD CPP .* /D "_DEBUG" )|$1/D "APR_POOL_DEBUG" |) {
                $verchg = -1;
                if ($oname =~ /apr\.dsp$/) {
                    $src =~ s|^(# ADD CPP .* /D "_DEBUG" )|$1/D "POOL_DEBUG" |;
                }
            }
            print $dstfl $src; 
        }
        undef $srcfl;
        undef $dstfl;
        if ($verchg) {
            unlink $oname || die;
            rename $tname, $oname || die;
            print "Converted project " . $oname . " to debug pools in " . $File::Find::dir . "\n"; 
        }
        else {
            unlink $tname;
        }
    }
}

sub tobrowsesources { 

    if (m|\.dsp$|) {
        $oname = $_;
        $tname = '.#' . $_;
        $verchg = 0;
        $srcfl = new IO::File $oname, "r" || die;
        $dstfl = new IO::File $tname, "w" || die;
        while ($src = <$srcfl>) {
            if ($src =~ s|^(# ADD CPP .*)( /Fd)|$1 /Fr "/httpd-2.0/srclib/apr"$2|) {
                $verchg = -1;
            }
            print $dstfl $src; 
        }
        undef $srcfl;
        undef $dstfl;
        if ($verchg) {
            unlink $oname || die;
            rename $tname, $oname || die;
            print "Converted project " . $oname . " to browse sources in " . $File::Find::dir . "\n"; 
        }
        else {
            unlink $tname;
        }
    }
}

sub frommakefiles {

    if (m|\.mak\.in$|) {
        $oname = $_;
        $dname = $_;
        $_ =~ s/\.mak\.in/.dsp/;
        $verchg = 0;
        $srcfl = new IO::File $oname, "r" || die;
        $dstfl = new IO::File $tname, "w" || die;
        while ($src = <$srcfl>) {
            if ($src =~ s|^(# ADD CPP .*)( /Fd)|$1 /Fr "/httpd-2.0/srclib/apr"$2|) {
                $verchg = -1;
            }
            print $dstfl $src; 
        }
        undef $srcfl;
        undef $dstfl;
        if ($verchg) {
            unlink $oname || die;
            rename $tname, $oname || die;
            print "Converted project " . $oname . " to browse sources in " . $File::Find::dir . "\n"; 
        }
        else {
            unlink $tname;
        }
    }
}


sub onemake {

    if ($dsptype == 0) {
        $cdefs = qq{/D "WIN32" /D "_CONSOLE"};
        $lmodel = qq{/subsystem:console};
        $targname = "Win32 (x86) Console Application";
        $targid = "0x0103";
        $debpath = "Debug"; $relpath = "Release";
    } elsif ($dsptype == 1) {
        $cdefs = qq{/D "WIN32" /D "_WINDOWS"};
        $lmodel = qq{/subsystem:windows /dll};
        $targname = "Win32 (x86) Dynamic-Link Library";
        $targid = "0x0102";
        $debpath = "Debug"; $relpath = "Release";
    } elsif($dsptype == 2) {
        $cdefs = qq{/D "WIN32" /D "_CONSOLE"};
        $lmodel = qq{/subsystem:console};
        $targname = "Win32 (x86) Static Library";
        $targid = "0x0104";
        $debpath = "LibD"; $relpath = "LibR";
    }
        $file = dspheader();


        $second = "";

        $model = "Release";
        $usedebuglib = "0";
        $debugdef = "NDEBUG";
        $cflags = "/MD /W3 /O2";
        $cincl = qq{/I "./include" /I "./os/win32" /I "./srclib/apr/include" /I "./srclib/apr-util/include"};
        $lflags = qq{/map};
        $file .= dsponemodel();

        $second = "ELSE";
        $model = "Debug";
        $usedebuglib = "1";
        $debugdef = "_DEBUG";
        $cflags = "/MDd /W3 /GX /Zi /Od";
        $cincl = qq{/I "./include" /I "./os/win32" /I "./srclib/apr/include" /I "./srclib/apr-util/include"};
        $lflags = qq{/incremental:no /debug};
        $file .= dsponemodel();

        $file .= qq{
!ENDIF 

# Begin Target

# Name "$name - Win32 Release"
# Name "$name - Win32 Debug"
};

        $toroot = ".";

#HERE IS OUR FOREACH!
        $file .= qq{# Begin Source File

SOURCE=./server/main.c
# End Source File
};

    if ($dsptype == 0) {
        #HERE IS OUR ICON!
        $icon="$toroot/build/win32/apache.ico";
        $file .= qq{# Begin Source File

SOURCE=$icon
# End Source File
};
        $icon = "icon=" . $icon . " ";
    }
    if ($dsptype == 0 || $dsptype == 1) {
        $file .= qq{
# Begin Source File

SOURCE=./$name.rc
# End Source File
# Begin Source File

SOURCE=$toroot/include/ap_release.h
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Creating Version Resource
InputPath=$toroot/include/ap_release.h $toroot/build/win32/win32ver.awk

"./$name.rc" : \$(SOURCE) "\$(INTDIR)" "\$(OUTDIR)"
        awk -f $toroot/build/win32/win32ver.awk $name "Apache HTTP Server" $toroot/include/ap_release.h $icon> ./Apache.rc

# End Custom Build
# End Source File
};
    }
        $file .= qq{
# End Target
# End Project
};
        print $file;
}

sub dspheader {
    if ($dsptype == 1) {
        $midl = "MTL=midl.exe\n";
    } else {
        $midl = ""
    }
qq{# Microsoft Developer Studio Project File - Name="$name" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "$targname" $targid

CFG=$name - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "$name.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "$name.mak" CFG="$name - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "$name - Win32 Release" (based on "$targname")
!MESSAGE "$name - Win32 Debug" (based on "$targname")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
${midl}RSC=rc.exe
};
}
sub dsponemodel {
    if ($model eq "Release") {
        $targpath = $relpath;
    } else {
        $targpath = $debpath;
    }
    if ($dsptype == 1) {
        $midl = 
qq{# ADD BASE MTL /nologo /D "$debugdef" /win32
# ADD MTL /nologo /D "$debugdef" /mktyplib203 /win32
};  }
    if ($dsptype == 2) {
        $linkop = qq{LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
};
    } else {
        $linkop = qq{LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib advapi32.lib ws2_32.lib mswsock.lib /nologo $lmodel $lflags /machine:I386
# ADD LINK32 kernel32.lib user32.lib advapi32.lib ws2_32.lib mswsock.lib /nologo $lmodel $lflags /machine:I386
};
    }        

qq{
!${second}IF  "\$(CFG)" == "$name - Win32 $model"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries $usedebuglib
# PROP BASE Output_Dir "$targpath"
# PROP BASE Intermediate_Dir "$targpath"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries $usedebuglib
# PROP Output_Dir "$targpath"
# PROP Intermediate_Dir "$targpath"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo $cflags $cincl /D "$debugdef" $cdefs /FD /c
# ADD CPP /nologo $cflags $cincl /D "$debugdef" $cdefs /Fd"$targpath/$name" /FD /c
${midl}# ADD BASE RSC /l 0x409 /d "$debugdef"
# ADD RSC /l 0x409 /d "$debugdef"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
$linkop};
}