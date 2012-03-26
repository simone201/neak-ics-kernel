#!/sbin/busybox sh
# All-in-one script for NEAK Options
# Only compatible with NEAK app/cwm
# by Simone201

mount -o remount,rw /data

# Check if our folder is there...
if [ ! -d /data/neak ]; then
	echo "creating /data/neak folder"
	mkdir /data/neak
else
	echo "neak data folder already exists"
fi;

# Conservative Module
if [ -e /data/neak/conservative ]; then
	echo "conservative module enabled"
	insmod /lib/modules/cpufreq_conservative.ko
fi;

# Lionheart Tweaks
if [ -e /data/neak/lionheart ]; then
	echo "lionheart tweaks enabled"
	./sbin/near/lionheart.sh
fi;

# Lazy Governor
if [ -e /data/neak/lazy ]; then
	echo "lazy module enabled"
	insmod /lib/modules/cpufreq_lazy.ko
fi;

# SCHED_MC Feature
if [ -e /data/neak/schedmc ]; then
	echo "schedmc enabled"
	echo "1" > /sys/devices/system/cpu/sched_mc_power_savings
else
	echo "schedmc disabled"
	echo "0" > /sys/devices/system/cpu/sched_mc_power_savings
fi;

# AFTR Idle Mode
if [ -e /data/neak/aftridle ]; then
	echo "aftr idle mode enabled"
	echo "3" > /sys/module/cpuidle_exynos4/parameters/enable_mask
fi;

# EXT4 Speed Tweaks
if [ -e /data/neak/ext4boost ]; then
	echo "ext4 boost tweaks enabled"
	/sbin/busybox mount -o noatime,remount,rw,discard,barrier=0,commit=60,noauto_da_alloc,delalloc /cache /cache;
	/sbin/busybox mount -o noatime,remount,rw,discard,barrier=0,commit=60,noauto_da_alloc,delalloc /data /data;
fi;
