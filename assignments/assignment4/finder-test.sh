#!/bin/sh
# Tester script for assignment 1 and assignment 2
# Author: Siddhant Jajoo

set -e
set -u

NUMFILES=10
WRITESTR=AELD_IS_FUN
WRITEDIR=/tmp/aeld-data
CONF_DIR="/etc/finder-app/conf"

echo "DEBUG: CONF_DIR is set to: $CONF_DIR"
echo "DEBUG: Checking for username.txt at: ${CONF_DIR}/username.txt"
ls -l "${CONF_DIR}/username.txt" || echo "File not found!"

username=$(cat "${CONF_DIR}/username.txt")

if [ $# -lt 3 ]
then
    echo "Using default value ${WRITESTR} for string to write"
    if [ $# -lt 1 ]
    then
        echo "Using default value ${NUMFILES} for number of files to write"
    else
        NUMFILES=$1
    fi    
else
    NUMFILES=$1
    WRITESTR=$2
    WRITEDIR=/tmp/aeld-data/$3
fi

MATCHSTR="The number of files are ${NUMFILES} and the number of matching lines are ${NUMFILES}"

echo "Writing ${NUMFILES} files containing string ${WRITESTR} to ${WRITEDIR}"

rm -rf "${WRITEDIR}"

# create $WRITEDIR if not assignment1
assignment=$(cat "${CONF_DIR}/assignment.txt")

if [ "$assignment" != 'assignment1' ]
then
    mkdir -p "$WRITEDIR"

    if [ -d "$WRITEDIR" ]
    then
        echo "$WRITEDIR created"
    else
        exit 1
    fi
fi

# Use system-installed writer binary
for i in $(seq 1 $NUMFILES)
do
    writer "$WRITEDIR/${username}$i.txt" "$WRITESTR"
done

OUTPUTSTRING=$(finder "$WRITEDIR" "$WRITESTR")

# Write results to output file
echo "$OUTPUTSTRING" > /tmp/assignment4-result.txt

# remove temporary directories
rm -rf /tmp/aeld-data

set +e
echo "${OUTPUTSTRING}" | grep "${MATCHSTR}"
if [ $? -eq 0 ]; then
    echo "success" >> /tmp/assignment4-result.txt
    exit 0
else
    echo "failed: expected ${MATCHSTR} in ${OUTPUTSTRING} but instead found" >> /tmp/assignment4-result.txt
    exit 1
fi