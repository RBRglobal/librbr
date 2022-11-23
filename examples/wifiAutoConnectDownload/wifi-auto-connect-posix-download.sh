#! /bin/bash

# This file runs on Ubuntu and Raspbian. Interrupt with Ctl+C anytime.
# Run with command $./<filename.sh> or $sh ./<filename.sh>
# If it doesn't work, use command $chmod u+x <filename.sh> first before running.

SCRIPT_DIR="$(dirname "$0")"

service_status=3

clean_up() { # Perform pre-exit housekeeping
  sudo ip addr flush dev $wl_name
  if [ "$service_status" -eq "0" ]
  then	
    sudo systemctl restart NetworkManager wpa_supplicant dhcpcd >/dev/null 2>&1
    echo "================================"
  else
    echo "================================"
    echo "Tip: please use \"sudo systemctl restart NetworkManager wpa_supplicant dhcpcd >/dev/null 2>&1\" command to bring network services up."
  fi
  exit 0
}

signal_exit() { # Handle trapped signals  
  local signal="$1"
  
  case "$signal" in
    INT)
      echo "Program interrupted by user" ;;
    TERM)
      echo "Program terminated" ;;
    *)
      echo "Terminating on unknown signal" ;;
  esac
  clean_up
}

# Trap signals
trap "signal_exit TERM" TERM HUP
trap "signal_exit INT"  INT

# get wifi card name:
wl_name=$(ls /sys/class/ieee80211/*/device/net)

echo "Please turn on the instrument wifi and wait..."

# check if wpa_supplicant is running at initial state:
sudo systemctl status wpa_supplicant >/dev/null 2>&1
service_status=$(echo $?)
# if wpa_supplicant is running at intial state, stop it for now:
if [ "$service_status" -eq "0" ]
then
  sudo systemctl stop NetworkManager wpa_supplicant dhcpcd >/dev/null 2>&1
  sleep 3s
  sudo ip link set dev "$wl_name" up
fi

# search for RBR instrument wifi:
echo "Searching for RBRinstrument wifi"
rbr_wifi=$(sudo iw dev "$wl_name" scan | grep -Po "SSID: \KRBR [[:digit:]]+") >/dev/null
if [ -z "$rbr_wifi" ]
then
	echo "RBR instrument wifi not found. Please check or restart instrument wifi."
	exit 1
else 
	echo "RBR instrument wifi found: $rbr_wifi"
fi

# auto connect to RBR instrument wifi and download data:
sudo iw dev "$wl_name" connect -w "$rbr_wifi" >/dev/null 2>&1
sudo dhclient "$wl_name" >/dev/null 2>&1
echo "================================"
$SCRIPT_DIR/../posix/posix-download-wifi

# put wpa_supplicant to initial state
clean_up