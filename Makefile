
LIB=conlib.c
CFLAG=-Wall -g3

greedy_snake:
	gcc main.c game.c z.c $(LIB) $(CFLAG) -o greedy_snake
	-strip greedy_snake.exe

clean:
	-rm *.exe *.o

.PHONE:
	greedy_snake
