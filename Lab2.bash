#!/bin/bash

# counter of interruptions
counter=0
# file name
filename=''

# prints prompt for user. Mostly helpful in SIGINT handler
printPrompt(){
	echo 'Enter filename: '
}

# checks if file <filename> exists, doing until user do not write correct filename
getCorrectFileName(){
	while true
	do
		# prints prompt for user
		printPrompt
		# gets user input
		read filename
		# checks if file exists
		if [[ ! -f $filename ]]; then
			echo "File $filename not found"
			# null filename for SIGINT handler
			filename=''
		else
			# in <filename> now correct file name, exit from inf cycle
			break
		fi
	done
}

# SIGINT handler
sigintHandler(){
	# beautifier
	echo ''
	# increase interrupt counter
	(( counter++ ))
	# every odd interruption
	if [[ $((counter % 2)) = 1 ]]; then
		# prints all non-binary files in current directory
		find . -maxdepth 1 -type f -exec grep -Iq . {} \; -print;
	fi
	# if user on input filename stage, we should give him new prompt
	if [[ $filename = '' ]]; then
		printPrompt
	fi
}

# rewrite SIGINT handler
trap 'sigintHandler' SIGINT

# get filename from user input
getCorrectFileName

# get words count, print info to STDOUT and file info.txt
wc $filename | awk {'print "Words count: " $2'} | tee info.txt

# command cycle (for interruptions)
while true
do
	echo 'Enter "exit" to stop or Ctrl+C to show the list of non-binary files in current dir'
	read command
	if [[ $command = "exit" ]]; then
		exit 0
	fi
done