CC=gcc #Le compilateur à utiliser
TARGET=cassini #Nom du fichier à créer

all:
	$(CC) src/cassini.c -o $(TARGET) # Genere l'executable associé à src/cassini.c

clean:
	rm $(TARGET) # Permet de supprimer l'executable via : make clean
