# Awk script to generate protobuf documentation.  Awk works by defining functions,
# then running each line of text through a procedural script.  This script works by
# keeping track of the depth based on { and }, and doing string substitutions on
# enum and message names in order to match the protobuf naming style for depth.
# Other miscellaneous protobuf-specific styles are converted to C++ as well, such as
# lines ending in semicolons that need to end in commas, etc.
#
# The intent is not to create perfect C++, but to create good enough C++ that an
# automated tool such as Doxygen can read the comments and documentation from the
# proto file.
#
# usage:
#    awk -f genPbDox.awk ../SDK/simData/GeneratedCode/simData.proto > simData.pb.dox

# Prints a doxygen header like:
# /**@<str>
#  * <prevLine>
#  */
# If prevLine is blank, then the middle line is not printed.
function printHdr(str)
{
	print "/**@" str
	if( prevLine != "") print " * " prevLine
	prevLine = ""

	print " */"
}

# Handles proper naming for nested enumerations and messages so that the returned
# name matches the pattern that Google Protobuf uses for the naming scheme.
function addDepth(start)
{
	# Append the depth name at each layer
	for(i = 0; i < depth-1; ++i)
		start = start dName[i] "_"

	return start
}

BEGIN {
	# track nesting depth
	depth = 0
	inEnum = 0
}

# Whenever we encounter an open brace, increase our depth count
/{/ {
	depth++
}

# Whenever we encounter a close brace, decrease our depth count
/}/ {
	depth--
}

# convert pb "package" command to namespace
/^package / {
	name = $2
	# Remove trailing semicolon from the name
	sub(/;$/, "", name)
	print "namespace " name " {"
	# Skip processing for the rest of this line and restart the script for next line
	next
}

# emit Doxygen class update info for messages
/^message /, depth==0&&/}/ {
	if($1 == "message") {
		dName[depth-1] = $2

		# Print out the long-doc style looking like:
		#    /**@class Depth1_Depth2_<messagename>
		# ... based on the name of the message, and depth prefixes
		printf "/**@class "
		for(i = 0; i < depth-1; ++i)
			printf "%s_", dName[i]
		print $2

		# Print out the documentation strings if there are any, then clear them and close out comment
		if( prevLine != "") print " * " prevLine
		prevLine = ""
		print " */"

	} else if( $1 == "enum" ) {
		# Capture the name of the enumeration
		eName = $2
		# Calculate the C++ enum name based on depth
		name = addDepth("enum ") eName

		# Print out documentation for the header
		printHdr(name)

		printf "%s", name
		for(i = 3; i <= NF; ++i)
			printf " %s", $i
		printf "\n"

		# Remember that we're inside an enum for future processing
		inEnum = 1
	} else if( inEnum ) {
		# Are we exiting the enum?
		if( match($0, /}/) )
		{
			# Print a newline, flag exit, and continue processing next line anew
			inEnum = 0
			print
			next
		}

		# get the indentation from all
		ws = $0
		# strip the non-whitespace to eol
		sub(/[^ ]+.*/, "", ws)

		# assumes "val = num;" - $1 is val, $2 is =, $3 is $num
		# enum values have the namespace prependend; match funky protobuf naming
		enumFqn = addDepth("") eName "_" $1

		# Replace the trailing semicolon from "val = num;" with a comma, C++ style
		sep = $3
		sub(/;/, ",", sep)

		# Prints out whitespace, the enum name, the =, and the value with appended comma
		printf "%s%s %s %s", ws, enumFqn, $2, sep

		# print anything else
		for(i = 4; i <= NF; ++i)
			printf " %s", $i

		# Finish the line off
		printf "\n"

	} else if ($1 == "required" || $1 == "optional" || $1 == "repeated" ) {
                # If there is a comment stored, it goes to this field
                # Clear it so it's not incorrectly printed with a message or enum that has no comment
                prevLine = ""
        }else if( $0 ~ /^ *\/\/\//) {
		# Capture comments into prevLine
		prevLine = $0
		sub(/ *\/\/\/ */, "", prevLine)
	}

	# Skip processing for the rest of this line and restart the script for next line
	next
}

/^enum /, depth==0&&/}/ {
	# Print documentation for enums
	if( $1 == "enum") {
		if( prevLine != "" ) printHdr("enum " $2)
	}

	fullLine = $0
	# Replace semicolons with commas to convert proto syntax to C++ syntax
	sub(/;/, ",", fullLine)
	# ... except the previous line replaces "};" with "}," -- switch it back
	sub(/},/, "};", fullLine)
	# Print out the line
	print fullLine

	# Skip processing for the rest of this line and restart the script for next line
	next
}

# Matches lines that start with ///
/^ *\/\/\// {
	# Save the comment (without the ///) as prevLine, for use in printHdr and other documentation lines
	prevLine = $0
	sub(/ *\/\/\/ */, "", prevLine)
	# Skip processing for the rest of this line and restart the script for next line
	next
}

# If we're here, there's no changes required, just print the line
{
	print $0
}

# At the end of the file, print a closing brace for the package namespace
END {
	print "}"
}

