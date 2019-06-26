#!/usr/bin/perl
#
#                             Copyright (c) 2014
#                           Lantiq Deutschland GmbH
#
# For licensing information, see the file 'LICENSE' in the root folder of
# this software module.
#
# updateIoctl.pl - update the sources according to the XML description.
#

use strict;
use warnings;

$#ARGV == 0 or die "
   Usage: updateIoctl.pl <top_src_dir>
";

my ($TOP_SRC_DIR) =  @ARGV;

sub replaceBlock
{
   # parse arguments
   my ($in_file, $marker_start, $marker_end, $new_data) = @_;

   # new data should store markers
   $new_data = "$marker_start\n$new_data$marker_end";

   # read file
   open INFILE, $in_file or die "Could not open file '$in_file'. $!";
   my $data = join("", <INFILE>);
   close INFILE;

   $data =~ m/\Q$marker_start\E/ or die "Could not find marker '$marker_start'";

   $data =~ m/\Q$marker_end\E/ or die "Could not find marker '$marker_end'";

   # exit if data already in place
   $data =~ m#\Q$new_data\E# and return 0;

   # replace block
   $data =~ s#\Q$marker_start\E.*\Q$marker_end\E#$new_data#ism;

   $data =~ m#\Q$new_data\E# or die "Data was not inserted between '$marker_start' and '$marker_end'";

   # write result back
   open OUTFILE, ">", $in_file or die "Could not open file '$in_file'. $!";
   print OUTFILE ($data);
   close OUTFILE;
}

sub findHandler
{
   my ($regexp, @search_files) = @_;

   # print "regexp: $regexp\n\n";
   # print "search_files: @search_files\n\n";
   # die;

   # allow spaces between '()*,'
   $regexp =~ s/([\(\)\*,]{1})/ \Q$1\E\ /sg;
   #remove double spaces
   $regexp =~ s/  / /sg;
   # replace spaces by regular expression
   $regexp =~ s/ /\\s\*/sg;
   # replace variable name by regular expression
   $regexp =~ s/variable/\\S\*/sg;
   # allow unsigned and singet arguments
   $regexp =~ s/IFX_uint32_t/IFX_[u]?int32_t/sg;

   foreach my $in_file (@search_files) {
      my $data;
      # read file
      open INFILE, $in_file or die "Could not open file. $!";
      {
         local $/;
         $data = join("", <INFILE>);
      }
      close INFILE;

      $data =~ m/$regexp/ism and return 1;
   }

   return 0;
}

sub applyXSL_list
{
   my ($xsl_file, $xml_file, $cmd_filter) = @_;

   open INFILE, "xsltproc $xsl_file $xml_file |" . ($cmd_filter || "")
   or die "Could not apply $xsl_file. $!";
   my @data = <INFILE>;
   close INFILE;

   return @data;
}

sub applyXSL
{
   my ($xsl_file, $xml_file, $cmd_filter) = @_;

   local $/;
   return join("", applyXSL_list ($xsl_file, $xml_file, $cmd_filter));
}

sub updateTypes
{
# *** WARNING *** The locale specified by the  environment  affects  sort
# order.  Set LC_ALL=C to get the traditional sort order that uses native
# byte values.

   replaceBlock (
      <$TOP_SRC_DIR/src/drv_tapi_ioctl_handlers.h>,
      "/* IOCTL_HANDLER_TYPES_START */",
      "/* IOCTL_HANDLER_TYPES_END */",
      applyXSL (
         <$TOP_SRC_DIR/xml/drv_tapi_io_handler_type.xsl>,
         <$TOP_SRC_DIR/xml/drv_tapi_io.xml>,
         "LC_ALL=C sort -u |"
         )
      );

   replaceBlock (
      <$TOP_SRC_DIR/src/drv_tapi_ioctl_handlers.h>,
      "/* IOCTL_QOS_HANDLER_TYPES_START */",
      "/* IOCTL_QOS_HANDLER_TYPES_END */",
      applyXSL (
         <$TOP_SRC_DIR/xml/drv_tapi_io_handler_type.xsl>,
         <$TOP_SRC_DIR/xml/drv_tapi_qos_io.xml>,
         "LC_ALL=C sort -u |"
         )
      );
}

