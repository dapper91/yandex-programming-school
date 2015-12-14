#!/bin/bash

FILE_NAME=`basename $0`

API_URL='http://api.vk.com'
METHOD='/method/users.get.json'
STATUS_FILE="$HOME/.vkusers_status"

usage() {
cat <<EOF
$FILE_NAME -u VK_UID
$FILE_NAME [-i MIN] -a VK_UID
$FILE_NAME -d VK_UID
$FILE_NAME -h

Options:
	-u VK_UID	show if vk user status with VK_UID has been changed
	-a VK_UID	add vk user traking with VK_UID to cron
	-d VK_UID	del vk user traking with VK_UID from cron
	-i MIN		checking interval (default 15)
	-h 	        help
EOF
}

is_number() {
	if [ "$1" -eq "$1" ] 2>/dev/null ; then
 		return 0
	else
  		return 1
	fi
}

is_online() {
	local VK_UID=$1

	local STATUS=`curl "$API_URL$METHOD?uid=$VK_UID&fields=online" 2> /dev/null | grep -Po '(?<="online":).'`
	if [ "$?" != 0 ]; then
		echo 'vk connection error' 1>&2
		return 1
	fi

	is_number "$STATUS"
	if [ "$?" != 0 ]; then
		echo 'vk response error' 1>&2
		return 1
	fi

	echo "$STATUS"
	return 0
}

get_name_by_id() {
	local VK_UID=$1

	local RESPONSE=`curl "$API_URL$METHOD?uid=$VK_UID&fields=first_name,last_name" 2> /dev/null`	
	if [ "$?" != 0 ]; then
		echo 'vk connection error' 1>&2
		return 1
	fi	

	local FIRST_NAME=`echo "$RESPONSE" | grep -Po '(?<="first_name":").*?(?=")'`
	local LAST_NAME=`echo "$RESPONSE" | grep -Po '(?<="last_name":").*?(?=")'`

	echo "$FIRST_NAME $LAST_NAME"	
}

is_status_changed() {
	local VK_UID=$1

	if [ ! -f "$STATUS_FILE" ]; then		
		touch "$STATUS_FILE" &> /dev/null	
		if [ "$?" != 0 ]; then
			echo "status file $STATUS_FILE can't be created" 1>&2
			return 1
		fi
	fi	

	local CURRENT_STATUS=`is_online $VK_UID`
	if [ "$?" != 0 ]; then		
		return 1
	fi

	local LAST_STATUS=`awk -v uid="$VK_UID" '$1==uid {print $2}' $STATUS_FILE`	
	if [ -z "$LAST_STATUS" ] || [ "$LAST_STATUS" -ne "$CURRENT_STATUS" ]; then	
		TMP_FILE=`mktemp 2> /dev/null`
		if [ "$?" != 0 ]; then
			echo "tmp file $TMP_FILE can't be created" 1>&2
			return 1
		fi	

		awk -v uid="$VK_UID" '$1!=uid {print}' "$STATUS_FILE" > "$TMP_FILE"
		echo "$1	$CURRENT_STATUS" >> "$TMP_FILE"
		mv "$TMP_FILE" "$STATUS_FILE" &> /dev/null
		echo "1 $CURRENT_STATUS"
	else 
		echo "0 $CURRENT_STATUS"
	fi

	return 0
}

notify() {	
	VK_UID=$1

	which notify-send &> /dev/null
	if [ $? != 0 ]; then
		echo "notify-send not found" 1>&2
		exit 1
	fi
	
	RESULT=`is_status_changed $VK_UID`	
	if [ $? != 0 ]; then		
		exit 1
	fi

	CHANGED=`echo $RESULT | cut -d ' ' -f1`
	CURRENT_STATUS=`echo $RESULT | cut -d ' ' -f2`
	if [ "$CHANGED" -eq 1 ]; then
		USER_NAME=`get_name_by_id $VK_UID`			
		if [ "$CURRENT_STATUS" -eq 0 ]; then
			MESSAGE="VK User $USER_NAME is offline"
		else
			MESSAGE="VK User $USER_NAME is online"
		fi

		export $(egrep -z DBUS_SESSION_BUS_ADDRESS /proc/$(pgrep -u $LOGNAME gnome-session)/environ)
		notify-send "$MESSAGE" -i gtk-info &> /dev/null
	fi
}

add_to_cron() {
	VK_UID=$1

	CRON_JOB="*/${INTERVAL:-15} * * * * $0 -u $VK_UID"	
	crontab -l 2> /dev/null | { cat; echo "$CRON_JOB"; } | crontab -	
}

del_from_cron() {
	VK_UID=$1
	
	crontab -l 2> /dev/null | { grep -v "$FILE_NAME -u $VK_UID"; } | crontab - 
}


while getopts "hu:i:a:d:" opt ; do
	case "$opt" in
	u) notify "$OPTARG" ;;
	i) INTERVAL="$OPTARG" ;;
	a) add_to_cron "$OPTARG" ;;
	d) del_from_cron "$OPTARG" ;;
	h) usage ; exit 0 ;;
	*) usage ; exit 1 ;;
	esac
done