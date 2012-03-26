#!/sbin/busybox sh
# thanks to hardcore and nexxx
# thanks to knzo, gokhanmoral, pikachu01
# modded by simone201 for NEAK Kernel

MMC=`ls -d /sys/block/mmc*`;

/sbin/busybox cp /data/user.log /data/user.log.bak
/sbin/busybox rm /data/user.log
exec >>/data/user.log
exec 2>&1

echo $(date) START of post-init.sh

##### Early-init phase #####

# IPv6 privacy tweak
  echo "2" > /proc/sys/net/ipv6/conf/all/use_tempaddr

# Remount all partitions with noatime
  for k in $(/sbin/busybox mount | /sbin/busybox grep relatime | /sbin/busybox cut -d " " -f3)
  do
        sync
        /sbin/busybox mount -o remount,noatime $k
  done

# Remount ext4 partitions with optimizations
  for k in $(/sbin/busybox mount | /sbin/busybox grep ext4 | /sbin/busybox cut -d " " -f3)
  do
        sync
        /sbin/busybox mount -o remount,commit=15 $k
  done
  
# Miscellaneous tweaks
  echo "1500" > /proc/sys/vm/dirty_writeback_centisecs
  echo "200" > /proc/sys/vm/dirty_expire_centisecs
  echo "0" > /proc/sys/vm/swappiness
  
# Thunderbolt! CFS Tweaks - by pikachu01
  sysctl -w kernel.sched_min_granularity_ns=200000;
  sysctl -w kernel.sched_latency_ns=400000;
  sysctl -w kernel.sched_wakeup_granularity_ns=100000;

# SD cards (mmcblk) read ahead tweaks
  echo "1024" > /sys/devices/virtual/bdi/179:0/read_ahead_kb
  echo "1024" > /sys/devices/virtual/bdi/179:16/read_ahead_kb
  echo "256" > /sys/devices/virtual/bdi/default/read_ahead_kb

# TCP tweaks
  echo "0" > /proc/sys/net/ipv4/tcp_timestamps;
  echo "1" > /proc/sys/net/ipv4/tcp_tw_reuse;
  echo "1" > /proc/sys/net/ipv4/tcp_sack;
  echo "1" > /proc/sys/net/ipv4/tcp_tw_recycle;
  echo "1" > /proc/sys/net/ipv4/tcp_window_scaling;
  echo "5" > /proc/sys/net/ipv4/tcp_keepalive_probes;
  echo "30" > /proc/sys/net/ipv4/tcp_keepalive_intvl;
  echo "30" > /proc/sys/net/ipv4/tcp_fin_timeout;
  echo "404480" > /proc/sys/net/core/wmem_max;
  echo "404480" > /proc/sys/net/core/rmem_max;
  echo "256960" > /proc/sys/net/core/rmem_default;
  echo "256960" > /proc/sys/net/core/wmem_default;
  echo "4096 16384 404480" > /proc/sys/net/ipv4/tcp_wmem;
  echo "4096 87380 404480" > /proc/sys/net/ipv4/tcp_rmem;

# UI tweaks
setprop debug.performance.tuning 1; 
setprop video.accelerate.hw 1;
setprop debug.sf.hw 1;

# Hotplug thresholds
echo "35" > /sys/module/stand_hotplug/parameters/load_l1
echo "75" > /sys/module/stand_hotplug/parameters/load_h0
echo "200" > /sys/module/stand_hotplug/parameters/rate

# Renice kswapd0 - kernel thread responsible for managing the memory
renice 6 `pidof kswapd0`

# New scheduler tweaks + readahead tweaks (thx to Pikachu01)
for i in $MMC;
do
	if [ -e $i/queue/rotational ]; 
	then
		echo "0" > $i/queue/rotational; 
	fi;
	if [ -e $i/queue/nr_requests ];
	then
		echo "8192" > $i/queue/nr_requests;
	fi;
	if [ -e $i/queue/read_ahead_kb ];
	then
		echo "256" >  $i/queue/read_ahead_kb;
	fi;
done;

# Misc Kernel Tweaks
sysctl -w vm.vfs_cache_pressure=70
echo "8" > /proc/sys/vm/page-cluster;
echo "64000" > /proc/sys/kernel/msgmni;
echo "64000" > /proc/sys/kernel/msgmax;
echo "10" > /proc/sys/fs/lease-break-time;
sysctl -w kernel.sem="500 512000 100 2048";
sysctl -w kernel.shmmax=268435456;

# Install Superuser apk + binary
	/sbin/busybox sh /sbin/near/install-su.sh

# Doing some cleanup before init.d support & neak options
    /sbin/busybox sh /sbin/near/cleanup.sh
	
# NEAK Options
	/sbin/busybox sh /sbin/near/neak-options.sh
	
read sync < /data/sync_fifo
rm /data/sync_fifo

echo $(date) PRE-INIT DONE of post-init.sh

##### Post-init phase #####

sleep 10

# init.d support
echo $(date) USER EARLY INIT START from /system/etc/init.d
if cd /system/etc/init.d >/dev/null 2>&1 ; then
    for file in E* ; do
        if ! cat "$file" >/dev/null 2>&1 ; then continue ; fi
        echo "START '$file'"
        /system/bin/sh "$file"
        echo "EXIT '$file' ($?)"
    done
fi
echo $(date) USER EARLY INIT DONE from /system/etc/init.d

echo $(date) USER EARLY INIT START from /data/init.d
if cd /data/init.d >/dev/null 2>&1 ; then
    for file in E* ; do
        if ! cat "$file" >/dev/null 2>&1 ; then continue ; fi
        echo "START '$file'"
        /system/bin/sh "$file"
        echo "EXIT '$file' ($?)"
    done
fi
echo $(date) USER EARLY INIT DONE from /data/init.d

echo $(date) USER INIT START from /system/etc/init.d
if cd /system/etc/init.d >/dev/null 2>&1 ; then
    for file in S* ; do
        if ! ls "$file" >/dev/null 2>&1 ; then continue ; fi
        echo "START '$file'"
        /system/bin/sh "$file"
        echo "EXIT '$file' ($?)"
    done
fi
echo $(date) USER INIT DONE from /system/etc/init.d

echo $(date) USER INIT START from /data/init.d
if cd /data/init.d >/dev/null 2>&1 ; then
    for file in S* ; do
        if ! ls "$file" >/dev/null 2>&1 ; then continue ; fi
        echo "START '$file'"
        /system/bin/sh "$file"
        echo "EXIT '$file' ($?)"
    done
fi
echo $(date) USER INIT DONE from /data/init.d

echo $(date) END of post-init.sh
