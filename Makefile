nct: nct.c build.h build.c embed.py
	python3 embed.py  
	gcc nct.c -o nct -Wall -Wextra -Werror -O3
install: nct
	cp nct /usr/bin/nct
