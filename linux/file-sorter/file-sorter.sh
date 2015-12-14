#!/bin/bash


SOURCE_DIR="."
DESTINATION_DIR="."
NAME=`basename $0`


declare -A ext_dir_map
declare -A files_count

function print_usage {
cat <<EOF
usage:
	$NAME [-s <directory>] [-d <directory>] [-v] (-r <rules>|-f <file>)
	$NAME -p
	$NAME -h

Sorts the files in the directory

Options:
	-s, --source        source directory [default: current]
	-d, --destination   destination directory [default: current]
	-r, --rules         set rules
	-f, --file          get rules from the file	
	-p, --print         print the current rules
	-v, --verbose       verbose mode
	-h, --help          show this screen

Rules format:
	"directory1:ext1,ext2,...;directory2:ext3,ext4,..."

EOF
}

function print_rules {
cat <<EOF
	Current rules:
	
	"$RULES"

EOF
}

function parse_rules {
	rules=`echo "$1" | tr -d "\n\t " | tr ";" "\n"`	
	for rule in ${rules[@]}; do			
		directory=`echo "$rule" | cut -d ":" -f1`
		extensions=(`echo "$rule" | cut -d ":" -f2`)		
		OIFS=$IFS
		IFS=','		
		for extension in ${extensions[@]}; do			
			ext_dir_map["$extension"]="$directory"								
		done
		IFS=$OIFS 		
	done

	if [ $? -ne 0 ]; then
		echo "rules parsing error"
		exit 1
	fi
}

function sort {
	parse_rules "$RULES"
	
	total=0
	fails=0

	files=`find $SOURCE_DIR -type f`
	OIFS=$IFS
	IFS=$'\n'			
	for file in ${files[@]}; do		
		basename=`basename "$file"`			
		extension="${basename##*.}"
		directory=${ext_dir_map["$extension"]}		
		if [ -z "$directory" ]; then
			directory="unknown"
		fi

		if [ ! -d "$DESTINATION_DIR/$directory" ]; then
			
			mkdir "$DESTINATION_DIR/$directory"
			if [[ $? -ne 0 && -n "$verbose" ]]; then
				echo "directory $DESTINATION_DIR/$directory has been created"
			fi
		fi
		
		cp "$file" "$DESTINATION_DIR/$directory/$basename"
		if [ $? -eq 0 ]; then
			if [ -n "$verbose" ]; then				
				echo "file $file has been moved to $DESTINATION_DIR/$directory"
			fi
			files_count["$directory"]=`expr ${files_count["$directory"]} + 1`
			total=`expr $total + 1`			
		else
			fails=`expr $fails + 1`
		fi
	done
	IFS=$OIFS	
	
	printf "Result:\n"	
	for file in "${!files_count[@]}"; do	
		printf "%-15s %10d\n" "$file" "${files_count["$file"]}"
	done
	printf "%-15s %10d\n" "total" "$total"
	printf "%-15s %10d\n" "fails" "$fails"	
}

function rules_from_file {		
	RULES=`cat "$1"`	
}

while true ; do
    case "$1" in
        -s|--source)
            SOURCE_DIR="$2" ;  shift 2 ;;        
        -d|--destination)
            DESTINATION_DIR="$2" ; shift 2 ;;
        -r|--rulse)
            RULES="$2" ; sort ; exit 0 ;;
        -f|--file)
            rules_from_file "$2" ; sort ; exit 0 ;;
        -p|--print)
            print_rules ; exit 0 ;;
        -v|--verbose)
            verbose=1 ; shift ;;
        -h|--help)
            print_usage ; exit 0 ;;
        *)
            print_usage ; exit 1 ;;
    esac
done