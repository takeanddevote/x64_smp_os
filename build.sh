#!/bin/bash
gen_clang_json_file() {
    local cmd=$1
    if [[ $configed == "true" ]]; then
        rm compile_commands.json
    else
        mv compile_commands.json compile_commands_bk.json
    fi
    bear make -j16
    if [[ $configed != "true" ]]; then
        jq -s 'add' compile_commands_bk.json compile_commands.json > compile_commands_final.json	 # 实现增量编译
        rm compile_commands_bk.json compile_commands.json
        mv compile_commands_final.json compile_commands.json
    fi
}


has_c_option="false"

while getopts ":c" opt; do
	case $opt in
		c)
			has_c_option="true"
			;;
		\?)
			echo "invalid option: -${OPTARG}" >&2
			exit 1
			;;
	esac
done

if [[ $has_c_option == "true" ]]; then
    make config
    configed="true"

    if [ $? != 0 ]; then
        echo "[ERROR] config fail."
        exit 1
    fi
fi


make buildInc buildIncRoot="netstack" 
gen_clang_json_file "make -j16"

make run