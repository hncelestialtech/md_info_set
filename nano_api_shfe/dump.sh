#!/bin/bash
# 查找key=0x00000000的共享内存段ID

shmid=$(ipcs -m | awk '
BEGIN {
    FS="[[:space:]]+";
    found=0;
}
$1 == "0x00000000" && $2 ~ /^[0-9]+$/ {
    print $2;
    found=1;
    exit;
}
END {
    if (!found) exit 1;
}')

if [ $? -eq 0 ]; then
    echo "成功提取shmid: $shmid"
else
    echo "未找到key=0x00000000的共享内存段" >&2
    exit 1
fi


echo $shmid

./shm_tool $shmid dump bin ./dump

./shm_tool $shmid dump csv ./dump


