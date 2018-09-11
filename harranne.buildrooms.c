/* Anne Harris (harranne@oregonstate.edu)
 * CS 344 - 400
 * Program 2 - adventure
 * Description: This file will create a series of files that hold descriptions
 * of the in-game rooms and how rooms are connected
 */

/* Include Statements */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

//constant integer for number of rooms
const int NUM = 7;

/* Define Room struct */
struct Room
{
	/*name*/
	char* name;
	/*room type*/
	char* type;
	/*array of outgoing connections */
	struct Room* connectingOut[6];	
	/* number of connections */
	int numOutConnections;
	/* id */
	int id;
};

/* pseudocode modeled from 2.2 Program Outlining in Program 2 course page */

/* IsGraphFull FUNCTION */
/* Parameters: pointer to array of Rooms
 * Returns true if all rooms have 3 to 6 outbound connections, returns
 * false otherwise */
bool IsGraphFull(struct Room *roomsIn){
	//check each rooms number of connections
	int i = 0;
	for(i; i < NUM; i++){
		int tempConnections = roomsIn[i].numOutConnections;
		if(tempConnections < 3){
			//return false if theres a room with less than 3 connections
			return false;
		}
	}
	//return true if each room has at least 3 connections
	return true;

}

/* GetRandomRoom FUNCTION */
/* Parameters: none
 * Returns random room, doesn't validate if connection can be added */
int GetRandomRoom(){
	/* generate random number between 0 and 6 */
	int upper = 6;
	int lower = 0;
	/* fromula from Geeks for Geeks article titled "generating random number
 	* in a range in C */
	int randRoomNum = (rand()%(upper-lower + 1)) + lower;
	
	/* return room from random number*/ 
	return randRoomNum;
}

/* CanAddConnectionFrom FUNCTION */
/* Parameters: pointer to array of rooms, int representing room index to check
 * returns true if a connection can be added from room x (< 6 outbound connections)
 * returns false otherwise*/
bool CanAddConnectionFrom(struct Room *roomsIn, int x){
	//check connections for room at index x
	int num = roomsIn[x].numOutConnections;
	if(num < 6){
		return true;
	}
	else{
		return false;
	}
}

/* ConnectionAlreadyExists FUNCTION */
/* Parameters: pointer to array of Rooms, two ints to represent the index of the room numbers
 * Returns true if a connection from Room x to Room y already exists, false otherwise */
bool ConnectionAlreadyExists(struct Room *roomsIn, int x, int y){
	//store id of room to connect to
	int yID = roomsIn[y].id;
	int i = 0;
	//compare id's to rooms current connecitons
	for(i; i < roomsIn[x].numOutConnections; i++){
		int zID = roomsIn[x].connectingOut[i]->id;
		if(zID == yID){
			return true;
		}
	}
	return false;
}

/* ConnectRoom FUNCTION */
/* Parameters: pointer to array of Rooms, int representing the index of two rooms to connect
 * Connects Roomx x and y together, doesn't check if this connection is valid */
void ConnectRoom(struct Room *roomsIn, int x, int y){
	//create connection from x to y
	int xConnections = roomsIn[x].numOutConnections;
	roomsIn[x].connectingOut[xConnections] = &roomsIn[y];
	roomsIn[x].numOutConnections++;
}

/* IsSameRoom FUNCTION */
/* Parameters: pointer to array of rooms, int representing the index of two rooms to check
 * Returns true if Rooms x and y are the same Room, false otherwise */
bool IsSameRoom(struct Room *roomsIn, int x, int y){
	//get room id's and compare
	int xID = roomsIn[x].id;
	int yID = roomsIn[y].id;
	if(xID == yID){
		//same room
		return true;
	}
	else{
		return false;
	}
}

/* AddRandomConnection FUNCTION */
/* Parameters: pointer to array of rooms
 * Adds a random, valid outbund connection from a Room to another Room */
void AddRandomConnection(struct Room *roomsIn){
	//index of two random rooms
	int a;
	int b;
	while(true){
		//generate random room
		a = GetRandomRoom();
		if(CanAddConnectionFrom(roomsIn, a) == true)
			break;
	}
	do{
		b = GetRandomRoom();
	}
	while(CanAddConnectionFrom(roomsIn, b) == false || IsSameRoom(roomsIn, a, b) == true || ConnectionAlreadyExists(roomsIn, a, b) == true);
	//add connections both ways
	ConnectRoom(roomsIn, a, b); 
	ConnectRoom(roomsIn, b, a); 

}

