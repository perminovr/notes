#!/bin/sh

# $1 = keys passphrase
# $2 = keys

PWD=$(pwd)
BIN_ELSYMA_M01="$PWD/arago-tmp-external-linaro-toolchain/deploy/images/mtax_am335x/zImage"
UBI_ELSYMA_M01="$PWD/arago-tmp-external-linaro-toolchain/deploy/images/mtax_am335x/core-image-minimal-mtax_am335x.ubi"
DTB_ELSYMA_M01_REV2="$PWD/arago-tmp-external-linaro-toolchain/deploy/images/mtax_am335x/zImage-elm01-rev2.dtb"
DTB_ELSYMA_M01_REV3="$PWD/arago-tmp-external-linaro-toolchain/deploy/images/mtax_am335x/zImage-elm01-rev3.dtb"

DTBS="$DTB_ELSYMA_M01_REV2 $DTB_ELSYMA_M01_REV3"



cat << EOF > k.img.its
/*
 * U-boot uImage source file with multiple kernels, ramdisks and FDT blobs
 */

/dts-v1/;

/ {
        description = "Kernel and filesystem";
        #address-cells = <1>;

        images {
                kernel@1 {
                        description = "elsyma-m01.bin";
                        data = /incbin/("$BIN_ELSYMA_M01");
                        type = "kernel";
                        arch = "arm";
                        os = "linux";
                        compression = "none";
                        load = <0x00200000>; /*"NAND.kernel"*/
                        hash@1 {
                                algo = "sha1";
                        };
			signature@1 {
				algo = "sha1,rsa2048";
				key-name-hint = "img";
			};
                };
                filesystem@1 {
                        description = "elsyma-m01.ubi";
                        data = /incbin/("$UBI_ELSYMA_M01");
                        type = "filesystem";
                        arch = "arm";
                        compression = "none";
                        load = <0x00a00000>; /*"NAND.rootfs"*/
                        hash@1 {
                                algo = "sha1";
                        };
			signature@1 {
				algo = "sha1,rsa2048";
				key-name-hint = "img";
			};
                };      

        };

        configurations {
                default = "config@1";

                config@1 {
                        description = "kernel fs";
                        kernel = "kernel@1";
                        filesystem = "filesystem@1";
                };

        };
};
EOF

TOOLS="/opt/build/tools"
[ "${SYSROOT_LINUX}" == "" ] && SYSROOT_LINUX="/opt/build/sysroots/elsyma-m01/i686-linux"
[ ! -d ${SYSROOT_LINUX} ] && SYSROOT_LINUX=""
[ "${SYSROOT_LINUX}" == "" ] && SYSROOT_LINUX="/opt/build/sysroots/elsyma-m01/x86_64"
[ ! -d ${SYSROOT_LINUX} ] && SYSROOT_LINUX=""

install -d -m 700 .keys
cd .keys
gpg --batch --yes --passphrase $1 --output keys.tar -d $2
retval=$?
if [ "$retval" == "0" ]; then
	tar -xf keys.tar
	retval=$?
fi
cd ..

if [ "$retval" == "0" ]; then
	RETVALERR="/tmp/retval_err"
	(
		PATH=$PATH:$TOOLS:$SYSROOT_LINUX/usr/bin
		LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$TOOLS
		for dtb in $DTBS; do
				$TOOLS/mkimage -f k.img.its -k .keys -K "$dtb" -r k.img.update
				if [ "$?" != "0" ]; then
					touch $RETVALERR
					break
				fi
		done
	)
	if [ -f $RETVALERR ]; then
		retval=1
		rm $RETVALERR
	fi
fi

rm -rf .keys k.img.its
exit $retval
