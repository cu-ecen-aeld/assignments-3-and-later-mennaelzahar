#!/bin/bash


if [ $# -eq 0 ]; then
	echo "A path to the dir and the keywork are needed."
	exit 1
fi

filesdir=$1
searchstr=$2

if [ -z "$filesdir" ]; then
	echo "Failed to find the path to the directory."
	exit 1
elif [ -z "$searchstr" ]; then
	echo "Failed to find the search keyword."
	exit 1
elif [ ! -d "$filesdir" ]; then
	echo "$filesdir is not a dir."
	exit 1
fi

X=0
Y=0
for file in $(find $filesdir -type f); do
	((X++))
	count=$(grep -c "$searchstr" $file)
	((Y=Y+count))
done	



echo "The number of files are $X and the number of matching lines are $Y."



