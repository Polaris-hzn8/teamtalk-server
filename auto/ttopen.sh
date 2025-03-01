#!/bin/bash
# this is a setup scripts for mysql table init.
# setup mysql

SQL_FILE=ttopen.sql
MYSQL_PASSWORD=20001201


print_hello(){
	echo "==========================================="
	echo "$1 prepare mysql tables for teamtalk."
	echo "==========================================="
}

check_user() {
	if [ $(id -u) != "0" ]; then
    	echo "Error: You must be root to run this script, please use root to install mysql"
    	exit 1
	fi
}

create_database() {
	cd ./conf/
	if [ -f "$SQL_FILE" ]; then
		echo "$SQL_FILE existed, begin to run $SQL_FILE"
	else
		echo "Error: $SQL_FILE not existed."
		cd ..
		return 1
	fi

	mysql -u root -p$MYSQL_PASSWORD < $SQL_FILE
	if [ $? -eq 0 ]; then
		echo "run sql successed."
		cd ..
	else
		echo "Error: run sql failed."
		cd ..
		return 1
	fi
}

build_all() {
	create_database
	if [ $? -eq 0 ]; then
		echo "create database successed."
	else
		echo "Error: create database failed."
		exit 1
	fi	
}


print_help() {
	echo "Usage: "
	echo "  $0 check --- check environment"
	echo "  $0 install --- check & run scripts to install"
}

case $1 in
	check)
		print_hello $1
		;;
	install)
		print_hello $1
		build_all
		;;
	*)
		print_help
		;;
esac



