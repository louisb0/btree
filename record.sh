#!/bin/bash

make profile
sudo perf record --delay 2000 ./build/profile
