#! perl -w
#
# w32locatedb.pl -- Build apr-util with Berkeley DB on Win32
#
# Usage: perl w32locatedb.pl <type> <incdir> <libdir>
#    type:   Library type to link with ('lib' or 'dll')
#    incdir: BDB includes directory (for db.h)
#    libdir: Library directory (for libdbXY[s][d].lib)
#
# This script falls under the Apache License.
# See http://www.apache.org/docs/LICENSE

require 5.008;
use strict;
use File::Spec::Functions qw(canonpath rel2abs
                             splitpath catpath splitdir catdir);

########
# Subroutine prototypes
sub usage();
sub find_srcdir();
sub get_lib_name($$);
sub edit_header($$);
sub edit_project($$);

########
# Parse program arguments and set globals
die usage() unless scalar @ARGV >= 3;

my $type = lc($ARGV[0]);
die "Invalid library type '$type'\n"
    unless $type eq 'lib' or $type eq 'dll';

my $incdir = $ARGV[1];
die "No 'db.h' in $incdir\n" unless -f "$incdir/db.h";

my $libdir = $ARGV[2];
die "$libdir: $!" unless -d $libdir;

my $libname = get_lib_name($type, $incdir);
die "No '$libname.lib' in $libdir" unless -f "$libdir/$libname.lib";
die "No '${libname}d.lib' in $libdir" unless -f "$libdir/${libname}d.lib";

my $srcdir = find_srcdir();
my $apu_hw = canonpath("$srcdir/include/apu.hw");
my $apu_want_hw = canonpath("$srcdir/include/apu_want.hw");
my $apu_select_dbm_hw = canonpath("$srcdir/include/private/apu_select_dbm.hw");
my $aprutil_dsp = canonpath("$srcdir/aprutil.dsp");
my $libaprutil_dsp = canonpath("$srcdir/libaprutil.dsp");
die "Can't find $apu_hw" unless -f $apu_hw;
die "Can't find $apu_want_hw" unless -f $apu_want_hw;
die "Can't find $apu_select_dbm_hw" unless -f $apu_select_dbm_hw;
die "Can't find $aprutil_dsp" unless -f $aprutil_dsp;
die "Can't find $libaprutil_dsp" unless -f $libaprutil_dsp;


########
# Edit the header file templates
my $db_h = rel2abs(canonpath("$incdir/db.h"));
$db_h =~ s/\\/\//g;
edit_header($apu_hw,
            [['^\s*\#\s*define\s+APU_HAVE_DB\s+0\s*$',
              '#define APU_HAVE_DB     1']]);
edit_header($apu_want_hw,
            [['^\s*\#\s*include\s+\<db\.h\>\s*$',
              "#include \"$db_h\""]]);
edit_header($apu_select_dbm_hw,
            [['^\s*\#\s*define\s+APU_USE_DB\s+0\s*$',
              '#define APU_USE_DB      1'],
             ['^\s*\#\s*include\s+\<db\.h\>\s*$',
              "#include \"$db_h\""]]);

########
# Edit the .dsp files
my $libpath = rel2abs(canonpath("$libdir/$libname"));
edit_project($aprutil_dsp, $libpath);
edit_project($libaprutil_dsp, $libpath);


########
# Print usage
sub usage()
{
    return ("Usage: perl w32locatedb.pl <type> <incdir> <libdir>\n"
            . "    type:   Library type to link with ('lib' or 'dll')\n"
            . "    incdir: BDB includes directory (for db.h)\n"
            . "    libdir: Library directory (for libdbXY[s][d].lib)\n");
}

########
# Calculate the (possibly relative) path to the top of the apr-util
# source dir.
sub find_srcdir()
{
    my $srcdir = rel2abs(canonpath($0));
    my ($vol, $dir, $file) = splitpath($srcdir);
    my @dirs = splitdir($dir);
    die if scalar @dirs < 1;
    do { $_ = pop @dirs } while ($_ eq '');
    return catpath($vol, catdir(@dirs), '');
}

