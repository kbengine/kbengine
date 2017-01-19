#!/usr/bin/perl -w
# Simple script that uses openssl and c_rehash to set up a ca to generate certs
# for test-mongoc-stream-tls

use strict;

use File::Copy;

my $subj = "/C=US/ST=NY/L=New York/O=MongoDB Inc./OU=C Driver/SAN=foo.com/CN=";

my ($ca_dir, $conf) = @ARGV;

my @dirs = (
   $ca_dir,
   "$ca_dir/build",
   "$ca_dir/verify",
   "$ca_dir/ca.db.certs",
   "$ca_dir/keys",
   "$ca_dir/crl",
);

system("rm", "-rf", "$ca_dir");

# Create the relevant directory structure and files for a CA
{
   for my $dir (@dirs) {
      mkdir $dir or die "Couldn't create dir: $!";
   }

   set_file("$ca_dir/ca.db.index", "");
   set_file("$ca_dir/ca.db.serial", "01\n");
   set_file("$ca_dir/ca.db.rand", "151\n");
}

# generate the root key and cert
{
   openssl ("genrsa", "-out", "$ca_dir/signing-ca.key", "1024");
   openssl ("req", "-new", "-x509", "-days", 365, "-key",
            "$ca_dir/signing-ca.key", "-out", "$ca_dir/signing-ca.crt",
            "-config", $conf, "-subj", "${subj}mongo_root");
   copy ("$ca_dir/signing-ca.crt", "$ca_dir/verify/mongo_root.pem");
}

# generate a simple password less cert
{
   openssl("req", "-nodes", "-newkey", "rsa:1024", req_args("mongodb.com"));
   openssl("ca", ca_args("mongodb.com"));
   dist_files("mongodb.com");
}

# generate a simple passworded cert
{
   openssl ("req", "-passout", "pass:testpass", "-newkey", "rsa:1024",
            req_args ("pass.mongodb.com"));
   openssl ("ca", ca_args ("pass.mongodb.com"));
   dist_files ("pass.mongodb.com");
}

# generate a cert and revoke it to test crls
{
   openssl ("req", "-nodes", "-newkey", "rsa:1024",
            req_args ("rev.mongodb.com"));
   openssl ("ca", ca_args ("rev.mongodb.com"));
   dist_files ("rev.mongodb.com");

   openssl ("ca", "-config", $conf, "-revoke",
            "$ca_dir/verify/rev.mongodb.com.pem");
   openssl ("ca", "-config", $conf, "-gencrl", "-out",
            "$ca_dir/crl/root.crl.pem");
}

# generate a cert with some alt names including a wild card
{
   openssl ("req", "-nodes", "-newkey", "rsa:1024",
            req_args ("alt.mongodb.com"));
   openssl ("ca", "-extensions", "v3_req", ca_args ("alt.mongodb.com"));
   dist_files ("alt.mongodb.com");
}

# generate a cert for localhost
{
   openssl ("req", "-nodes", "-newkey", "rsa:1024",
            req_args ("127.0.0.1"));
   openssl ("ca", ca_args ("127.0.0.1"));
   dist_files ("127.0.0.1");
}

# generate the hash directory structure ssl needs
system("c_rehash", "$ca_dir/verify") and die "failed: $?";

sub dist_files {
   my ($name) = @_;
   
   system("cat '$ca_dir/build/$name.key' '$ca_dir/build/$name.crt' > '$ca_dir/keys/$name.pem'") and die "terribly: $?";
   copy("$ca_dir/build/$name.crt", "$ca_dir/verify/$name.pem");
}

sub ca_args {
   my $name = $_[0];

   return
      "-batch", "-config", $conf, "-out", "$ca_dir/build/$name.crt", "-in",
      "$ca_dir/build/$name.req",
   ;
}

sub req_args {
   my $name = $_[0];

   return (
      "-keyout", "$ca_dir/build/$name.key", "-out", "$ca_dir/build/$name.req",
      "-config", $conf, "-subj", "$subj$name",
   );
}

sub openssl {
   my @args = @_;

   system("openssl", @args) and die "failed: $?"
}

sub set_file {
   my ($name, $contents) = @_;

   open my $file, "> $name" or die "Couldn't open $name: $!";
   print $file $contents;
   close $file;
}
