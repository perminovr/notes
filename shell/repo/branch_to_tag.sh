#!/bin/bash

DATE=$(date "+%Y-%m-%d")

svn_repo="http://trac.plant.elesy.com/usvn/svn/tm7-project"
branches="${svn_repo}/branches/boards"
tags="${svn_repo}/tags/boards"

Usage() {
	echo "\
		Make tag from branch by revision:
		-r | --revision REV
			revision number
		[-i | --interactive]
			enable interactive copy-mode
	"
	exit 1
}

while true; do
	case "$1" in
	-r | --revision)
		REV="$2"; shift 2
	;;
	-i | --interactive)
		interactive="1"; shift 2
	;;
	--help)
		Usage
		break
	;;
	* )
		break
	;;
	esac
done

if [ "$REV" == "" ]; then
	echo "Revision does not set!"
	echo
	Usage
fi

msg="$DATE tag of $REV was made automatically"

echo -n "Enter svn admin login: "
read SVNL
echo -n "Enter svn admin passwd: "
read -s SVNP
echo

BDIRS=`svn ls --username $SVNL --password $SVNP -R $branches | grep "${REV}/$"`

if [ "$BDIRS" == "" ]; then
	echo "Revision ${REV} not found in $branches!"
	exit 1
fi

for bd in $BDIRS ; do
	echo "Copy from"
	echo "   ${branches}/$bd"
	echo "to"
	echo "   ${tags}/$bd"
	ucmd="y"
	[ "$interactive" == "1" ] && read -n 1 -p "? [y/n/q]: " ucmd
	[ "$ucmd" == "q" ] && exit 1
	[ "$ucmd" == "y" ] && svn cp --username $SVNL --password $SVNP --parents -m "$msg" ${branches}/$bd ${tags}/$bd
	echo "################################################################################"
done

exit 0


