#!/usr/bin/perl

my $DEBUG = 0;

my $F_FIRST_ARG = "J_Object";
my $HEADER_TEMPLATE = "native.tmpl";

my %TYPES = (
			 "J_Object"  => "Ljava/lang/Object;",
			 "J_String"  => "Ljava/lang/String;",
			 "J_Void"    => "V",
			 "J_Int"     => "I",
			 "J_Float"   => "F",
			 "J_Byte"    => "B",
			 "J_Char"    => "C",
			 "J_Short"   => "S",
			 "J_Boolean" => "Z",
			 );

##
# 
#
my $DECLARATIONS = "";
my $FARRAY = "";

##
#  Templator
#
sub templator {
  my ($filename,$fillings) = @_;
  my $text;
  local $/;
  local *F;
  open( F, "< $filename" ) || return "File $filename was not found\n";
  $text = <F>;
  close(F);
  $text =~ s{ %% (.*?) %% }
  { exists( $fillings->{$1} )
		? $fillings->{$1}
	: ""
	  }gsex;
  return $text;
}


##
#  Output header
#
sub produce_header(){
  my ($file) = @_;
  %fields = ( FUNCS_DEFS => $DECLARATIONS,
			  FUNCS_ARRAY => $FARRAY );
  print &templator($file,\%fields);
}
	
##
#  Parse function name and product:
#  java_lang_Object_valueOf --> java/lang/Object.valueOf
#
sub parse_function(){
  my ($func) = @_;
  my (@elements) = split(/_/,$func);
  my $real_name = @elements[$#elements];
  my $ret = "";
  foreach $i (@elements){
	goto EX if($i eq $real_name);
	$ret .= $i . "/";
  }
 EX:
  $ret =~ s/\/$/\.$real_name/g;
  return $ret;
}


##
#  Parse C file and try to find function definitions
#
sub parse_file(){
  my ($file) = @_;
  open F, "<$file" or die("> can't open file $file: $!");
 M1:
  while(<F>){
	if(/^([A-Za-z_0-9]+)[\ \t]+?([A-Za-z_0-9]+)\((.*)\)\{?/){	  
	  my $fname = $2;
	  my $ret_type = $1;
	  my $orig_args = $3;
	  
	  print "function: $fname <$3> return type is $ret_type \n" if $DEBUG;
	  @args = split(",",$orig_args);	# get arguments array
	  
	  # check for first argument
	  if(!(@args[0]=~/^$F_FIRST_ARG[\ \t]+?.*/)){
		print "> Wrong first argument in function '$fname'. Must have of $F_FIRST_ARG type.\n";
		next M1;
	  }

	  # add to declarations 
	  $DECLARATIONS .= "$ret_type $fname($orig_args); \n";

	  # Try to get arguments signature
	  my $args_signature = "";
	  my $f = 1;
	M2:
	  foreach $i (@args){
		my $modif = "";	
		if($f){ $f = 0; next M2; };	# skip first argument (must be J_Object)

		$i =~ s/\*/\ \*\ /g;        # separate all asterisks by spaces
		$i =~ s/\[+?.*?\]/\ \*\ /g; # replace all '[]' on '*'
		$i =~ s/^\ +?//g;           # remove leading spaces
 		$i =~ s/\ {1,}/\ /g;        # if occured more than 1 spaces replace it on one (used for splitting)
		
		# get array of arguments items: 'J_Int *var' -> { 'J_Int', '*', 'var' }
		my (@a) = split(/\ /,$i); 
		for($j=1;$j<@a;$j++){
		  if($a[$j] eq "*"){ $modif .= '['; } # count [] modificators
		}
		# make arguments signature
		$args_signature .= $modif . $TYPES{$a[0]};
	  }
	  
	  # get signature
	  $args_signature = "($args_signature)" . $TYPES{$ret_type};
	  
	  # parse function
	  my $fsign = &parse_function($fname);
	  
	  print "Function signature: $fsign$args_signature\n" if $DEBUG;
	  $FARRAY .= "\t{0,\"$fsign$args_signature\",(J_NativeFunction)$fname},\n";
	}
  }
  close(F);
}


sub list_dir_files(){
  my ($src) = @_;
  my @header = ();

  opendir(DIR,$src) || die("can't opendir $src: $!");
  @listdir = readdir(DIR);
  foreach $i (@listdir){
	if($i eq "." || $i eq ".." || -d $src."/".$i){ next; }
	if($i=~/.*\.c$/){
	  print "> processing $i ...\n" if $DEBUG;
	  my $t = &parse_file("$src/$i");
	  push(@header,$t) if($t!="");
	  print "> done\n" if $DEBUG;
	}
  }
  close(DIR);
  return @header;
}


&list_dir_files(@ARGV[0]);
&produce_header("@ARGV[0]/$HEADER_TEMPLATE");
