use strict;
use Test;

use Sigar ();

BEGIN {
    plan tests => 1;
}

my $sigar = new Sigar;

ok $sigar;
