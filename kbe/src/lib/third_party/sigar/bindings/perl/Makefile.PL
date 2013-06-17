use ExtUtils::MakeMaker;

use Config;
use lib qw(.. lib);
use SigarWrapper ();

my $installdir;
if ($ARGV[0]) {
    $installdir = $ARGV[0];
}
else {
    #check binary-dist location and svn source location
    for (qw(../../sigar-bin ../java/sigar-bin)) {
        if (-e $_) {
            $installdir = $_;
            last;
        }
    }
}

SigarWrapper::generate(Perl => '.');

my $archname = SigarWrapper::archname();
my $libname = 'sigar';
if ($archname) {
    $libname .= '-' . $archname;
}

my $ccname = $Config{ccname};

my $define = {
    gcc => '-Wall -Werror',
    cc_r => '-qhalt=w',
}->{$ccname} || '';

WriteMakefile(
    'NAME'		=> 'Sigar',
    'VERSION_FROM'	=> 'Sigar.pm',
    'LIBS'		=> ["-L$installdir/lib -l$libname"],
    'INC'		=> "-I$installdir/include",
    'DEFINE'            => $define,
    'depend'            => { 'Sigar.c' => 'Sigar_generated.xs' },
);
