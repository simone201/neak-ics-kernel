#!/sbin/busybox sh
# Setting Bus ASV table entries
# mod by myfluxi - recovery interface by simone201

mount -o remount,rw /data

# Check if our folder is there...
if [ ! -d /data/neak ]; then
	echo "creating /data/neak folder"
	mkdir /data/neak
else
	echo "neak data folder already exists"
fi;

# Check if the asv set file is there, if not create it
if [ ! -e /data/neak/bus_asv_table ]; then
	touch /data/neak/bus_asv_table
	echo 4 > /data/neak/bus_asv_table
fi;

# Checking ASV group set
asv_table=`/sbin/busybox cat /data/neak/bus_asv_table`

# Setting the selected group
if [ -e /sys/devices/system/cpu/cpufreq/busfreq_asv_group ]; then
	echo $asv_table > /sys/devices/system/cpu/cpufreq/busfreq_asv_group
else
	echo "This kernel doesn't support modding of bus asv group!"
fi;
