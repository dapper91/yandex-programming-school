#!/bin/bash

WATCH=
NAME=`basename $0`


function print_usage {
cat <<EOF
usage: 
	$NAME [-w <sec>] [options]		
	$NAME -h | --help

Prints system informaton about cpu, file systems, network interfaces, operation system, processes

Options:
	-c, --cpu                 list cpu info
	-m, --mem                 list memory info	
	-f, --fs                  list file systems info
	-n, --net                 list net info
	-o, --os                  list os info
	-p, --proc                list processes info
	-w, --watch <seconds>     list informaton in real time	
	-h, --help                show this screen
EOF
}

function list_cpu_info {
	printf "\nCPU INFORMATION:\n"
	printf "model:       %s\n" "`cat /proc/cpuinfo | grep -m 1 "model name" | cut -d ":" -f2 | sed 's/^ *//; s/ *$//'`"
	printf "processors:  %s\n" "`cat /proc/cpuinfo | grep processor | wc -l`"
	printf "cache:       %s\n" "`cat /proc/cpuinfo | grep -m 1 "cache size" | cut -d ":" -f2 | sed 's/^ *//; s/ *$//'`"
	printf "cpu MHz:     %s\n" "`cat /proc/cpuinfo | grep -m 1 "cpu MHz" | cut -d ":" -f2 | sed 's/^ *//; s/ *$//'`"
}

function list_mem_info {
	TMEM=`cat /proc/meminfo | grep MemTotal  | awk '{ print $2 }'`
	FMEM=`cat /proc/meminfo | grep MemFree   | awk '{ print $2 }'`
	TSWP=`cat /proc/meminfo | grep SwapTotal | awk '{ print $2 }'`
	FSWP=`cat /proc/meminfo | grep SwapFree  | awk '{ print $2 }'`

	printf "\nMEMORY INFORMATION:\n"
	printf "Total:       %s\n" `expr $TMEM / 1024`" MB"
	printf "Free:        %s\n" `expr $FMEM / 1024`" MB"
	printf "Swap informaton:\n"
	printf "Total:       %s\n" `expr $TSWP / 1024`" MB"
	printf "Free:        %s\n" `expr $FSWP / 1024`" MB"
}

function list_fss_info {
	printf "\nFILE SYSTEMS INFORMATION:\n"
	printf "%s\n" "`lsblk -o NAME,FSTYPE,SIZE,MODEL,LABEL,MOUNTPOINT`"
}

function list_net_info {
	printf "\nNETWORK INTERFACES INFORMATION:\n"
	printf "%s\n" "`ifconfig | egrep "HWaddr|inet"`"
}

function list_os_info {
	UPTIME=`cat /proc/uptime | awk '{ print $2 }' | cut -d "." -f1`
	MINS=`expr $UPTIME / 60`
	SECS=`expr $UPTIME % 60`

	printf "\nOS INFORMATION:\n"
	printf "Upitme:     %s\n" "$MINS min $SECS sec"
	printf "Kernel:     %s\n" "`uname -s`"
	printf "Version:    %s\n" "`uname -v`"
	printf "OS:         %s\n" "`uname -o`"
	printf "Release:    %s\n" "`uname -r`"
	printf "Machine:    %s\n" "`uname -m`"	
	printf "Nodename:   %s\n" "`uname -n`"

	printf "\nUSERS INFORMATION:\n"
	printf "Current: "
	printf " %s\n" "`w -h | awk '{ print $1 }' | uniq`"
	printf "Last:    "
	printf " %s" "`last | awk '{ print $1 }' | sort | uniq | sed '/./!d' | tr "\n" " " `"
	printf "\n"
	printf "I/O average load: "
	printf " %s\n" "`cat /proc/loadavg | awk '{ print "1 min: " $1 ", 5 min: " $2 ", 15 min: " $3 }'`" 
}

function list_proc_info {
	printf "ALL PROCESSES: %s\n" `ps h -e | wc -l`
	printf "TOP PROCESSES:\n"
	printf "%s\n" "`ps -Ao pcpu,pmem,comm | sort -rk 1,2 | head -n 10`"	
}

function list_info {	
	if [ -n "$CPU_INFO" ]; then		
		list_cpu_info	
	fi	
	if [ -n "$MEM_INFO" ]; then
		list_mem_info	
	fi
	if [ -n "$FS_INFO" ]; then
		list_fss_info	
	fi
	if [ -n "$NET_INFO" ]; then
		list_net_info	
	fi
	if [ -n "$OS_INFO" ]; then
		list_os_info	
	fi
	if [ -n "$PROC_INFO" ]; then
		list_proc_info	
	fi
}
export -f list_info
export -f list_cpu_info
export -f list_mem_info
export -f list_fss_info
export -f list_os_info
export -f list_net_info
export -f list_proc_info


while true ; do	
    case "$1" in
        -c|--cpu)			
            export CPU_INFO=1 ; shift 1 ;;        
        -m|--mem)
            export MEM_INFO=1 ; shift 1 ;;        
        -n|--net)
            export NET_INFO=1 ; shift 1 ;;
        -f|--fs)
            export FS_INFO=1 ; shift 1 ;;
        -o|--os)
            export OS_INFO=1  ; shift 1 ;;
        -p|--proc)
            export PROC_INFO=1 ; shift 1 ;;
        -w|--watch)
            WATCH="watch -t -n $2" ; shift 2 ;;
        -h|--help)
            print_usage ; exit 0 ;;  
        "")
        	break ;;             
        *)
            print_usage ; exit 1 ;;        
    esac
done

$WATCH bash -c 'list_info'
