#!/bin/bash
#######################################################################################################################
#
# 	  Display some status information of ESPs
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

for ip in ${TARGET_IPs[@]}; do
	host="192.168.0.$ip"
	if ping -c 1 $host &>/dev/null; then
		echo "Querying $host:8080 ..."
		rsp="$(curl -s "http://$host:8080")"
		bld="$(echo "$rsp" | jq -C .bld)"
		echo $bld
		rt="$(echo "$rsp" | jq -C .rt)"
		echo $rt
		dev="$(echo "$rsp" | jq -C .dev)"
        echo $dev
	else
		echo "$host not online"
	fi
done
