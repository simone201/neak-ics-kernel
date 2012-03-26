#!/sbin/busybox sh
# Doing some cleanup
# by Simone201

mount -o remount,rw /system

if [ -e /data/app/NEAK-Downloader.apk ]; then
	rm /data/app/NEAK-Downloader.apk
fi;

if [ -e /data/neak/downloader ]; then
	rm /data/neak/downloader
fi;

if [ -e /data/neak/lagfree ]; then
	rm /data/neak/lagfree
fi;

if [ -e /system/etc/init.d/S98bolt_siyah ]; then
	rm /system/etc/init.d/S98bolt_siyah
fi;

if [ -e /system/etc/init.d/s78enable_touchscreen_1 ]; then
	rm /system/etc/init.d/s78enable_touchscreen_1
fi;

if [ -e /system/etc/init.d/S02conservative ]; then
	rm /system/etc/init.d/S02conservative
	touch /data/neak/conservative
fi;

if [ -e /system/etc/init.d/S03lazy ]; then
	rm /system/etc/init.d/S03lazy
	touch /data/neak/lazy
fi;

if [ -e /system/etc/init.d/S04voodoo ]; then
	rm /system/etc/init.d/S04voodoo
fi;

if [ -e /system/etc/lionheart ]; then
	rm /system/etc/lionheart
	touch /data/neak/lionheart
fi;

if [ -e /system/etc/schedmc ]; then
	rm /system/etc/schedmc
	touch /data/neak/schedmc
fi;

mount -o remount,ro /system
