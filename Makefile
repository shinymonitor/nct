default:
	gcc nct.c -o nct -Wall -Wextra -Werror -O3
install:
	gcc nct.c -o nct -Wall -Wextra -Werror -O3
	cp nct /usr/bin/nct