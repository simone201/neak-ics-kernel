#!/sbin/busybox sh
# Installing Superuser apk + binary
# thx to hardcore
# Modified by Simone201

/sbin/busybox mount /system -o remount,rw

echo "Copying su binary"
/sbin/busybox rm /system/bin/su
/sbin/busybox rm /system/xbin/su
/sbin/busybox cp /res/misc/su /system/xbin/su
/sbin/busybox chown 0.0 /system/xbin/su
/sbin/busybox chmod 4755 /system/xbin/su

#echo "Copying Superuser.apk"
echo "Cleaning Superuser.apk for next installation"
/sbin/busybox rm /system/app/Superuser.apk
/sbin/busybox rm /data/app/Superuser.apk
#/sbin/busybox xzcat /res/misc/Superuser.apk.xz > /system/app/Superuser.apk
#/sbin/busybox chown 0.0 /system/app/Superuser.apk
#/sbin/busybox chmod 644 /system/app/Superuser.apk

/sbin/busybox mount /system -o remount,ro
