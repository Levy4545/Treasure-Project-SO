all: treasure_manager treasure_hub calculate_score_exec

treasure_manager: treasure_manager.c treasure.c treasure.h
	gcc -Wall -o treasure_manager treasure_manager.c treasure.c

treasure_hub: treasure_hub.c
	gcc -Wall -o treasure_hub treasure_hub.c

calculate_score_exec: calculate_score_exec.c
	gcc -Wall -o calculate_score_exec calculate_score_exec.c

clean:
	rm -f treasure_manager treasure_hub calculate_score_exec command.txt
