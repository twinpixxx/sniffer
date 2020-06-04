#!/bin/sh

sigint()
{
	clear
    printf 'Stopped by user. Bye!\n'
    exit 1
}

main()
{
    clear
    printf "\t\t\tWelcome to Sniffer Course Work\n\n"

    while [ 1 ]; do
	printf "Select option: \n\n"
	printf "1 : Build Project\n"
	printf "2 : Launch Project\n"
	printf "3 : Remove Object files\n"
	printf "4 : Rebuild\n"
	printf "0 : Exit\n"

	printf "\nYou choose: "
	read option
	
	if [ $option = 0 ]
	then
	    exit
	fi
	if [ $option -ge 1 ] && [ $option -le 4 ]
	then
	    if [ $option = 1 ]
	    then
			make "sniffer"
			clear
	    fi
	    if [ $option = 2 ]
	    then
	    	clear
			sudo "./sniffer"
	    fi
	    if [ $option = 3 ]
	    then
			make clean
	    fi
	    if [ $option = 4 ]
	    then
		   	clear
			make re
	    fi	    
	else
	    printf "This option does not exist\n"
	fi
    done
}

trap 'sigint' 2

main
