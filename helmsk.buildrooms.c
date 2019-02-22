/***********************************************************
* Author:          Kelsey Helms
        * Date Created:    February 5, 2017
* Filename:        helmsk.buildrooms.c
        *
        * Overview:
* This program builds a maze of rooms with a start room,
* end room, and middle connecting rooms, and writes them
        * out to a directory.
************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

#define MIN_CONNECTIONS 3
#define MAX_CONNECTIONS 6
#define ROOMS_IN_GAME 7


/* ************************************************************************
	                  Global Variables
 ************************************************************************ */

const char *roomNames[] = {    //prepicked room names based on Seattle neighborhoods and landscapes!
        "DennyDen",
        "BallardBurrow",
        "FremontForest",
        "MontlakeMountains",
        "PioneerPlains",
        "ColumbiaCaverns",
        "SodoSwamp",
        "LeschiLake",
        "RavennaRidge",
        "WallingfordWoods"
};

struct Room {
    const char *name;    //name of room
    int maxConnections;    //number of connecting rooms
    const char *roomType;    //start, mid, or end room
    struct Room *connectingRooms[MAX_CONNECTIONS];    //holds connecting rooms
    int currConnections;    //counts connected rooms as they are added
};


/* ************************************************************************
	                 Function Prototypes
 ************************************************************************ */

void createDirectory();
void createArrays(int *rooms, int *connections);
void createRooms(int *rooms, int *connections, struct Room **roomArray);
void createConnections(struct Room **roomArray);
void writeFile(struct Room **roomArray);


/* ************************************************************************
	                     Functions
 ************************************************************************ */

/***********************************************************
 * main: calls functions to create rooms.
 *
 * parameters: none.
 * returns: exit int.
 ***********************************************************/

int main() {
    int rooms[ROOMS_IN_GAME];    //array of numbers that connect up to room names
    int connections[ROOMS_IN_GAME];    //array of numbers representing room connection amounts
    struct Room *roomArray[ROOMS_IN_GAME];    //array that holds rooms

    srand(time(0));    //seeds to generate randomness of chosen rooms and number of connecting rooms

    createDirectory();    //create the directory to hold room files

    createArrays(rooms, connections);    //create arrays to hold room name indices and amount of connections

    createRooms(rooms, connections, roomArray);    //create the rooms and add to room array

    createConnections(roomArray);    //create room connections

    writeFile(roomArray);    //write room information to files

    return 0;    //success!
}


/***********************************************************
 * createDirectory: creates the directory to store room
 * files and changes into that directory.
 *
 * parameters: none.
 * returns: none.
 ***********************************************************/

void createDirectory() {
    int pid = getpid();    //get process id for unique directory name
    char prefix[] = "helmsk.rooms.";    //onid id prefix
    char dirName[0];    //directory name
    sprintf(dirName, "%s%d", prefix, pid);    //adds process id to prefix to get the directory name

    mkdir(dirName, 0755);    //create the directory

    chdir(dirName);    //change into new directory for easy writing
}


/***********************************************************
 * createArrays: creates arrays containing randomly
 * generated numbers to match up to rooms and connections.
 *
 * parameters: 2 int arrays.
 * returns: none.
 ***********************************************************/

void createArrays(int *rooms, int *connections) {

    int index = 0;

    do {
        int name = rand() % 10;    //get a random number as index to get name from room names
        int number = 0;
        int selected = 0;    //make sure not to select the same room twice
        int i;

        for (i = 0; i < index; i++) {    //for all room name indices already added to array
            if (rooms[i] == name)    //check that name at that index isn't already added to array of room name indices
                selected = 1;    //if it has been, mark as selected
        }

        if (selected == 0) {    //if not already selected
            rooms[index] = name;    //add that name index to array holding room name indices
            number = (rand() % (MAX_CONNECTIONS - MIN_CONNECTIONS + 1)) + MIN_CONNECTIONS;    //get random amount of connections
            connections[index] = number;    //add that to array holding amount of connections
            index++;    //count rooms completed
        }

    } while (index < ROOMS_IN_GAME);    //continue until all rooms for game have been chosen

}


