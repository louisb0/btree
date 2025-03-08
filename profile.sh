#!/bin/bash

current_governor=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor)

echo "Setting CPU governor to performance mode for core 0..."
echo performance | sudo tee /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

make profile
sudo perf stat --delay 2000 -e \
task-clock,context-switches,cpu-migrations,page-faults,\
cycles,stalled-cycles-frontend,instructions,\
branches,branch-misses,\
cache-references,cache-misses,\
L1-dcache-loads,L1-dcache-load-misses,\
dTLB-loads,dTLB-load-misses \
./build/profile

echo "Restoring original CPU governor setting..."
echo $current_governor | sudo tee /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
