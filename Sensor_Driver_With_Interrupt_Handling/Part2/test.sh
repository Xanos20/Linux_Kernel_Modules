#!/bin/bash

echo "Set Trigger"
echo -n 7 > /sys/class/HCSR/HCSR_2/trigger_pin
cat /sys/class/HCSR/HCSR_2/trigger_pin

echo "Set Echo"
echo -n 4 > /sys/class/HCSR/HCSR_2/echo_pin
cat /sys/class/HCSR/HCSR_2/echo_pin

echo "Sampling Period"
echo -n 150 > /sys/class/HCSR/HCSR_2/sampling_period
cat /sys/class/HCSR/HCSR_2/sampling_period

echo "Number Samples"
echo -n 7 > /sys/class/HCSR/HCSR_2/number_samples
cat /sys/class/HCSR/HCSR_2/sampling_period

echo "Latest Distance="
cat /sys/class/HCSR/HCSR_2/latest_distance

echo "Check Enable="
cat /sys/class/HCSR/HCSR_2/enable_measurements
sleep 2

echo "Start Measurements" 
echo -n 1 > /sys/class/HCSR/HCSR_2/enable_measurements

sleep 7

echo "Latest Distance="
cat /sys/class/HCSR/HCSR_2/latest_distance


sleep 5
echo "Start Second Device"
echo "Set Echo"
echo -n 5 > /sys/class/HCSR/HCSR_4/echo_pin
cat /sys/class/HCSR/HCSR_4/echo_pin
echo "Set Trigger"
echo -n 8 > /sys/class/HCSR/HCSR_4/trigger_pin
cat /sys/class/HCSR/HCSR_4/trigger_pin
echo -n 1 > /sys/class/HCSR/HCSR_4/enable_measurements
sleep 5
cat /sys/class/HCSR/HCSR_4/latest_distance



















