/***********************************************************
 * Author:          Kelsey Helms
 * Date Created:    February 5, 2017
 * Filename:        helmsk.adventure.c
 *
 * Overview:
 * This program uses rooms from helmsk.buildrooms files to
 * run a command line maze game.
 ************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <pthread.h>
#include <assert.h>

#define MIN_CONNECTIONS 3
#define MAX_CONNECTIONS 6
#define ROOMS_IN_GAME 7


/* ************************************************************************
	                  Global Variables
 ************************************************************************ */

pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;    //mutex lock

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

void *displayTime(void *arg);
void selectDirectory();
void readMaze(struct Room **);
struct Room *readFile(const char *filename);
void play(struct Room **rooms);


/* ************************************************************************
	                     Functions
 ************************************************************************ */

/***********************************************************
 * main: calls functions to play maze game.
 *
 * parameters: none.
 * returns: exit int.
 ***********************************************************/

int main() {
    struct Room *roomArray[ROOMS_IN_GAME];    //array that holds rooms

    selectDirectory();    //select most recent directory

    readMaze(roomArray);    //read in room maze information

    play(roomArray);    //play!

    return 0;    //success!
}


/***********************************************************
 * displayTime: writes the current time to a file.
 *
 * parameters: null pointer.
 * returns: none.
 ***********************************************************/

void *displayTime(void *arg) {
    pthread_mutex_lock(&myMutex);    //try to lock the lock, will not continue if it's already locked
    time_t rawtime;
    struct tm *timeinfo;    //time struct
    int hour;    //holds hour
    int minute;    //holds minute
    char *ampm;    //holds am or pm
    int iday;    //holds number day of week
    char *day;    //holds day name
    int imonth;    //holds number month of year
    char *month;    //hold month name
    int date;    //holds date of month
    int year;    //holds year

    time(&rawtime);
    timeinfo = localtime(&rawtime);    //get time info

    if (timeinfo->tm_hour > 12) {    //if it's after noon
        hour = timeinfo->tm_hour - 12;    //make it not 24 hour time
        ampm = "pm";    //set to pm
    } else if (timeinfo->tm_hour == 0) {    //if midnight
        hour = 12;    //set hour to midnight
        ampm = "am";    //set to am
    } else {    //otherwise
        hour = timeinfo->tm_hour;    //set hour
        ampm = "am";    //set to am
    }

    minute = timeinfo->tm_min;    //set minute

    iday = timeinfo->tm_wday;    //grab number day of week
    switch (iday) {    //set name of day based on number day of week
        case 0:
            day = "Sunday";
            break;
        case 1:
            day = "Monday";
            break;
        case 2:
            day = "Tuesday";
            break;
        case 3:
            day = "Wednesday";
            break;
        case 4:
            day = "Thursday";
            break;
        case 5:
            day = "Friday";
            break;
        default:
            day = "Saturday";
    }

    imonth = timeinfo->tm_mon;    //grab number month of year
    switch (imonth) {    //set name of month based on number month of year
        case 0:
            month = "January";
            break;
        case 1:
            month = "February";
            break;
        case 2:
            month = "March";
            break;
        case 3:
            month = "April";
            break;
        case 4:
            month = "May";
            break;
        case 5:
            month = "June";
            break;
        case 6:
            month = "July";
            break;
        case 7:
            month = "August";
            break;
        case 8:
            month = "September";
            break;
        case 9:
            month = "October";
            break;
        case 10:
            month = "November";
            break;
        default:
            month = "December";
    }

    date = timeinfo->tm_mday;    //set date of month

    year = timeinfo->tm_year + 1900;    //set year making sure to account for Y2K

    const char *filename;
    filename = "currentTime.txt";    //name file correctly
    FILE *file = fopen(filename, "w");    //open file for writing
    fprintf(file, "%d:%02d%s, %s, %s %d, %d\n\n", hour, minute, ampm, day, month, date, year);    //format information correctly, making sure minutes have two digits
    fclose(file);    //close file

    pthread_mutex_unlock(&myMutex);    //unlock the lock so rest of program can continue
}


/***********************************************************
 * selectDirectory: finds the room directory that was most
 * recently created and changes into that directory.
 *
 * parameters: none.
 * returns: none.
 ***********************************************************/

