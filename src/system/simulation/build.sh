#!/bin/bash
check_and_install() {
    if ! dpkg -l | grep -qw "$1"; then
        echo "$1 is not installed, installing..."
        sudo apt-get install -y "$1"
        echo "$1 has been installed."
    else
        echo "Detected $1 is already installed."
    fi
}
# 检查并安装iperf
check_and_install iperf

# 检查并安装ethtool
check_and_install ethtool

g++  cpu_occupy.cc -o cpu_occupy
g++ memory_occupy.cc -o mem_occupy