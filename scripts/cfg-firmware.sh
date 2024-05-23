#!/bin/bash

dealUserInputCfg() {
	local confarray
	IFS='-' read -a confarray < <(echo $1)
	local outfile=$2
	local oldCfgFile=$old_config_file

	if [ -z ${oldCfgFile} ]; then
		oldCfgFile="./.config.old"
	fi

	([ -f ${oldCfgFile} ] || touch ${oldCfgFile}) && cat /dev/null > ${oldCfgFile}
	([ -f ./tmpFile ] || touch ./tmpFile) && cat /dev/null > ./tmpFile

	for element in "${confarray[@]}"; do
		local field1=$(cut -d':' -f1 <(echo "${element}"))
		local field2=$(cut -d':' -f2 <(echo "${element}"))
		read -a arr_field2 < <(echo ${field2})
		
		while true; do
			echo "### Select ${field1} :"
			gawk '
			{
				split($0,fields)
				printf "	"
				printf "[ " > "./tmpFile"
				for(element in fields) {
					printf "%d-%s  ",element-1,fields[element]
					printf "%d_%s  ",element-1,fields[element] >> "./tmpFile"
				}
				printf "\n?#"
				printf "]" >> "./tmpFile"
			}
			' <(echo ${field2})
			
			local selectNum
			if [ ${#arr_field2[@]} -eq 1 ]; then
				selectNum=0
				printf "0\n"
			else
				read selectNum
			fi
			
			if [ -z $selectNum ] || [ $selectNum -lt 0 ] || [ $selectNum -gt $((${#arr_field2[@]}-1)) ]; then
				echo "[WARNING] invalid selection. try again"
				continue
			else
				break
			fi
		done
		
		echo "CONFIG_${field1^^}_${arr_field2[${selectNum}]^^}=y" >> ${outfile}
		
		echo "${selectNum} ${field1} $(cat ./tmpFile)" >> ${oldCfgFile}
		
	done

	[ -f ./tmpFile ] && rm -f ./tmpFile
}

buildHeaderFile()
{
	local confarray
	IFS='-' read -a confarray < <(echo $1)
	local outfile=$2
	
	local title=__${outfile##*/}__
	title=${title/./_}
	title=${title^^}
	
	echo "#ifndef ${title}" >> ${outfile}
	echo "#define ${title}" >> ${outfile}
	echo >> ${outfile}
	
	for element in "${confarray[@]}"; do
		local leftField=$(cut -d'=' -f1 < <(echo ${element}))
		local rightField=$(cut -d'=' -f2 < <(echo ${element}))
		
		if [ $rightField == "y" ]; then
			echo "#define ${leftField^^}" >> ${outfile}
		fi
	done
	
	echo >> ${outfile}
	echo "#endif //${title}" >> ${outfile}
}

buildMkCfgFile() {
	local confarray
	IFS='-' read -a confarray < <(echo $1)
	local outfile=$2
	
	printf "%s\n" ${confarray[@]^^} >> ${outfile}
}

dealOldCfg() {
	local confarray
	IFS='-' read -a confarray < <(echo $1)
	local outfile=$2
	local oldCfgFile=$old_config_file
	
	if [ -z ${oldCfgFile} ]; then
		oldCfgFile="./.config.old"
	fi
	
	[ ! -f ${oldCfgFile} ] && echo "[ERROR] file ${oldCfgFile} not exist... " && exit 1
	
	local cout=1
	for element in "${confarray[@]}"; do
		local field1=$(cut -d':' -f1 <(echo "${element}"))
		local field2=$(cut -d':' -f2 <(echo "${element}"))
		read -a arr_field2 < <(echo ${field2})
		
		local selectNum=$(cut -d' ' -f1 < <(gawk -v _line="${cout}" 'NR==_line' ${oldCfgFile}))
		echo "CONFIG_${field1^^}_${arr_field2[${selectNum}]^^}=y" >> ${outfile}
		cout=$((${cout}+1))
	done
}

has_o_option="false"
has_c_option="false"
has_h_option="false"
has_m_option="false" 

while getopts ":ochm" opt; do
	case $opt in
		o)
			has_o_option="true"
			;;
		c)
			has_c_option="true"
			;;
		h)
			has_h_option="true"
			;;
		m)
			has_m_option="true"
			;;
		\?)
			echo "invalid option: -${OPTARG}" >&2
			exit 1
			;;
	esac
done

infile=${2}
outfile=${3}

if [ ! -f ${infile} ]; then
	echo "[ERROR] config file ${infile} not exist..."
	exit 1
fi

([ -f ${outfile} ] || touch ${outfile}) && cat /dev/null > ${outfile}

readarray -t confarray < ${infile}

callfunc=""
if [ $has_c_option == "true" ]; then
	callfunc=dealUserInputCfg
elif [ $has_h_option == "true" ]; then
	callfunc=buildHeaderFile
elif [ $has_m_option == "true" ]; then
	callfunc=buildMkCfgFile
elif [ $has_o_option == "true" ]; then
	callfunc=dealOldCfg
fi

[ ! -z ${callfunc} ] && ${callfunc} "$(printf "%s-" "${confarray[@]}")" "${outfile}"