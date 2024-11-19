#!/bin/bash

# 用于设置环境变量RAFT_HOME_PATH到当前工作目录的绝对路径
# 获取脚本所在的绝对路径

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )


# 用于设置环境变量RAFT_HOME_PATH到脚本所在的目录
export RAFT_HOME_PATH="$SCRIPT_DIR"

# 检查参数，并根据参数执行相应操作
if [[ "$1" == "--clean" ]]; then
    # 如果第一个参数是--clean，则清除环境变量
    unset RAFT_HOME_PATH
    unset RAFT_TEST_ID
    echo "Environment variables RAFT_HOME_PATH and RAFT_TEST_ID have been cleared."
    return
fi

if [[ "$1" == "--local" ]]; then
    if [[ -n "$2" ]]; then
        
        if [[ "$2" =~ ^[0-9]+$ ]]; then
            export RAFT_TEST_ID="$2"
        else
            echo "Error: The ID specified for --local option must be a number."
            return
        fi
    else
        echo "Error: No ID specified for --local option."
        return
    fi
fi

# 如果有参数，但不是--clean和--local，则报错
if [[ -n "$1" && "$1" != "--clean" && "$1" != "--local" ]]; then
    echo "Error: Invalid option. Valid options are '--clean' to clear environment variables or '--local [id]' to set the RAFT_TEST_ID."
    return
fi

echo "RAFT_HOME_PATH: $RAFT_HOME_PATH"
if [[ -n "$RAFT_TEST_ID" ]]; then
    echo "RAFT_TEST_ID: $RAFT_TEST_ID"
fi