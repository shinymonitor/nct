default:
	gcc nct.c -o nct -Wall -Wextra -Werror -pedantic -O3
install:
	gcc nct.c -o nct -Wall -Wextra -Werror -pedantic -O3
	cp nct /usr/bin/nct