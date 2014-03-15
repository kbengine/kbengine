#
# fixwin32mak.pl ::: Apache/Win32 maintanace program
#
# This program, launched from the build/ directory, replaces all nasty absoulute paths
# in the win32 .mak files with the appropriate relative root.
#
# Run this program prior to committing or packaging any newly exported make files.

use Cwd;
use IO::File;
use File::Find;

$root = cwd;
# ignore our own direcory (allowing us to move into any parallel tree)
$root =~ s|^.:(.*)?$|cd "$1|;
$root =~ s|/|\\\\|g;
$altroot = $root;
$altroot =~ s| ".:| "|;
print "Stripping " . $root . " and " . $altroot . "\n";
find(\&fixcwd, '.');

# Given this pattern that disregarded the RECURSE flag...
#
# !IF "$(RECURSE)" == "0" 
# 
# ALL : "$(OUTDIR)\mod_charset_lite.so"
# 
# !ELSE 
# 
# ALL : "libhttpd - Win32 Release" "libaprutil - Win32 Release" "libapr - Win32 Release" "$(OUTDIR)\mod_charset_lite.so"
# 
# !ENDIF 
#...
# DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep
#...
# ALL : $(DS_POSTBUILD_DEP)
#
# $(DS_POSTBUILD_DEP) : "libhttpd - Win32 Release" "libaprutil - Win32 Release" "libapr - Win32 Release" "$(OUTDIR)\mod_charset_lite.so"
#
# we will axe the final ALL : clause,
# strip all but the final element from $(DS_POSTBUILD_DEP) : clause
# move the DS_POSTBUILD_DEP assignment above the IF (for true ALL : targets)
# and in pass 2, append the $(DS_POSTBUILD_DEP) to the valid ALL : targets


sub fixcwd { 
    if (m|.mak$|) {
        $thisroot = $File::Find::dir;
        $thisroot =~ s|^./(.*)$|$1|;
        $thisroot =~ s|/|\\\\|g;
        $thisroot = $root . "\\\\" . $thisroot;
        $thisaltroot = $altroot . "\\\\" . $thisroot;
        $oname = $_;
        $tname = '.#' . $_;
        $verchg = 0;
        $postdep = 0;
        $srcfl = new IO::File $_, "r" || die;
        $dstfl = new IO::File $tname, "w" || die;
        while ($src = <$srcfl>) {
            if ($src =~ m|^DS_POSTBUILD_DEP=.+$|) {
                $postdepval = $src;
            }
            if ($src =~ s|^ALL : \$\(DS_POSTBUILD_DEP\)||) {
                $postdep = -1;
                $verchg = -1;
                $src = <$srcfl>;
                $src = <$srcfl> if ($src =~ m|^$|);
            }
            if ($postdep) {
                $src =~ s|^(\$\(DS_POSTBUILD_DEP\)) :.+(\"[^\"]+\")$|"$1" : $2|;
            }
            if ($src =~ m|^\s*($root[^\"]*)\".*$|) {
                $orig = $thisroot;
            } elsif ($src =~ m|^\s*($altroot[^\"]*)\".*$|) {
                $orig = $thisaltroot;
            }
            if (defined($orig)) {
                $repl = "cd \".";
                while (!($src =~ s|$orig|$repl|)) {
                   if (!($orig =~ s|^(.*)\\\\[^\\]+$|$1|)) {
                       break;
                   }
                   $repl .= "\\..";
                }
                print "Replaced " . $orig . " with " . $repl . "\n";
                $verchg = -1;
                undef $orig;
            }
            # With modern LINK.EXE linkers, there is a different LINK for
            # each platform, and it's determined by the file path.  Best
            # that here, after we compiled the code to the default CPU,
            # that we also link here to the default CPU.  Omitting the
            # /machine spec from the .dsp was not enough, MSVC put it back.
            #
            if ($src =~ s#^(LINK32_FLAGS=.*) /machine:(x|IX|I3)86 #$1 #) {
                $verchg = -1;
            }
            print $dstfl $src; 
        }
        undef $srcfl;
        undef $dstfl;
        if ($verchg) {
            if ($postdep) {
                $srcfl = new IO::File $tname, "r" || die;
                $dstfl = new IO::File $oname, "w" || die;
                while ($src = <$srcfl>) {
                    if ($src =~ m|^INTDIR=|) {
                        print $dstfl $src;
                        $src = $postdepval;
                    }
                    $src =~ s|^(ALL : .+)$|$1 "\$\(DS_POSTBUILD_DEP\)"|;
                    print $dstfl $src;
                }
                undef $srcfl;
                undef $dstfl;
                unlink $tname || die;
                print "Corrected post-dependency within " . $oname . " in " . $File::Find::dir . "\n"; 
            }
            else {
                unlink $oname || die;
                rename $tname, $oname || die;
                print "Corrected absolute paths within " . $oname . " in " . $File::Find::dir . "\n"; 
            }
        }
        else {
            unlink $tname;
        }
        $dname = $oname;
        $dname =~ s/.mak$/.dsp/;
        @dstat = stat($dname);
        @ostat = stat($oname);    
        if ($ostat[9] && $dstat[9] && ($ostat[9] != $dstat[9])) {
            @onames = ($oname);
            utime $dstat[9], $dstat[9], @onames;
            print "Touched datestamp for " . $oname . " in " . $File::Find::dir . "\n"; 
        }
        $oname =~ s/.mak$/.dep/;
        $verchg = 0;
        $srcfl = new IO::File $oname, "r" || die;
        $dstfl = new IO::File $tname, "w" || die;
        while ($src = <$srcfl>) {
            if (($src =~ m/^\t"(\.\.\\)+(apr|apr-util|apr-iconv)\\.*"\\/) || 
                ($src =~ m/^\t{\$\(INCLUDE\)}".*"\\/)) {
                $verchg = -1;
            }
            else {
                print $dstfl $src;
            }
        }
        undef $srcfl;
        undef $dstfl;
        if ($verchg) {
            unlink $oname || die;
            rename $tname, $oname || die;
            print "Stripped external dependencies from " . $oname . " in " . $File::Find::dir . "\n"; 
        }
        else {
            unlink $tname || die;
        }
        @ostat = stat($oname);    
        if ($ostat[9] && $dstat[9] && ($ostat[9] != $dstat[9])) {
            @onames = ($oname);
            utime $dstat[9], $dstat[9], @onames;
            print "Touched datestamp for " . $oname . " in " . $File::Find::dir . "\n"; 
        }
    }
}