void selectDirectory() {
    int mostRecent = 0;
    char *recentFile;
    struct stat attr;
    DIR *d;
    struct dirent *dir;
    d = opendir(".");    //open current directory

    if (d) {    //if it opens
        while ((dir = readdir(d)) != NULL) {    //check all directories
            if (strstr(dir->d_name, "helmsk.rooms.") != NULL) {    //only look at directories with correct prefix
                stat(dir->d_name, &attr);    //get stat struct
                if (attr.st_mtime > mostRecent) {    //if created more recently than previous most recent
                    mostRecent = attr.st_mtime;    //update most recent
                    recentFile = dir->d_name;    //store most recent directory name
                }
            }
        }
        closedir(d);    //close directory
    }
    chdir(recentFile);    //change to most recent directory
}


/***********************************************************
 * readMaze: opens each room file and stores the information
 * into a struct room array for game play.
 *
 * parameters: struct Room array.
 * returns: none.
 ***********************************************************/

void readMaze(struct Room **rooms) {
    DIR *d;
    struct dirent *dir;
    d = opendir(".");    //open current directory (now in most recent room directory)
    int index = 0;
    const char *filename;


    if (d) {    //if it opens
        /*reads files in directory until null is incountered*/
        while ((dir = readdir(d)) != NULL) {    //check all files
            if (dir->d_name[0] != '.' &&    //if it starts with '.' it's a directory going back. don't open
                dir->d_name[0] != 'c')    //if it starts with 'c' it's the currentTime.txt from a previous run. don't open
            {
                filename = dir->d_name;    //sets filename to the filename currently being read in directory
                rooms[index] = readFile(filename);    //calls read file and puts returned struct in array
                index++;    //updates index counter
            }
        }
        closedir(d);    //close directory
    }
}


/***********************************************************
 * readFile: reads the current file and stores information
 * into a room struct.
 *
 * parameters: c-string.
 * returns: struct Room.
 ***********************************************************/

struct Room *readFile(const char *filename) {
    struct Room *gameRoom = malloc(sizeof(struct Room));    //create a room struct
    int number, count = 0;
    char line[100];
    char input[30];

    FILE *file;
    file = fopen(filename, "r");    //open file for reading

    if (!file) {    //if it doesn't open
        printf("Could not open %s", filename);    //print error message and exit
        exit(1);
    }

    while (fgets(line, sizeof(line), file) != NULL) {    //while a line is still able to read in
        if (strncmp(line, "ROOM NAME", 9) == 0) {    //if first 9 characters of line match "ROOM NAME"
            sscanf(line, "ROOM NAME: %s\n", input);    //scan in line taking room name as input
            char *temp = malloc(strlen(input));    //dynamically allocate char array
            strcpy(temp, input);    //copy to char array
            gameRoom->name = temp;    //use to add room name to room struct

        } else if (strncmp(line, "CONNECTION", 10) == 0) {    //if first 10 characters of line match "CONNECTION"
            gameRoom->connectingRooms[count] = malloc(sizeof(struct Room));    //add more space to connection room array
            sscanf(line, "CONNECTION %d: %s\n", &number, input);    //scan in line taking connecting room number and name as input
            char *temp = malloc(strlen(input));    //dynamically allocate char array
            strcpy(temp, input);    //copy to char array
            gameRoom->connectingRooms[count]->name = temp;    //use to add room name to connecting room array
            count++;    //increase connecting room count

        } else {    //otherwise
            sscanf(line, "ROOM TYPE: %s\n", input);    //scan in line taking room type as input
            char *temp = malloc(strlen(input));    //dynamicall allocate char array
            strcpy(temp, input);    //copy to char array
            gameRoom->roomType = temp;    //use to add room type
        }
    }

    fclose(file);    //close file
    gameRoom->currConnections = count;    //sets the number of connections room has

    return gameRoom;    //return the game room to add to room struct array
}


/***********************************************************
 * play: creates the interface for the game and lets the
 * user play.
 *
 * parameters: struct Room array.
 * returns: none.
 ***********************************************************/

