OBJ=objet
SRC=src
INC=include

all: testVersion testOffset testCommit

testVersion: ${OBJ}/testVersion.o ${OBJ}/version.o 
	gcc -o testVersion ${OBJ}/testVersion.o ${OBJ}/version.o
	
${OBJ}/testVersion.o: ${SRC}/testVersion.c
	gcc -c -o ${OBJ}/testVersion.o ${SRC}/testVersion.c -Wall
	
${OBJ}/version.o: ${SRC}/version.c 
	gcc -c -o ${OBJ}/version.o ${SRC}/version.c -Wall

testOffset: ${OBJ}/testOffset.o ${OBJ}/version.o ${OBJ}/commit.o 
	gcc -o testOffset ${OBJ}/testOffset.o ${OBJ}/version.o ${OBJ}/commit.o

${OBJ}/testOffset.o: ${SRC}/testOffset.c
	gcc -c -o ${OBJ}/testOffset.o ${SRC}/testOffset.c -Wall
	
${OBJ}/commit.o: ${SRC}/commit.c 
	gcc -c -o ${OBJ}/commit.o ${SRC}/commit.c -Wall

testCommit: ${OBJ}/testCommit.o ${OBJ}/commit.o ${OBJ}/version.o
	gcc -o testCommit ${OBJ}/testCommit.o ${OBJ}/commit.o ${OBJ}/version.o -L -lcommit

${OBJ}/testCommit.o: ${SRC}/testCommit.c 
	gcc -c -o ${OBJ}/testCommit.o ${SRC}/testCommit.c -Wall


clean:
	rm -rf ${OBJ}/*.o testVersion testOffset testCommit