########
# Construct the name of the BDB library, based on the type and
# version information in db.h
sub get_lib_name($$)
{
    my ($type, $incdir) = @_;
    my $major = undef;
    my $minor = undef;
    my $patch = undef;

    open(DBH, "< $incdir/db.h")
        or die "Can't open $incdir/db.h: $!";
    while (<DBH>) {
        chomp;
        m/^\s*\#\s*define\s+DB_VERSION_(MAJOR|MINOR|PATCH)\s+(\d+)\s*$/;
        next unless defined $1 and defined $2;
        if    ($1 eq 'MAJOR') { $major = $2; }
        elsif ($1 eq 'MINOR') { $minor = $2; }
        elsif ($1 eq 'PATCH') { $patch = $2; }
        last if defined $major and defined $minor and defined $patch;
    }
    close(DBH);
    die "Can't determine BDB version\n"
        unless defined $major and defined $minor and defined $patch;

    print "Using BDB version $major.$minor.$patch\n";

    my $libname = "libdb$major$minor";
    $libname .= 's' if $type eq 'lib';
    return $libname;
}

########
# Replace a file, keeping a backup copy
sub maybe_rename_with_backup($$$)
{
    my ($tmpfile, $file, $maybe) = @_;
    if ($maybe) {
        # Make the file writable by the owner. On Windows, this removes
        # any read-only bits.
        chmod((stat($file))[2] | 0600, $file);
        rename($file, "${file}~");
        rename($tmpfile, $file);
    } else {
        print "No changes in $file\n";
        unlink($tmpfile);
    }
}

########
# Edit a header template in-place.
sub edit_header($$)
{
    my ($file, $pairs) = @_;
    my $tmpfile = "$file.tmp";
    my $substs = 0;

    open(IN, "< $file") or die "Can't open $file: $!";
    open(TMP, "> $tmpfile") or die "Can't open $tmpfile: $!";
    while (<IN>) {
        chomp;
        foreach my $pair (@$pairs) {
            $substs += s/${$pair}[0]/${$pair}[1]/;
        }
        print TMP $_, "\n";
    }
    close(IN);
    close(TMP);

    maybe_rename_with_backup($tmpfile, $file, $substs > 0);
}

########
# Edit a project file in-place
sub edit_project($$)
{
    my ($file, $libpath) = @_;
    my $tmpfile = "$file.tmp";
    my $substs = 0;
    my ($prog, $debug) = (undef, undef);

    my $libsearch = $libpath;
    $libsearch =~ s/\\/\\\\/g;

    open(IN, "< $file") or die "Can't open $file: $!";
    open(TMP, "> $tmpfile") or die "Can't open $tmpfile: $!";
    while (<IN>) {
        chomp;

        if (m/^\# TARGTYPE \"[^\"]+\" 0x([0-9A-Za-z]+)/
            and defined $1) {
            $prog = 'LINK32' if $1 eq '0102';
            $prog = 'LIB32' if $1 eq '0104';
            die "Unknown project type 0x$1" unless defined $prog;
        } elsif (defined $prog
                 and m/^\# PROP Use_Debug_Libraries ([01])/
                 and defined $1) {
            $debug = $1;
        } elsif (defined $prog and defined $debug
                 and m/^\# ADD $prog (\"$libsearch)?/
                 and not defined $1) {
            my $fullpath =
                ($debug eq '1' ? "${libpath}d.lib" : "$libpath.lib");
            $substs += s/^\# ADD $prog /\# ADD $prog \"$fullpath\" /;
        } elsif (m/^\# ADD CPP/) {
            $substs += s/APU_USE_SDBM/APU_USE_DB/g;
        }

        print TMP $_, "\n";
    }
    close(IN);
    close(TMP);

    maybe_rename_with_backup($tmpfile, $file, $substs > 0);
}