/***********************************************************
 * createRooms: creates a room struct that holds information
 * for each room using the previously created int arrays.
 *
 * parameters: 2 int arrays, 1 struct Room array.
 * returns: none.
 ***********************************************************/

void createRooms(int *rooms, int *connections, struct Room **roomArray) {

    int i;
    for (i = 0; i < ROOMS_IN_GAME; i++) {    //for all rooms in the game
        struct Room *gameRoom = malloc(sizeof(struct Room));    //creates a struct to hold room info
        gameRoom->name = roomNames[rooms[i]];    //grabs room name from roomName array using array holding room name indices
        gameRoom->maxConnections = connections[i];    //sets max connections room can have using array holding connection amounts
        gameRoom->currConnections = 0;    //sets current connections to 0

        if (i == 0)
            gameRoom->roomType = "START_ROOM";    //set first room as start room
        else if (i == (ROOMS_IN_GAME - 1))
            gameRoom->roomType = "END_ROOM";    //set last room as end room
        else
            gameRoom->roomType = "MID_ROOM";    //all others are mid rooms

        roomArray[i] = gameRoom;    //add room to room array
    }
}


/***********************************************************
 * createConnections: creates the connections between each
 * room using the amount of connections given to each room,
 * making sure that connection goes both ways, and stores
 * connections in roomArray.
 *
 * parameters: struct Room array.
 * returns: none.
 ***********************************************************/

void createConnections(struct Room **roomArray) {
    int i, j;

    for (i = 0; i < ROOMS_IN_GAME; i++) {    //create connections for all rooms
        if (roomArray[i]->maxConnections == 6) {    //if room has 6 connections
            for (j = i + 1; j < ROOMS_IN_GAME; j++) {    //connect to all other rooms
                roomArray[i]->connectingRooms[roomArray[i]->currConnections] = roomArray[j];    //add room j to room i connections
                roomArray[j]->connectingRooms[roomArray[j]->currConnections] = roomArray[i];    //add room i to room j connections
                roomArray[i]->currConnections++;    //increase current connection count
                roomArray[j]->currConnections++;
            }
        }

        else {
            if (roomArray[i]->maxConnections > roomArray[i]->currConnections) {    //if room has less than 6 connections
                for (j = i + 1; j < ROOMS_IN_GAME; j++) {    //for all other rooms
                    if (roomArray[j]->maxConnections > roomArray[j]->currConnections) {    //if room j can still make more connections
                        roomArray[i]->connectingRooms[roomArray[i]->currConnections] = roomArray[j];    //add room j to room i connections
                        roomArray[j]->connectingRooms[roomArray[j]->currConnections] = roomArray[i];    //add room i to room j connections
                        roomArray[i]->currConnections++;    //increase current connection count
                        roomArray[j]->currConnections++;
                    }
                }

                if (roomArray[i]->maxConnections < roomArray[i]->currConnections)    //if there aren't enough rooms to connect to
                    roomArray[i]->maxConnections = roomArray[i]->currConnections;    //adjust amount of connections
            }
        }
    }

}


/***********************************************************
 * writeFile: writes room information into files in the
 * current directory.
 *
 * parameters: struct Room array.
 * returns: none.
 ***********************************************************/

void writeFile(struct Room **roomArray) {

    int i, j;
    const char *filename;

    for (i = 0; i < ROOMS_IN_GAME; i++) {    //for all rooms
        filename = roomArray[i]->name;    //set filename to room name

        FILE *file = fopen(filename, "w");    //open file

        if (!file)    //if file couldn't open
        {
            printf("Error opening room file");    //print error message and exit
            exit(1);
        }

        fprintf(file, "ROOM NAME: %s\n", roomArray[i]->name);    //print room name into file

        for (j = 0; j < roomArray[i]->currConnections; j++)    //for all connections
            fprintf(file, "CONNECTION %d: %s\n", j + 1, roomArray[i]->connectingRooms[j]->name);    //print name of connecting rooms to file

        fprintf(file, "ROOM TYPE: %s\n", roomArray[i]->roomType);    //print room type to file

        fclose(file);    //close file
    }
}