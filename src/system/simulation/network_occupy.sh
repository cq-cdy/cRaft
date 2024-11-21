#!/bin/bash

if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 
   exit 1
fi

# 获取本机的网络接口名称
interface=$(ip link show | grep -Eo 'ens0|ens33|eth0' | head -n 1)

if [[ $interface == "ens0" ]]; then
    interface="eth0"
elif [[ $interface == "ens33" ]]; then
    interface="ens33"
fi

max_bandwidth=$(ethtool $interface | grep "Speed:" | awk '{print $2}' | tr -d 'Mb/s')


if ! [[ $max_bandwidth =~ ^[0-9]+$ ]]; then
  echo "Failed to get valid bandwidth from ethtool."
  exit 1
fi

# 将最大带宽转换为bps（bits per second）
max_bandwidth_bps=$(($max_bandwidth * 1000 * 1000))
echo "Maximum bandwidth in bps: $max_bandwidth_bps"


while true; do
  # 随机生成占用带宽的百分比（95%-100%）
  bandwidth_usage=$(($RANDOM % 50 + 20))
    echo "Bandwidth usage percentage: $bandwidth_usage"
  # 计算实际带宽值
  target_bandwidth=$(($max_bandwidth_bps * $bandwidth_usage / 100))
  echo "Target bandwidth in bps: $target_bandwidth"

  # 随机生成持续时间（3-6秒）
  duration=$((RANDOM % 2 + 1))

  # 随机生成下一次占用的间隔时间（10-25秒）
  interval=$((RANDOM % 5 + 5))

  # 启动iperf服务器
  echo "Starting iperf server..."
  iperf -s > /dev/null 2>&1 &
  iperf_pid=$!

  # 等待服务器启动
  sleep 2

  # 启动iperf客户端，占用指定带宽
  echo "Occupying bandwidth for $duration seconds..."
  iperf -c 127.0.0.1 -t $duration -b ${target_bandwidth} > /dev/null 2>&1

  # 杀死iperf服务器进程
  kill $iperf_pid
  wait $iperf_pid 2>/dev/null

  # 等待下一次占用
  echo "Waiting for $interval seconds before next bandwidth occupation..."
  sleep $interval
done
