
do_fixrootfs () {
	#rm -rf ${IMAGE_ROOTFS}/usr/lib/libQt5WaylandClient.so.5.11.3
	#rm -rf ${IMAGE_ROOTFS}/usr/lib/libQt5Widgets.so.5.11.3
	#rm -rf ${IMAGE_ROOTFS}/usr/lib/libstdc++.so.6
	#ln -sf /usr/lib/libstdc++.so.6.0.25 ${IMAGE_ROOTFS}/usr/lib/libstdc++.so.6

	rm -rf ${IMAGE_ROOTFS}/boot/uImag*
	rm -rf ${IMAGE_ROOTFS}/boot/zImag* 
	rm -rf ${IMAGE_ROOTFS}/boot/.debug

	ROOTPASSWD=`grep "^root" ${IMAGE_ROOTFS}/etc/passwd`
	ROOTSHADOW=`grep "^root" ${IMAGE_ROOTFS}/etc/shadow`
	sed -i "/^root/ d ; 1i${ROOTPASSWD}" ${IMAGE_ROOTFS}/etc/passwd
	sed -i "/^root/ d ; 1i${ROOTSHADOW}" ${IMAGE_ROOTFS}/etc/shadow	

	# reduce python stuff hack
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/__pycache__
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/asyncio
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/concurrent
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/ctypes
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/curses
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/dbm
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/distutils
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/email
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/ensurepip
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/html
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/http
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/idlelib
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/json
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/lib2to3
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/logging
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/multiprocessing
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/plat-linux
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/pydoc_data
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/sqlite3
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/test
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/tkinter
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/turtledemo
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/unittest
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/venv
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/wsgiref
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/xml
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/xmkprc	
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/turtle.py
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/typing.py
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/tpydoc.py
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/pdb.py
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/tpydoc.py
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/mailbox.py
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/tpydoc.py
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/ipaddress.py
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/doctest.py
	rm -rf  ${IMAGE_ROOTFS}/usr/lib/python3.5/_pydecimal.py

	
}
