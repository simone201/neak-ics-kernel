#!/sbin/busybox sh
# EFS Backup script
# by simone201 & GM

if [ ! -d /sdcard/near ]; then
	mkdir  /sdcard/near
else
	echo "near sdcard folder already exists"
fi;

if [ ! -f /emmc/near/efsbackup.tar.gz ];
then
  /sbin/busybox tar zcvf /emmc/near/efsbackup.tar.gz /efs
  /sbin/busybox dd if=/dev/block/mmcblk0p1 of=/emmc/near/efsdev-mmcblk0p1.img bs=4096
  /sbin/busybox gzip /emmc/near/efsdev-mmcblk0p1.img
else
	echo "efs backup already exists"
fi;
