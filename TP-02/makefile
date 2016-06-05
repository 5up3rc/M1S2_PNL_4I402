OBJ=objet
SRC=src
INC=include

#all: testVersion testOffset testCommit
all: testCommit testComment

#testVersion: ${OBJ}/testVersion.o ${OBJ}/version.o 
#	gcc -g -o testVersion ${OBJ}/testVersion.o ${OBJ}/version.o
	
#${OBJ}/testVersion.o: ${SRC}/testVersion.c
#	gcc -g -c -o ${OBJ}/testVersion.o ${SRC}/testVersion.c -Wall
	
${OBJ}/version.o: ${SRC}/version.c 
	gcc -g -c -o ${OBJ}/version.o ${SRC}/version.c -Wall

#testOffset: ${OBJ}/testOffset.o ${OBJ}/version.o ${OBJ}/commit.o 
#	gcc -g -o testOffset ${OBJ}/testOffset.o ${OBJ}/version.o ${OBJ}/commit.o

#${OBJ}/testOffset.o: ${SRC}/testOffset.c
#	gcc -g -c -o ${OBJ}/testOffset.o ${SRC}/testOffset.c -Wall
	
${OBJ}/commit.o: ${SRC}/commit.c 
	gcc -g -c -o ${OBJ}/commit.o ${SRC}/commit.c -Wall

testCommit: ${OBJ}/testCommit.o ${OBJ}/commit.o ${OBJ}/version.o
	gcc -g -o testCommit ${OBJ}/testCommit.o ${OBJ}/commit.o ${OBJ}/version.o -L -lcommit

${OBJ}/testCommit.o: ${SRC}/testCommit.c 
	gcc -g -c -o ${OBJ}/testCommit.o ${SRC}/testCommit.c -Wall
	
	
${OBJ}/comment.o: ${SRC}/comment.c 
	gcc -g -c -o ${OBJ}/comment.o ${SRC}/comment.c -Wall

testComment: ${OBJ}/testComment.o ${OBJ}/comment.o 
	gcc -g -o testComment ${OBJ}/testComment.o ${OBJ}/comment.o
	
${OBJ}/testComment.o: ${SRC}/testComment.c
	gcc -g -c -o ${OBJ}/testComment.o ${SRC}/testComment.c -Wall


clean:
	rm -rf ${OBJ}/*.o testCommit testComment #testVersion testOffset 
