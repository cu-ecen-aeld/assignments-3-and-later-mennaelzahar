#!/bin/bash


if [ $# -eq 0 ]; then
	echo "A path to the file and the text are needed."
	exit 1
fi

writefile=$1
writestr=$2

if [ -z "$writefile=" ]; then
	echo "Failed to find the path to the file name."
	exit 1
elif [ -z "$writestr" ]; then
	echo "Failed to find the text to be written."
	exit 1
fi

# Extract the directory from the file path
dirpath=$(dirname "$writefile")


mkdir -p $dirpath
touch $writefile
echo "$writestr" >  $writefile

if [[ ! -f "$writefile" ]]; then
    echo "Error: Could not create the file."
    exit 1
fi


echo "File $writefile is successfully created."
