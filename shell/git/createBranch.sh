#!/bin/sh
branchname="$1"
if [ "$branchname" = "" ]; then
	echo "branch name?"	
else
	git checkout -b $branchname
	git push origin $branchname
fi
