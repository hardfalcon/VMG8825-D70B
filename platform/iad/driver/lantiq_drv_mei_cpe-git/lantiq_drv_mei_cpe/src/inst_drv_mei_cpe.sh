#! /bin/sh
#
# Install MEI CPE device driver
#
# if no para : use local debug level
# para 1 ($1): debug level (0 = use local debug level)
# para 2 ($2): number of lines (default: 2)
# para 3 ($3): number of devices (default: 2)
# 

# check for linux 2.6.x
uname -r | grep -q 2.6.
if [ $? -eq 0 ]; then
    MODEXT=.ko
fi

#drv_major_number=252
drv_dev_base_name=mei_cpe
drv_obj_file_name=drv_mei_cpe$MODEXT

lines=1
devices=1

# set debug_level: 1=low, 2=normal, 3=high, 4=off
debug_level=1

# use parameter as debug_level, if != 0
if [ $# != 0 ] && [ "$1" != 0 ]; then
   debug_level=$1
fi

# enable debugging outputs, if necessary
if [ "$debug_level" != 4 ]; then
    echo 8 > /proc/sys/kernel/printk
fi

echo "- loading MEI CPE device driver -"
insmod -f ./$drv_obj_file_name debug_level=$debug_level 
# add "drv_major_number=$drv_major_number" for fixed major number

if [ $? -ne 0 ]; then
    echo "- loading driver failed! -"
    exit 1
fi

major_no=`grep mei_cpe /proc/devices |cut -d' ' -f1`

# exit if major number not found (in case of devfs)
if [ -z $major_no ]; then
    exit 0
fi

echo - create device nodes for MEI CPE device driver -

prefix=/dev/$drv_dev_base_name
test ! -d $prefix/ && mkdir $prefix/

# get numbers from params
if [ $# -ge 1 ]; then
    lines=$2
fi
if [ $# -ge 2 ]; then
    devices=$3
fi

I=0
while test $I -lt $lines; do
    test ! -e $prefix/$I && mknod $prefix/$I c $major_no `expr $I`
    I=`expr $I + 1`
done

I=0
while test $I -lt $devices; do 
    test ! -e $prefix/cntrl$I && mknod $prefix/cntrl$I c $major_no `expr $I + 128`
    I=`expr $I + 1`
done