void play(struct Room **rooms) {
    struct Room *currRoom;    //holds room information of room player is in
    int path[50];    //holds numbers that corresponds to indices in rooms
    int i, index, steps = 0;
    const char *nextRoom;    //holds next room name
    int resultCode;    //thread result code
    pthread_t myThread;    //thread name
    pthread_mutex_lock(&myMutex);    //try to lock the lock, will not continue if it's already locked

    for (i = 0; i < ROOMS_IN_GAME; i++) {    //search through all rooms
        if (strcmp(rooms[i]->roomType, "START_ROOM") == 0)    //find start room
            currRoom = rooms[i];    //make start room the current room
    }

    do {
        char input[30];
        int i, valid = 0;
        resultCode = pthread_create(&myThread, NULL, displayTime, NULL);    //create time thread

        do {
            valid = 0;    //check if input is valid
            printf("CURRENT LOCATION: %s\n", currRoom->name);    //print current location
            printf("POSSIBLE CONNECTIONS: ");    //print connections possible

            for (i = 0; i < currRoom->currConnections; i++) {    //for all current room's connections
                if ((i + 1) < currRoom->currConnections)
                    printf("%s, ", currRoom->connectingRooms[i]->name);    //print connecting room names with comma after
                else
                    printf("%s.\n", currRoom->connectingRooms[i]->name);    //print last connecting room name with period after
            }

            printf("WHERE TO? >");    //ask where to go
            fgets(input, 30, stdin);    //get input from user

            int last = strlen(input) - 1;    //check last char
            if (input[last] == '\n')    //if it was a newline
                input[last] = '\0';    //replace with null terminator

            for (i = 0; i < currRoom->currConnections; i++) {    //for all room connections
                if (strcmp(input, currRoom->connectingRooms[i]->name) == 0) {    //if input matches connecting room name
                    valid = 1;    //it's a valid choice
                    nextRoom = currRoom->connectingRooms[i]->name;    //make this the next room
                }
            }

            if (strcmp(input, "time") == 0) {    //if input was time
                pthread_mutex_unlock(&myMutex);    //unlock the lock! this allows the second thread (time thread) to run
                valid = 2;    //it's a valid choice
                resultCode = pthread_join(myThread, NULL);    //don't continue until second thread has completed
                pthread_mutex_lock(&myMutex);    //try to lock the lock, will not continue if it's already locked
            }

            if (valid == 0)    //if choice isn't valid, print error message and reloop
                printf("\nHUH? I DON'T UNDERSTAND THAT ROOM.  TRY AGAIN\n");

            printf("\n");
        } while (valid == 0);    //continue loop until choice is valid

        if (valid == 1) {    //if choice was connecting room
            steps++;    //increase step count

            for (i = 0; i < ROOMS_IN_GAME; i++) {    //for all rooms
                if (strcmp(rooms[i]->name, nextRoom) == 0) {    //when next room matches a name in room struct array
                    currRoom = rooms[i];    //set current room
                    path[steps - 1] = i;    //add room index to the path to print later
                }
            }
        }

        if (valid == 2) {    //if choice was time
            int c;
            FILE *file = fopen("currentTime.txt", "r");    //open currentTime file
            if (file) {    //if it opens
                while ((c = getc(file)) != EOF)    //while the end of file hasn't been reached
                    putchar(c);    //print each char
                fclose(file);    //close file
            }
        }

    } while ((strcmp(currRoom->roomType, "END_ROOM") != 0) && (steps < 50));    //continue looping until end room is reached or path array is full (50 steps)

    if (steps == 50) {    //if path array is full (50 steps)
        printf("IT TOOK YOU 50 STEPS AND YOU STILL COULDN'T SOLVE IT... SAD!\n");    //print fail message and exit
        return;
    }

    if (strcmp(currRoom->roomType, "END_ROOM") == 0) {    //if end room is reached
        printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");    //print congrats message
        printf("YOU TOOK %d STEPS.  YOUR PATH TO VICTORY WAS:\n", steps);    //print amount of steps

        for (i = 0; i < steps; i++) {    //for each step
            index = path[i];    //get index of room name from path
            printf("%s\n", rooms[index]->name);    //print room name along path taken
        }
    }
    resultCode = pthread_cancel(myThread);    //cancel second thread
    pthread_mutex_destroy(&myMutex);    //destroy lock
}
