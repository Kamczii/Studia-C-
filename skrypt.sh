#!/bin/bash
#podstawowe menu
komunikat(){
	echo > menu.txt
	echo "1. Dodaj słowo kluczowe:" >> menu.txt
	echo "2. Usuń słowo kluczowe:"	>> menu.txt
	echo "3. Wyświetl słowa kluczowe" >> menu.txt
	echo "4. Kiedy wyszukano słowo:" >> menu.txt
	echo "5. Odwiedzane witryny:" >> menu.txt
	echo "6. Data i słowo:"	>> menu.txt
	echo "7. Koniec" >> menu.txt
}
#wczytuje menu z pliku
readMenu(){
	komunikat
	k=0
	while read i;do
		menu[k]="$i"
		((k++))
	done < menu.txt
}
#sprząta iptables oraz zatrzymuje mitmdump.service
cleanup(){
	sudo iptables -t nat -F
	systemctl stop mitmdump.service
	> /tmp/skrypt/temp.txt
}
#inicjalizuje iptables oraz uruchamia mitmdump.service, tworzy plik tymczasowy
start(){
	systemctl start mitmdump.service
	sudo iptables -t nat -A OUTPUT -p tcp -m owner ! --uid-owner root --dport 80  -j REDIRECT --to-port 8080
	sudo iptables -t nat -A OUTPUT -p tcp -m owner ! --uid-owner root --dport 443  -j REDIRECT --to-port 8080
	mkdir -p /tmp/skrypt
	touch /tmp/skrypt/temp.txt
}
#dodaje słowo kluczowe do pliku
add(){
		echo -e $1 >> kluczowe.txt
}
#usuwa słowo kluczowe z pliku
delete(){
	grep -v $1 kluczowe.txt > temp.txt
	cp temp.txt kluczowe.txt
}
#wyswietla slowa kluczowe
print(){
	k=0
	keys=()
	while read i;do
		keys[k]="$i"
		((k++))
	done < kluczowe.txt
	zenity --list --column="Słowa" ${keys[@]}
}
#wyswietla odwiedzane witryny
print_urls(){
	cat /var/log/monitor.log | grep "GET http" |cut -d " " -f 4 | sort | uniq -c > /tmp/skrypt/temp.txt
	k=0
	urls=()
	while read i;do
		urls[k]="$(echo $i| cut -d ' ' -f 1) $(echo $i| cut -d ' ' -f 2)" #wczytuje ilość wizyt dla danego url
		((k++))
	done < /tmp/skrypt/temp.txt
	zenity --list --separator=" " --height 500 --width 600 --column="Wizyt" --column="URL"    ${urls[@]} 
}
#kiedy wyszukano dane słowo kluczowe
key(){
	cat /var/log/monitor.log | grep "GET http" |cut -d " " -f 1,4 | grep $1 > /tmp/skrypt/temp.txt
	k=0
	urls=()
	while read i;do
		urls[k]="$(echo $i| cut -d ' ' -f 1,2)"
		((k++))
	done < /tmp/skrypt/temp.txt
	zenity --list --height 500 --width 600 --column="Czas" --column "URL"  --separator=" "  ${urls[@]} 
}
#zaawansowane wyszukiwanie ktore bierze pod uwage date i slowo
search(){
		cat /var/log/monitor.log | grep "GET http" |cut -d " " -f 1,4 > /tmp/skrypt/temp.txt
		k=0
		results=()
		
		while read i;do
			line=$i
			dt="$(echo $i| cut -d ' ' -f 1)"
			dt="${dt:1:${#dt}-2}" #wyciaga date
			wd="$(echo $i| cut -d ' ' -f 2)" #wyciaga szukane slowo
			if [[ $1 != "" ]];then
				if [[ $(date +%s -d $dt) -lt $(date +%s -d $1) ]];then #porownuje na podstawie sekund
					continue;
				fi
			fi
			if [[ $2 != "" ]];then
				if [[ $(date +%s -d $dt) -gt $(date +%s -d $2) ]]; then
					continue;
				fi
			fi
			if [[ $3 != "" ]];then
				if [[ "$wd" != *"$3"* ]];then
					continue;
				fi
			fi
			
			
			urls[k]=$i #jesli przeszlo kryteria pasuja dodajemy do tablicy
			((k++))
		done < /tmp/skrypt/temp.txt
		zenity --list --height 500 --width 600 --column="Czas" --column "URL"  --separator=" "  ${urls[@]} 
}
#pomoc 
help(){
	echo "[-h] [-v] -- Śledzenie historii wyswietlanych stron

	gdzie:
		-h  pomoc
		-v  informacje o wersji
		
	1. Dodaj słowo kluczowe - dodaje slowo kluczowe do pliku
	2. Usuń słowo kluczowe - usuwa slowo kluczowe do pliku
	3. Wyświetl słowa kluczowe - wyswietla slowa kluczowe z pliku
	4. Kiedy wyszukano słowo - wyswietla daty kiedy wyszukano slowo
	5. Odwiedzane witryny - wyswietla odwiedzone witryny
	6. Data i słowo - wyszukuje fraze w podanym przedziale czasu
	7. Koniec - konczy program"
	
}
#jesli uruchomiono z opcjami, należy po nich opuscic skrypt
while getopts 'hv' c
do
	case $c in
		h) help && exit;;
		v) echo "Wersja: 1.0.0" && exit;;
	esac
done

start
trap cleanup EXIT

A=0
while [[ "$A" == 0 ]];do
	readMenu
	INPUT=$(zenity --list --column=Menu "${menu[@]}" --height 300)
	if [[ $INPUT == 1.* ]];then
		KEY=`zenity --entry --title "Dodaj słowo kluczowe" --text ""`
		if [[ $KEY != "" ]]; then
			add $KEY
		fi
	elif [[ $INPUT == 2.* ]];then
		DEL=`zenity --entry --title "Usuń słowo kluczowe:" --text ""`
		if [[ $DEL != "" ]]; then
			delete $DEL
		fi
	elif [[ $INPUT == 3.* ]];then
		print
	elif [[ $INPUT == 4.* ]];then
		INPUT=`zenity --entry --title "Szukane słowo:" --text ""`
		if [[ $INPUT != "" ]]; then
			key $INPUT
		fi
	elif [[ $INPUT == 5.* ]];then
		print_urls
	elif [[ $INPUT == 6.* ]];then
		SINCE=`zenity --entry --title "Data od" --text ""`
		TO=`zenity --entry --title "Data do" --text ""`
		WORD=`zenity --entry --title "Słowo" --text ""`
		search $SINCE $TO $WORD
	elif [[ $INPUT == 7.* ]]; then
		exit
	fi
done
