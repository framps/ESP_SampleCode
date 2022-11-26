#######################################################################################################################
#
# 	 Find all ESP sensors in local network
#
#######################################################################################################################
#
#    Copyright (c) 2021 framp at linux-tips-and-tricks dot de
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#######################################################################################################################

#!/bin/bash

VERSION=0.1
MYSELF="$(basename "$0")"
MYNAME="${MYSELF%.*}"
MACLIST_FILENAME=/usr/local/etc/${MYNAME}.mac	# used to lookup descriptions for macs if host doesn't return any information

# Adapt following mac address regex to you local environment
MAC_Addresses=" (10:52:1c|24:6f:28|24:62:ab|a4:cf:12|f4:cf:a2|e0:98:06|fc:f5:c4|48:3f:da|bc:dd|24:a1:60)"

if ! command -v nmap COMMAND &> /dev/null; then
	echo "Missing required program nmap."
	exit 1
fi

if ! command -v host COMMAND &> /dev/null; then
	echo "Missing required program host."
	exit 1
fi

DEFAULT_SUBNETMASK="192.168.0.0/24"

if [[ "$1" =~ ^(-h|--help|-\?)$ ]]; then
	cat << EOH
Usage:
	$MYSELF                       Scan subnet $DEFAULT_SUBNETMASK for IOT devices
	$MYSELF <subnetmask>          Scan subnet for Raspberries
	$MYSELF -h | -? | --help      Show this help text

Defaults:
	Subnetmask: $DEFAULT_SUBNETMASK

Example:
	$MYSELF 192.168.179.0/24

EOH
	exit 0
fi

MY_NETWORK=${1:-$DEFAULT_SUBNETMASK}

echo "Scanning subnet $MY_NETWORK for IOT devices ..."

declare -A macAddress=()
declare -A hostName=()

devicesFound=0

# 192.168.0.12             ether   dc:a6:32:8f:28:fd   C                     wlp3s0 -
while read ip dummy mac rest; do
	macAddress["$ip"]="$mac"
done < <(nmap -sP $MY_NETWORK &>/dev/null; arp -n | grep -E $MAC_Addresses)

echo "${#macAddress[@]} IOT devices detected"

if (( ${#macAddress[@]} > 0 )); then
	echo "Retrieving hostnames ..."

	printf "%-15s %-17s %s\n" "IP address" "Mac address" "Hostname"

	# 12.0.168.192.in-addr.arpa domain name pointer asterix.
	for ip in "${!macAddress[@]}"; do
		h="$(host "$ip")"
		if (( ! $? )); then
			read arpa dummy dummy dummy host rest <<< "$h"
			host="${host::-1}" # delete trailing "."
			host="$(cut -f 1 -d . <<< "$host")"
		else
			macMapping="$(grep ${macAddress[$ip]} $MACLIST_FILENAME)"
			if (( ! $? )); then
				host="-$(cut -f 2 -d ' ' <<< "$macMapping")-"
			else
				host="-Unknown-"
			fi
		fi
		printf "%-15s %17s %s\n" $ip ${macAddress[$ip]} $host
	done
fi
