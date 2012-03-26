#!/bin/bash

if [ -e zImage ]; then
	rm zImage
fi

rm compile.log

# Set Default Path
TOP_DIR=$PWD
KERNEL_PATH="/home/simone/neak-ics"

# Set toolchain and root filesystem path
#TOOLCHAIN="/home/simone/arm-2009q3/bin/arm-none-linux-gnueabi-"
TOOLCHAIN="/home/simone/android-toolchain-eabi/bin/arm-eabi-"
#TOOLCHAIN="/home/simone/android/system/prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin/arm-eabi-"
ROOTFS_PATH="/home/simone/neak-ics/initramfs-aosp"

export KBUILD_BUILD_VERSION="N.E.A.K-2.0x"
export KERNELDIR=$KERNEL_PATH

export USE_SEC_FIPS_MODE=true

echo "Cleaning latest build"
make ARCH=arm CROSS_COMPILE=$TOOLCHAIN -j`grep 'processor' /proc/cpuinfo | wc -l` mrproper

# Making our .config
make neak_ics_aosp_defconfig

make -j`grep 'processor' /proc/cpuinfo | wc -l` ARCH=arm CROSS_COMPILE=$TOOLCHAIN CONFIG_INITRAMFS_SOURCE="$ROOTFS_PATH" >> compile.log 2>&1 || exit -1

# Copying kernel modules
find -name '*.ko' -exec cp -av {} $ROOTFS_PATH/lib/modules/ \;

make -j`grep 'processor' /proc/cpuinfo | wc -l` ARCH=arm CROSS_COMPILE=$TOOLCHAIN CONFIG_INITRAMFS_SOURCE="$ROOTFS_PATH" || exit -1

# Copy Kernel Image
rm -f $KERNEL_PATH/releasetools/zip/$KBUILD_BUILD_VERSION.zip
cp -f $KERNEL_PATH/arch/arm/boot/zImage .
cp -f $KERNEL_PATH/arch/arm/boot/zImage $KERNEL_PATH/releasetools/zip

cd arch/arm/boot
tar cf $KERNEL_PATH/arch/arm/boot/$KBUILD_BUILD_VERSION.tar ../../../zImage && ls -lh $KBUILD_BUILD_VERSION.tar

cd ../../..
cd releasetools/zip
zip -r $KBUILD_BUILD_VERSION.zip *

cp $KERNEL_PATH/arch/arm/boot/$KBUILD_BUILD_VERSION.tar $KERNEL_PATH/releasetools/tar/$KBUILD_BUILD_VERSION.tar
rm $KERNEL_PATH/arch/arm/boot/$KBUILD_BUILD_VERSION.tar
rm $KERNEL_PATH/releasetools/zip/zImage
