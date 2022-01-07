CC=gcc #Le compilateur à utiliser
TARGET=cassini #Nom du fichier à créer

all:
	$(CC) -o $(TARGET) -I -Wall -g src/cassini.c src/commandline.c src/read-pipe.c src/timing-text-io.c src/write-pipe.c src/task.c #Genere l'executable associé à src/cassini.c
	$(CC) -o saturnd -I -Wall -g src/saturnd.c src/commandline.c src/read-pipe.c src/timing-text-io.c src/write-pipe.c src/task.c
distclean:
	rm $(TARGET)
	rm saturnd
