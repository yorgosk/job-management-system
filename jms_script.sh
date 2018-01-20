#!/bin/bash

pathflag=0
comflag=0
nflag=0

if [ $# -eq 0 ]		# if no arguments were supplied
  	then
    	echo "ERROR! No arguments supplied!"
    	exit -1
elif [ $# -eq 4 ]	# if 4 arguments were supplied
	then
		if [ $1 = "-l" ]	# if the 1st argument is the path-flag
			then 
				path=$2
				pathflag=1
		elif [ $1 = "-c" ]	# else if the 1st argument is the command-flag
			then
				com=$2
				comflag=1
		fi
		if [ $3 = "-c" ]	# if the 3rd argument is the command-flag
			then
				com=$4
				comflag=1
		elif [ $3 = "-l" ]	# else if the 3rd argument is the path-flag
			then
				path=$4
				pathflag=1
		fi
elif [ $# -eq 5 ]	# if 5 arguments were supplied
	then
		if [ $1 = "-l" ]	# if the 1st argument is the path-flag
			then 
				path=$2
				pathflag=1
		elif [ $1 = "-c" ]	# else if the 1st argument is the command-flag
			then
				com=$2
				comflag=1
				if [ $com = "size" ]	# if the command is "size"...
					then
						if [ $3 != "-l" ]	# ...we expect a number to follow it
							then
								n=$3
								nflag=1
						fi
				fi
		fi
		if (( nflag == 1 ))
			then
				if [ $4 = "-l" ]		# if the 4th argument is the path-flag
					then
						path=$5
						pathflag=1
				fi
		else
			if [ $3 = "-c" ]			# else if the 3rd argument is the command-flag
				then
					com=$4
					comflag=1
					if [ $com = "size" ]	# if the commang is "size"...
						then
							n=$5			# ...we expect a number to follow it
							nflag=1
					fi
		elif [ $3 = "-l" ]				# else if the 3rd argument is the path-flag
			then
				path=$4
				pathflag=1
		fi
		fi
else
	echo "ERROR! Invalid number of arguments!"
fi

if (( pathflag == 0 )) || (( comflag == 0 ))	# if there is a flag missing
	then
		echo "ERROR! Bad patamerers!"
		exit -1
fi

if [ $com = "list" ]							# if the command was "list"...
	then
		ls -l $path
elif [ $com = "purge" ]							# else if the command was "purge"...
	then
		rm -rf $path 							# purge the contents of the "path"
elif [ $com = "size" ]							# else if the command was "size"...
	then
		if (( nflag == 1 ))						# ...if a number-limit was specified
			then
			du -a --max-depth=1 $path | sort -n | tail -n$n 	# print with the specified limit applied
		else
			du -a --max-depth=1 $path | sort -n 	# print with no limit applied
		fi
else
	echo "ERROR! Invalid: com = $com!"
	exit -1
fi