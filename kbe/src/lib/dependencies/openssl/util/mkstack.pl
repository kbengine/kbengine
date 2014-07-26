#!/usr/local/bin/perl -w

# This is a utility that searches out "DECLARE_STACK_OF()"
# declarations in .h and .c files, and updates/creates/replaces
# the corresponding macro declarations in crypto/stack/safestack.h.
# As it's not generally possible to have macros that generate macros,
# we need to control this from the "outside", here in this script.
#
# Geoff Thorpe, June, 2000 (with massive Perl-hacking
#                           help from Steve Robb)

my $safestack = "crypto/stack/safestack";

my $do_write;
while (@ARGV) {
	my $arg = $ARGV[0];
	if($arg eq "-write") {
		$do_write = 1;
	}
	shift @ARGV;
}


@source = (<crypto/*.[ch]>, <crypto/*/*.[ch]>, <ssl/*.[ch]>);
foreach $file (@source) {
	next if -l $file;

	# Open the .c/.h file for reading
	open(IN, "< $file") || die "Can't open $file for reading: $!";

	while(<IN>) {
		if (/^DECLARE_STACK_OF\(([^)]+)\)/) {
			push @stacklst, $1;
		} if (/^DECLARE_ASN1_SET_OF\(([^)]+)\)/) {
			push @asn1setlst, $1;
		} if (/^DECLARE_PKCS12_STACK_OF\(([^)]+)\)/) {
			push @p12stklst, $1;
		}
	}
	close(IN);
}



my $old_stackfile = "";
my $new_stackfile = "";
my $inside_block = 0;
my $type_thing;

open(IN, "< $safestack.h") || die "Can't open input file: $!";
while(<IN>) {
	$old_stackfile .= $_;

	if (m|^/\* This block of defines is updated by util/mkstack.pl, please do not touch! \*/|) {
		$inside_block = 1;
	}
	if (m|^/\* End of util/mkstack.pl block, you may now edit :-\) \*/|) {
		$inside_block = 0;
	} elsif ($inside_block == 0) {
		$new_stackfile .= $_;
	}
	next if($inside_block != 1);
	$new_stackfile .= "/* This block of defines is updated by util/mkstack.pl, please do not touch! */";
		
	foreach $type_thing (sort @stacklst) {
		$new_stackfile .= <<EOF;

#define sk_${type_thing}_new(st) SKM_sk_new($type_thing, (st))
#define sk_${type_thing}_new_null() SKM_sk_new_null($type_thing)
#define sk_${type_thing}_free(st) SKM_sk_free($type_thing, (st))
#define sk_${type_thing}_num(st) SKM_sk_num($type_thing, (st))
#define sk_${type_thing}_value(st, i) SKM_sk_value($type_thing, (st), (i))
#define sk_${type_thing}_set(st, i, val) SKM_sk_set($type_thing, (st), (i), (val))
#define sk_${type_thing}_zero(st) SKM_sk_zero($type_thing, (st))
#define sk_${type_thing}_push(st, val) SKM_sk_push($type_thing, (st), (val))
#define sk_${type_thing}_unshift(st, val) SKM_sk_unshift($type_thing, (st), (val))
#define sk_${type_thing}_find(st, val) SKM_sk_find($type_thing, (st), (val))
#define sk_${type_thing}_find_ex(st, val) SKM_sk_find_ex($type_thing, (st), (val))
#define sk_${type_thing}_delete(st, i) SKM_sk_delete($type_thing, (st), (i))
#define sk_${type_thing}_delete_ptr(st, ptr) SKM_sk_delete_ptr($type_thing, (st), (ptr))
#define sk_${type_thing}_insert(st, val, i) SKM_sk_insert($type_thing, (st), (val), (i))
#define sk_${type_thing}_set_cmp_func(st, cmp) SKM_sk_set_cmp_func($type_thing, (st), (cmp))
#define sk_${type_thing}_dup(st) SKM_sk_dup($type_thing, st)
#define sk_${type_thing}_pop_free(st, free_func) SKM_sk_pop_free($type_thing, (st), (free_func))
#define sk_${type_thing}_shift(st) SKM_sk_shift($type_thing, (st))
#define sk_${type_thing}_pop(st) SKM_sk_pop($type_thing, (st))
#define sk_${type_thing}_sort(st) SKM_sk_sort($type_thing, (st))
#define sk_${type_thing}_is_sorted(st) SKM_sk_is_sorted($type_thing, (st))
EOF
	}
	foreach $type_thing (sort @asn1setlst) {
		$new_stackfile .= <<EOF;

#define d2i_ASN1_SET_OF_${type_thing}(st, pp, length, d2i_func, free_func, ex_tag, ex_class) \\
	SKM_ASN1_SET_OF_d2i($type_thing, (st), (pp), (length), (d2i_func), (free_func), (ex_tag), (ex_class)) 
#define i2d_ASN1_SET_OF_${type_thing}(st, pp, i2d_func, ex_tag, ex_class, is_set) \\
	SKM_ASN1_SET_OF_i2d($type_thing, (st), (pp), (i2d_func), (ex_tag), (ex_class), (is_set))
#define ASN1_seq_pack_${type_thing}(st, i2d_func, buf, len) \\
	SKM_ASN1_seq_pack($type_thing, (st), (i2d_func), (buf), (len))
#define ASN1_seq_unpack_${type_thing}(buf, len, d2i_func, free_func) \\
	SKM_ASN1_seq_unpack($type_thing, (buf), (len), (d2i_func), (free_func))
EOF
	}
	foreach $type_thing (sort @p12stklst) {
		$new_stackfile .= <<EOF;

#define PKCS12_decrypt_d2i_${type_thing}(algor, d2i_func, free_func, pass, passlen, oct, seq) \\
	SKM_PKCS12_decrypt_d2i($type_thing, (algor), (d2i_func), (free_func), (pass), (passlen), (oct), (seq))
EOF
	}
	$new_stackfile .= "/* End of util/mkstack.pl block, you may now edit :-) */\n";
	$inside_block = 2;
}


if ($new_stackfile eq $old_stackfile) {
	print "No changes to $safestack.h.\n";
	exit 0; # avoid unnecessary rebuild
}

if ($do_write) {
	print "Writing new $safestack.h.\n";
	open OUT, ">$safestack.h" || die "Can't open output file";
	print OUT $new_stackfile;
	close OUT;
}
