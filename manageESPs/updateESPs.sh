#!/bin/bash
#######################################################################################################################
#
# 	  Update all ESPs with actual sensor binary
#
#######################################################################################################################
#
#    Copyright (c) 2023 framp at linux-tips-and-tricks dot de
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

source ./sensors.sh

TARGET_IPs=("$ESP8266_IPs")
TARGET_FILE="$ESP8266_FILE"

if [[ ! -f ${TARGET_FILE} ]]; then
	echo "${TARGET_FILE} not found"
	exit 1
fi

if (( $# > 0 )); then
	TARGET_IPs=( "$1" )
	echo "Uploading to $1 only..."
fi

for ip in ${TARGET_IPs[@]}; do
	host="192.168.0.$ip"
	if ping -c 1 $host &>/dev/null; then
		echo "Uploading $TARGET_FILE to $host ..."
		rsp="$(curl -s -F "data=@$TARGET_FILE" "$host")"
		echo "Response: $(sed -E 's/^.+>//' <<< "$rsp")"
	else
		echo "$host not online"
	fi
done
