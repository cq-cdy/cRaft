#!/bin/bash
source_dir="./"

backup_dirs=( "../rafts/0" "../rafts/1" "../rafts/2" "../rafts/3" "../rafts/4")

exclude_dir="build"
exclude_dir2=".data/persist"
exclude_dir3="bin"
exclude_dir4="cmake-build-debug"
exclude_dir5="lib"
for dir in "${backup_dirs[@]}"; do

    mkdir -p "$dir"

    rsync -av --delete --exclude="$exclude_dir" --exclude="$exclude_dir2" --exclude="$exclude_dir3" --exclude="$exclude_dir4" --exclude="$exclude_dir5" "$source_dir" "$dir"
done

echo "分发完成"