sub validateHandlers
{
   my $err = 0;
   foreach my $xml_file (@_)
   {
      foreach my $declaration (
         applyXSL_list (
            <$TOP_SRC_DIR/xml/drv_tapi_io_handler_decl.xsl>,
            $xml_file
            )
         )
      {
         chomp $declaration;

         findHandler ($declaration,
            <$TOP_SRC_DIR/include/drv_tapi_ll_interface.h>,
            <$TOP_SRC_DIR/src/*.c> ) or
         print STDERR "Could not find handler '$declaration'\n" and
         $err++;
      }
   }

   die if $err > 0;
}

sub updateHandlers
{
   replaceBlock (
      <$TOP_SRC_DIR/src/drv_tapi_ioctl_handlers.h>,
      "/* IOCTL_HANDLERS_START */",
      "/* IOCTL_HANDLERS_END */",
      applyXSL (
         <$TOP_SRC_DIR/xml/drv_tapi_io_handler.xsl>,
         <$TOP_SRC_DIR/xml/drv_tapi_io.xml>
         )
      );

   replaceBlock (
      <$TOP_SRC_DIR/src/drv_tapi_ioctl_handlers.h>,
      "/* IOCTL_QOS_HANDLERS_START */",
      "/* IOCTL_QOS_HANDLERS_END */",
      applyXSL (
         <$TOP_SRC_DIR/xml/drv_tapi_io_handler.xsl>,
         <$TOP_SRC_DIR/xml/drv_tapi_qos_io.xml>
         )
      );
}

sub updateIndexes
{
   replaceBlock (
      <$TOP_SRC_DIR/include/drv_tapi_io_indexes.h>,
      "/* IOCTL_INDEXES_START */",
      "/* IOCTL_INDEXES_END */",
      applyXSL (
         <$TOP_SRC_DIR/xml/drv_tapi_io_index.xsl>,
         <$TOP_SRC_DIR/xml/drv_tapi_io.xml>
         )
      );

   replaceBlock (
      <$TOP_SRC_DIR/include/drv_tapi_qos_io_indexes.h>,
      "/* IOCTL_QOS_INDEXES_START */",
      "/* IOCTL_QOS_INDEXES_END */",
      applyXSL (
         <$TOP_SRC_DIR/xml/drv_tapi_io_index.xsl>,
         <$TOP_SRC_DIR/xml/drv_tapi_qos_io.xml>
         )
      );
}

sub updateLookup
{
   replaceBlock (
      <$TOP_SRC_DIR/src/drv_tapi_debug.c>,
      "/* IOCTL_LOOKUP_START */",
      "/* IOCTL_LOOKUP_END */",
      applyXSL (
         <$TOP_SRC_DIR/xml/drv_tapi_io_lookup.xsl>,
         <$TOP_SRC_DIR/xml/drv_tapi_io.xml>
         )
      );

   replaceBlock (
      <$TOP_SRC_DIR/src/drv_tapi_debug.c>,
      "/* IOCTL_QOS_LOOKUP_START */",
      "/* IOCTL_QOS_LOOKUP_END */",
      applyXSL (
         <$TOP_SRC_DIR/xml/drv_tapi_io_lookup.xsl>,
         <$TOP_SRC_DIR/xml/drv_tapi_qos_io.xml>
         )
      );
}

sub validateDefinition
{
   eval ("use XML::Checker::Parser");
   if ($@)
   {
      warn "Could not find XML checker trying 'xmllint'";

      system ("xmllint --version &> /dev/null") == 0 or
         die "Could not run 'xmllint'";

      foreach my $xml_file (@_)
      {
         system ("xmllint --noout --valid $xml_file") == 0 or
            die "Validation failed for file '$xml_file'"
      }
   }
   else
   {
      require XML::Checker::Parser;

		XML::Checker::Parser->import;

      $ENV{'SGML_SEARCH_PATH'} = ".:$TOP_SRC_DIR:$TOP_SRC_DIR/src:$TOP_SRC_DIR/xml";

      # Throws an exception (with die) when an error is encountered, this
      # will stop the parsing process.
      # Don't die if a warning or info message is encountered, just print a message.
      sub parser_fail {
         my $code = shift;
         die XML::Checker::error_string ($code, @_) if $code < 300;
         XML::Checker::print_error ($code, @_);
      }

      foreach my $xml_file (@_)
      {
         my $parser = new XML::Checker::Parser ();

         eval {
            local $XML::Checker::FAIL = \&parser_fail;
            $parser->parsefile($xml_file);
				undef $XML::Checker::FAIL;
         };
         die $@ if $@;
      }
   }
}

print ("Validate ioctl definition...\n");
validateDefinition (
      <$TOP_SRC_DIR/xml/drv_tapi_io.xml>,
      <$TOP_SRC_DIR/xml/drv_tapi_qos_io.xml>
   );

print "Validate IOCTL handlers...\n";
validateHandlers(
      <$TOP_SRC_DIR/xml/drv_tapi_io.xml>,
      <$TOP_SRC_DIR/xml/drv_tapi_qos_io.xml>
   );

print "Updating IOCTL handlers...\n";
updateHandlers();

print ("Updating IOCTL handler types...\n");
updateTypes();

print ("Updating IOCTL lookup table...\n");
updateLookup();

print ("Updating IOCTL indexes...\n");
updateIndexes();

print ("IOCTL updating done.\n");

exit 0;
