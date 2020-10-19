#!/bin/bash

cgcreate -t calm:calm -a calm:calm -g memory:/calmwu_cg
echo $((512 * 1024 * 1024)) > /sys/fs/cgroup/memory/calmwu_cg/memory.limit_in_bytes
echo $((512 * 1024 * 1024)) > /sys/fs/cgroup/memory/calmwu_cg/memory.memsw.limit_in_bytes