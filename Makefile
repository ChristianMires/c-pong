#OBJS specifies which files to compile as part of the project
OBJS = pong.c

#OBJ_NAME specifies the name of our executable
OBJ_NAME = pong

#This is the target that compiles our executable
all : $(OBJS)
	gcc $(OBJS) -w -lSDL2 -lSDL2_image -lSDL2_ttf -g -o $(OBJ_NAME)
