#!/bin/bash

DATE=$(date "+%Y-%m-%d")
WORKDIR_DEF=".workingdir"
WORKDIR="$WORKDIR_DEF"

svn_repo="http://trac.plant.elesy.com/usvn/svn/tm7-project"
trunk="${svn_repo}/trunk/hardcopies/boards"
tags="${svn_repo}/tags/boards"

Usage() {
	echo "\
		Make hard copy of tag:
		-r | --revision REV
			revision number
		[-d | --local-dir DIR]
			save local copy of trunked into DIR
	"
	exit 1
}

while true; do
	case "$1" in
	-r | --revision)
		REV="$2"; shift 2
	;;
	-d | --local-dir)
		WORKDIR="$2"; shift 2
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

msg="$DATE release of tag with rev $REV was made automatically"

echo -n "Enter svn login: "
read SVNL
echo -n "Enter svn passwd: "
read -s SVNP
echo

BDIRS=`svn ls --username $SVNL --password $SVNP -R $tags | grep "${REV}/$"`

if [ "$BDIRS" == "" ]; then
	echo "Revision ${REV} not found in $tags!"
	exit 1
fi

mkdir -p $WORKDIR
(
cd $WORKDIR

for bd in $BDIRS ; do
	svn co $tags/$bd --username ${SVNL} --password ${SVNP}
done

rm -rf `find * -type d -name ".svn"`

for bd in $BDIRS ; do
	cd `basename $bd`
	svn mkdir --parents -m "$msg" --username ${SVNL} --password ${SVNP} $trunk/$bd
	svn co --username ${SVNL} --password ${SVNP} $trunk/$bd ./
	svn add *
	svn ci -m "$msg" --username ${SVNL} --password ${SVNP}
	cd - &> /dev/null
done
)
#clean up
[ "$WORKDIR" == "$WORKDIR_DEF" ] && rm -rf $WORKDIR

exit 0