/* RoomsToFiles FUNCTION */
/* Parameters: pointer to array of rooms
 * no return 
 * Write rooms to files */
void RoomsToFiles(struct Room *roomsIn){
	//create directory
	char directory[100];
	char *directoryName = "harranne.rooms.";
	//get process id
	int pid = getpid();
	sprintf(directory, "%s%d",directoryName, pid);
	int result = mkdir(directory, 0755);
	
	char *getName;
	// create files for each room
	if(result == 0){
		//loop through rooms
		int i = 0;
		for(i; i < NUM; i++){
			//create path and postfix name with "_room"
			getName = calloc(35, sizeof(char));
			strcpy(getName, directory);
			strcat(getName, "/");
			strcat(getName, roomsIn[i].name);
			strcat(getName, "_room");
			//create file
			FILE *newFile;
			newFile = fopen(getName, "w");
			if(newFile != NULL){
				//write room name
				fprintf(newFile, "ROOM NAME: %s\n", roomsIn[i].name);
				//write connections
				int j = 0;
				for(j; j < roomsIn[i].numOutConnections; j++){
					fprintf(newFile, "CONNECTION %d: %s\n", (j+1), roomsIn[i].connectingOut[j]->name);
				}
				//write room type
				fprintf(newFile, "ROOM TYPE: %s\n", roomsIn[i].type);
			}
			//close file
			fclose(newFile);
			free(getName);
		}
	}
}

/* Functions used to shuffle array to generate random rooms */
/* Implementation from geeks for geeks article "Shuffle a given array" */
void swap(int *a, int *b){
	int temp = *a;
	*a = *b;
	*b = temp;
}
void shuffle(int list[], int n){
	
	srand(time(NULL));
	int i = n-1;
	for(i ; i > 0; i--){
		int j = rand() % (i+1);
		swap(&list[i], &list[j]);
	}
}

/***************************************************************************
 * MAIN FUNCTION
 * ************************************************************************/
int main(){
	//Initalize Rooms
	/* Array of Room names */
	char *names[] = {
		"Wntrfll",
		"Drgnstn",
		"CstrlyRk",
		"Eyrie",
		"KngsLnd",
		"Essos",
		"RvrRun",
		"HrdHome",
		"Dorne",
		"Wstros"
	};

	//delcare array of structs
	struct Room usedRooms[NUM];
	//list of nums
	int numList[10] = {0,1,2,3,4,5,6,7,8,9};
	//shuffle list of numbers
	shuffle(numList, 10);
	
	//initalize rooms
	int i = 0;
	for(i; i < NUM; i++){
		//give room ID
		usedRooms[i].id = i;
		//give room Name
		usedRooms[i].name = calloc(9, sizeof(char));
		strcpy(usedRooms[i].name, names[numList[i]]);
		//set connections
		usedRooms[i].numOutConnections = 0;
		//give room type
		usedRooms[i].type = calloc(11, sizeof(char));
		if(i == 0){	//first room
			strcpy(usedRooms[i].type, "START_ROOM");
		}
		else if(i == NUM-1){	//last room
			strcpy(usedRooms[i].type, "END_ROOM");
		}
		else{
			strcpy(usedRooms[i].type, "MID_ROOM");
		}
	}	
	
	//reset time
	srand(time(0));
	//fill room graphh and add connections between rooms
	while(IsGraphFull(usedRooms) == false){
		AddRandomConnection(usedRooms);
	}

	//print statement used for debugging
	/*
	int a = 0;
	for(a; a < NUM; a++)
	{
		printf("the rooms connected to (%s/%d) are:\n", usedRooms[a].name, usedRooms[a].id);

		int b = 0;
		for(b; b < usedRooms[a].numOutConnections; b++){
			printf("   (%s/%d)\n", usedRooms[a].connectingOut[b]->name, usedRooms[a].connectingOut[b]->id);
		}
	}
	*/

	//create new directory and write rooms to files
	RoomsToFiles(usedRooms);

	int x = 0;
	for(x; x < NUM; x++){
		free(usedRooms[x].name);
		free(usedRooms[x].type);
	}

	//end program
	return 0;

}
