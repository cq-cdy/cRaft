#!/bin/bash

# 源目录
source_dir="./"

# 目标备份目录数组
backup_dirs=( "../rafts/0" "../rafts/1" "../rafts/2" "../rafts/3" "../rafts/4")
# 要排除的目录
exclude_dir="build"
exclude_dir2=".data/persist"
exclude_dir3="bin"
exclude_dir4="cmake-build-debug"
exclude_dir5="lib"
# 遍历每个备份目录
for dir in "${backup_dirs[@]}"; do
    # 创建备份目录，如果它不存在的话
    mkdir -p "$dir"

    # 使用 rsync 进行同步
    rsync -av --delete --exclude="$exclude_dir" --exclude="$exclude_dir2" --exclude="$exclude_dir3" --exclude="$exclude_dir4" --exclude="$exclude_dir5" "$source_dir" "$dir"
done

echo "分发完成"
