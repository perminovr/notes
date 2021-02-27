#!/bin/bash

Usage() {
	echo "Usage: createRepo.sh OPTS [LOCAL]"
	echo "	OPTS:"
	echo "		-u USERNAME"
	echo "		-t GITHUB_TOKEN"
	echo "		-r NEW_REPONAME"
	echo "	LOCAL: local path"
	exit 0
}

USERNAME=""
TOKEN=""
REPONAME=""
LOCALPATH=""
while [ $# -gt 0 ]; do
	case $1 in
		-u)
			USERNAME="$2"
			shift 2
		;;
		-t)
			TOKEN="$2"
			shift 2
		;;
		-r)
			REPONAME="$2"
			shift 2
		;;
		-h) Usage
		;;
	esac
done

[ "$USERNAME" == "" -o "$TOKEN" == "" -o "$REPONAME" == "" ] && Usage

LOCALPATH="$1"
[ "$LOCALPATH" == "" ] && LOCALPATH=`pwd`

mkdir -p $LOCALPATH/$REPONAME
cd $LOCALPATH/$REPONAME

curl -H "Authorization: token $TOKEN" https://api.github.com/user/repos -d "{\"name\":\"$REPONAME\"}"
git init
echo "# $REPONAME" > README.md
touch .gitignore
git add README.md
git add .gitignore
git commit -m "Initial commit"
git remote add origin https://github.com/$USERNAME/$REPONAME.git
git push -u origin master

