/* Anne Harris (harranne@oregonstate.edu
 * CS344 - 400
 * Program 2 - adventure
 * This file provides an interface for playing the game using the most recently generated room files.
 * This program will begin at the start room, display the connected rooms and prompt the user to 
 * choose the next room. The user will continue until it reaches the end room. This program also
 * utalizes mutexes and threads and the user can type "time" to get the current time
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>

//initalize mutex globally
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
//variable to make sure threads are working
int result_code; 

/***** TIME FUNCTION *******/
//Displays current time to screen
//source: www.turorialspoint.com/c_standard_library/c_function_strftime.htm
void* writeTime(){
	pthread_mutex_lock(&myMutex);
	
	//un formatted time
	time_t rawTime;
	struct tm *timeinfo;

	time(&rawTime);
	timeinfo = localtime(&rawTime);
	
	char displayTime[50];

	strftime(displayTime, 50, "%I:%M%P, %A, %B %d, %Y", timeinfo);
	printf("  %s\n\n", displayTime);

	//write time to file
	char timeStamp[16] = "currentTime.txt";	//file name
	FILE *timeFile;
	timeFile = fopen(timeStamp, "w");
	if(timeFile != NULL){
		fprintf(timeFile, displayTime);
	}
	//closefile
	fclose(timeFile);

	return NULL;
}

/******** MAIN FUNCTION *********/
//Drives the program by opening current directory, finding most recent room files, 
//and directing the game
int main(){
	//declare mutex/thread realated variables
	pthread_mutex_lock(&myMutex);
	pthread_t myThreadID;
	//create second thread
	result_code = pthread_create(&myThreadID, NULL, writeTime, NULL);
	assert(0 == result_code); 
	
	//copy of room names
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

	//reference to course page 2.4 - manipulating directories
	// get most current directory and open it
	int newestDirTime = -1; 
	char targetDirPrefix[35] = "harranne.rooms.";
	char newestDirName[256]; //holds name of newest dir created
	memset(newestDirName, '\0', sizeof(newestDirName));

	DIR * dirToCheck; //directory we're starting in 
	struct dirent *fileInDir; //holds the current subdir of the starting dir
	struct stat dirAttributes; //holds the information we've gained about subdir

	dirToCheck = opendir("."); //open up the directory this program was run in

	if(dirToCheck > 0){
		while((fileInDir = readdir(dirToCheck)) != NULL){ //check each directory
			//if entry has prefix
			if(strstr(fileInDir->d_name, targetDirPrefix) != NULL){ 
				//get attributes of the entry
				stat(fileInDir->d_name, &dirAttributes);
				
				//if this time is more recent
				if((int)dirAttributes.st_mtime > newestDirTime){
					newestDirTime = (int)dirAttributes.st_mtime;
					memset(newestDirName, '\0', sizeof(newestDirName));
					strcpy(newestDirName, fileInDir->d_name);
				}
			}
		}
	}

	//open newest directory 
	DIR * usedDir;
	usedDir = opendir(newestDirName);
	
	//display message and quit if it cannot open
	if(usedDir == NULL){
		printf("cannot open %s\n", newestDirName);
		return 0;
	}

	//file to read
	FILE *fileToRead;	
	struct dirent *filesInDir;
	//all rooms have the postfix _room, look for these files
	char postfix[6] = "_room";
	//buffer to store line from file
	char *buff;
	size_t buffsize = 256;
	buff = calloc(256, sizeof(char));

	//variables for rooms when reading in file
	//keep track of which room is the start and end room
	int startRoom;
	int endRoom;
	// array of numbers referencing the rooms used
	int fileRooms[7];

	// connections for each room
	int room1Connections[6];
	int room2Connections[6];
	int room3Connections[6];
	int room4Connections[6];
	int room5Connections[6];
	int room6Connections[6];
	int room7Connections[6];

	//number of connections for each file
	int numConnections1 = 0;
	int numConnections2 = 0;
	int numConnections3 = 0;
	int numConnections4 = 0;
	int numConnections5 = 0;
	int numConnections6 = 0;
	int numConnections7 = 0;

	errno = 0; //testing for errors
	char *roomPath;	//path to the room files
	roomPath = calloc(256, sizeof(char));
	char *tempName;	//temporary room name
	tempName = calloc(9, sizeof(char));
	int nameCmp; //variable for strcmp results
	int index = 0; //count for rooms in files
	char *tempType; //temporary room type for comparison
	tempType = calloc(11, sizeof(char));
	char roomName[12] = "ROOM NAME: "; //used to locate line in file
	char roomType[12] = "ROOM TYPE: "; //used to locate line in file
	char connections[15] = "CONNECTION "; //used to locate line in file

	//read all files to find start room
	if(newestDirName > 0){
		//read directory
		while((filesInDir = readdir(usedDir)) != NULL){
				//if files have postfix "_room"
				if(strstr(filesInDir->d_name, postfix) != NULL){
					//store path information
					memset(roomPath, '\0', sizeof(roomPath));
					strcpy(roomPath, newestDirName);
					strcat(roomPath, "/");
					strcat(roomPath, filesInDir->d_name);
					//open file
					fileToRead = fopen(roomPath, "r");
					//display error number and quit if file cannot be opened
					if(fileToRead == NULL){
						printf("Cannot open file, error %d\n", errno);
						return 0;
					}
					memset(buff, '\0', sizeof(buff));
				
					//get each line
					while(getline(&buff, &buffsize, fileToRead) != -1){
						//store room name
						if(strstr(buff, roomName) != NULL){
							//get JUST the room name
							//start index after "ROOM NAME: "
							memset(tempName, '\0', sizeof(tempName));
							strncpy(tempName, buff+11, strlen(buff)-11);
							//print statemenet for debugging
							//printf("TEMPNAME: %s\n", tempName);
							//find matching name in names array
							int b = 0;
							for(b; b < 10; b++){
								nameCmp = strncmp(names[b], tempName, 5);
								//if name matches, store that index in fileRooms array
								if(nameCmp == 0){
									fileRooms[index] = b;
									index++; //increment index
								}
							}
						}
						//store room type
						if(strstr(buff, roomType) != NULL){
							//get just the room type
							//start index after "ROOM TYPE: "
							memset(tempType, '\0', sizeof(tempType));
							strncpy(tempType, buff+11, strlen(buff)-11);
							//find the start room
							if(strncmp(tempType, "START_ROOM", 9) == 0){
								startRoom = index-1; //current location minus 1 as index was incremented
							
							}
							//end room
							else if(strncmp(tempType, "END_ROOM", 7) == 0){
								endRoom = index-1; //current location minus 1 as index was incremented
							}
						}
						//store connections
						if(strstr(buff, connections) != NULL){
							//room 1
							if((index -1) == 0){
								//find name in names[] array
								memset(tempName, '\0', sizeof(tempName));
								strncpy(tempName, buff+14, strlen(buff)-14);
								int b = 0;
								for(b; b < 10; b++){
									nameCmp = strncmp(names[b], tempName, 5);
									if(nameCmp == 0){
										//store index in room 1's connection array
										room1Connections[numConnections1] = b;
										numConnections1++;
									}
								}
							}	
							//room 2
							if((index -1) == 1){
								//find name in names [] array
								memset(tempName, '\0', sizeof(tempName));
								strncpy(tempName, buff+14, strlen(buff)-14);
								int b = 0;
								for(b; b < 10; b++){
									nameCmp = strncmp(names[b], tempName, 5);
									if(nameCmp == 0){
										//store index in room 2's connection array
										room2Connections[numConnections2] = b;
										numConnections2++;
									}
								}
							}
							//room 3
							if((index -1) == 2){
								//find name in names[] array
								memset(tempName, '\0', sizeof(tempName));
								strncpy(tempName, buff+14, strlen(buff)-14);
								int b = 0;
								for(b; b < 10; b++){
									nameCmp = strncmp(names[b], tempName, 5);
									if(nameCmp == 0){
										//store index in room 3's connection array
										room3Connections[numConnections3] = b;
										numConnections3++;
									}
								}
							}
							//room 4
							if((index -1) == 3){
								//find name in names[] array
								memset(tempName, '\0', sizeof(tempName));
								strncpy(tempName, buff+14, strlen(buff)-14);
								int b = 0;
								for(b; b < 10; b++){
									nameCmp = strncmp(names[b], tempName, 5);
									if(nameCmp == 0){
										//store index in room 4's connection array
										room4Connections[numConnections4] = b;
										numConnections4++;
									}
								}
							}
							//room 5
							if((index -1) == 4){
								//find name in names[] array
								memset(tempName, '\0', sizeof(tempName));
								strncpy(tempName, buff+14, strlen(buff)-14);
								int b = 0;
								for(b; b < 10; b++){
									nameCmp = strncmp(names[b], tempName, 5);
									if(nameCmp == 0){
										//store index in room 5's connection array
										room5Connections[numConnections5] = b;
										numConnections5++;
									}
								}
							}
							//room 6
							if((index -1) == 5){
								//find name in names[] array
								memset(tempName, '\0', sizeof(tempName));
								strncpy(tempName, buff+14, strlen(buff)-14);
								int b = 0;
								for(b; b < 10; b++){
									nameCmp = strncmp(names[b], tempName, 5);
									if(nameCmp == 0){
										//store index in room 6's connection array
										room6Connections[numConnections6] = b;
										numConnections6++;
									}
								}
							}
							//room 7
							if((index -1) == 6){
								//find name in names[] array
								memset(tempName, '\0', sizeof(tempName));
								strncpy(tempName, buff+14, strlen(buff)-14);
								int b = 0;
								for(b; b < 10; b++){
									nameCmp = strncmp(names[b], tempName, 5);
									if(nameCmp == 0){
										//store index in room 7's connection array
										room7Connections[numConnections7] = b;
										numConnections7++;
									}
								}
							}
						}
					}
				}
		}
	}
	free(roomPath);
	free(buff);
	free(tempName);
	free(tempType);
	
	//close directory
	closedir(dirToCheck);
	
	//variables for play
	char *nextRoom;	//user entered room
	nextRoom = calloc(9, sizeof(char));
	size_t nextSize = 9;
	int inputCmp; 	//stores result of strcmp
	int endRoomFound = 0; 	//the user has reached the end room
	int steps = 0; 	//number of rooms the user visited
	int path[100]; 	//stores the path of indices to the rooms the user visited
	int found = 0;	//found the room entered

	//current location of the game
	//initalized to the starting room index
	int curLoc = startRoom;

	//while the end room hasn't been found, prompt the user
	while(endRoomFound != 1){
		//if the current location is the end, display end message
		if(curLoc == endRoom){
			printf("YOU HAVE FOUND THE END ROOM. CONGATULATIONS!\n");
			printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", steps);
			//print path
			int j = 0;
			for(j; j < steps; j++){
				printf("%s\n", names[fileRooms[path[j]]]);
			}	
			//get out of while loop
			endRoomFound = 1;
		}
		//not end room, display options
		else{
			//display current location information
			printf("CURRENT LOCATION: %s\n", names[fileRooms[curLoc]]);
			printf("POSSIBLE CONNECTIONS: ");
			//first room
			if(curLoc == 0){
				//display room 1 connections
				int i = 0;
				for(i; i < numConnections1; i++){
					//last room adds new line, otherwise a comma
					if(i == numConnections1 - 1){
						printf("%s.\n", names[room1Connections[i]]);
					}
					else{
						printf("%s, ", names[room1Connections[i]]);
					}
				}
				do{
					//prompt suer
					printf("WHERE TO >");
					memset(nextRoom, '\0', sizeof(nextRoom));
					//get user input
					getline(&nextRoom, &nextSize, stdin);
					printf("\n");
					//reset found
					found = 0;
					//check for time input
					if(strncmp(nextRoom, "time", 4) == 0){
						pthread_mutex_unlock(&myMutex);
						//go to time function
						result_code = pthread_join(myThreadID, NULL);
						assert(0 == result_code);
						
						//recreate thread after join
						result_code = pthread_create(&myThreadID, NULL, writeTime, NULL);
						assert(0 == result_code); 
					}
					//check from room input
					else{
						//search room 1's connections for matching room via user input
						int k = 0;
						for(k; k < numConnections1; k++){
							//compare user input with room
							inputCmp = strncmp(nextRoom, names[room1Connections[k]], 5);
							//room is in the connections
							if(inputCmp == 0){
								//find file in names array
								int a = 0;
								for(a; a < 7; a++){
									inputCmp = strncmp(nextRoom, names[fileRooms[a]], 5);
									if(inputCmp == 0){
										//the room is found
										//current location is set to connecting room
										curLoc = a;
										//add the current location to the path
										path[steps] = curLoc;
										//increment steps
										steps++;
										//mark room as found
										found = 1;
									}
								}
							}
						}
						//if there is bad input reprompt the user
						if(found != 1){
							printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
						}
					}
				}while(found != 1);
			}
			//room 2
			else if(curLoc == 1){
				//display room 2 connections
				int i = 0;
				for(i; i < numConnections2; i++){
					//final room gets newline, comma otherwise
					if(i == numConnections2-1){
						printf("%s.\n", names[room2Connections[i]]);
					}
					else{
						printf("%s, ", names[room2Connections[i]]);
					}
				}
				// get user input
				do{
					//prompt user
					printf("WHERE TO? >");
					memset(nextRoom, '\0', sizeof(nextRoom));
					//get user input
					getline(&nextRoom, &nextSize, stdin);
					printf("\n");
					//reset found
					found = 0;
					//check for time input
					if(strncmp(nextRoom, "time", 4) == 0){
						pthread_mutex_unlock(&myMutex);
						//go to time function
						result_code = pthread_join(myThreadID, NULL);
						assert(0 == result_code);
						
						//recreate thread after join
						result_code = pthread_create(&myThreadID, NULL, writeTime, NULL);
						assert(0 == result_code); 
					}
					//check from room input
					else{
						//search room 2's connections for matching room via user input
						int k = 0;
						for(k; k < numConnections2; k++){
							//compare user input with room name
							inputCmp = strncmp(nextRoom, names[room2Connections[k]], 5);
							if(inputCmp == 0){
								//find file in names array
								int a = 0;
								for(a; a < 7; a++){
									inputCmp = strncmp(nextRoom, names[fileRooms[a]], 5);
									if(inputCmp == 0){
										//room is found
										//current location is set to connecting rooms
										curLoc = a;
										//add the current location to the path
										path[steps] = curLoc;
										//increment steps
										steps++;
										//mark room as found
										found = 1;
									}
								}
							}
						}
						//if theres bad input, reprompt the user
						if(found != 1){
							printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
						}
					}
				}while(found != 1);
			}
			//room 3
			else if(curLoc == 2){
				//display room 3's connections
				int i = 0;
				for(i; i < numConnections3; i++){
					if(i == numConnections3 -1){
						printf("%s.\n", names[room3Connections[i]]);
					}
					else{
						printf("%s, ", names[room3Connections[i]]);
					}
				}
				// get user input
				do{
					//reset found
					found = 0;
					//prompt user
					printf("WHERE TO? >");
					memset(nextRoom, '\0', sizeof(nextRoom));
					//get input
					getline(&nextRoom, &nextSize, stdin);
					printf("\n");
					//check for time input
					if(strncmp(nextRoom, "time", 4) == 0){
						pthread_mutex_unlock(&myMutex);
						//go to time function
						result_code = pthread_join(myThreadID, NULL);
						assert(0 == result_code);
						
						//recreate thread after join
						result_code = pthread_create(&myThreadID, NULL, writeTime, NULL);
						assert(0 == result_code); 
					}
					//check from room input
					else{
						//compare input to connection names
						int k = 0;
						for(k; k < numConnections3; k++){
							inputCmp = strncmp(nextRoom, names[room3Connections[k]], 5);
							if(inputCmp == 0){
								//compare input to names array
								int a = 0;
								for(a; a < 7; a++){
									inputCmp = strncmp(nextRoom, names[fileRooms[a]], 5);
									if(inputCmp == 0){
										//room found, reset current location and add to path
										curLoc = a;
										path[steps] = curLoc;
										steps++;
										found = 1;
									}
								}
							}
						}
						//if bad input, reprompt user
						if(found != 1){
							printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
						}
					}
				}while(found != 1);
			}
			//room 4
			else if(curLoc == 3){
				//print room 4's connections
				int i = 0;
				for(i; i < numConnections4; i++){
					if(i == numConnections4-1){
						printf("%s.\n", names[room4Connections[i]]);
					}
					else{
						printf("%s, ", names[room4Connections[i]]);
					}
				}
				// get user input
				do{
					//prompt user
					printf("WHERE TO? >");
					memset(nextRoom, '\0', sizeof(nextRoom));
					//get user input
					getline(&nextRoom, &nextSize, stdin);
					printf("\n");
					found = 0;
					//check for time input
					if(strncmp(nextRoom, "time", 4) == 0){
						pthread_mutex_unlock(&myMutex);
						result_code = pthread_join(myThreadID, NULL);
						assert(0 == result_code);
						
						//recreate thread after join
						result_code = pthread_create(&myThreadID, NULL, writeTime, NULL);
						assert(0 == result_code); 
					}
					//check from room input
					else{
						//check input against connections
						int k = 0;
						for(k; k < numConnections4; k++){
							inputCmp = strncmp(nextRoom, names[room4Connections[k]], 5);
							if(inputCmp == 0){
								//check input agains names array to get index
								int a = 0;
								for(a; a < 7; a++){
									inputCmp = strncmp(nextRoom, names[fileRooms[a]], 5);
									if(inputCmp == 0){
										//update current lcoation and path
										curLoc =a;
										path[steps] = curLoc;
										steps++;
										found = 1;
									}
								}
							}
						}
						//if there's bad input reprompt user
						if(found != 1){
							printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN\n\n");
						}
					}
				}while(found != 1);
			}
			//room 5
			else if(curLoc == 4){
				//display room 5's connections
				int i = 0;
				for(i; i < numConnections5; i++){
					if(i == numConnections5-1){
						printf("%s.\n", names[room5Connections[i]]);
					}
					else{
						printf("%s, ", names[room5Connections[i]]);
					}
				}
				// get user input
				do{
					//prompt user
					printf("WHERE TO? >");
					memset(nextRoom, '\0', sizeof(nextRoom));
					//get input from user
					getline(&nextRoom, &nextSize, stdin);
					printf("\n");
					found = 0;
					//check for time input
					if(strncmp(nextRoom, "time", 4) == 0){
						pthread_mutex_unlock(&myMutex);
						result_code = pthread_join(myThreadID, NULL);
						assert(0 == result_code);
						
						//recreate thread after join
						result_code = pthread_create(&myThreadID, NULL, writeTime, NULL);
						assert(0 == result_code); 
					}
					//check from room input
					else{
						//compare against room's connections
						int k = 0;
						for(k; k < numConnections5; k++){
							inputCmp = strncmp(nextRoom, names[room5Connections[k]], 5);
							if(inputCmp == 0){
								//compare against names array to get index
								int a = 0;
								for(a; a < 7; a++){
									inputCmp = strncmp(nextRoom, names[fileRooms[a]], 5);
									if(inputCmp == 0){
										//room found, update current lcoation, path and steps
										curLoc = a;
										path[steps] = curLoc;
										steps++;
										found = 1;
									}
								}
							}
						}
						//bad input, reprompt user
						if(found != 1){
							printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
						}
					}
				}while(found != 1);
			}
			// room 6
			else if(curLoc == 5){
				//display room 6 connections
				int i = 0;
				for(i; i < numConnections6; i++){
					if(i == numConnections6-1){
						printf("%s.\n", names[room6Connections[i]]);
					}
					else{
						printf("%s, ", names[room6Connections[i]]);
					}
				}
			// get user input
				do{
					//prompt user
					printf("WHERE TO? >");
					memset(nextRoom,'\0', sizeof(nextRoom));
					//get user input
					getline(&nextRoom, &nextSize, stdin);
					printf("\n");
					found = 0;
					//check for time input
					if(strncmp(nextRoom, "time", 4) == 0){
						pthread_mutex_unlock(&myMutex);
						result_code = pthread_join(myThreadID, NULL);
						assert(0 == result_code);
						
						//recreate thread after join
						result_code = pthread_create(&myThreadID, NULL, writeTime, NULL);
						assert(0 == result_code); 
					}
					//check from room input
					else{
						//compare against rooms connections
						int k = 0;
						for(k; k < numConnections6; k++){
							inputCmp = strncmp(nextRoom, names[room6Connections[k]], 5);
							if(inputCmp == 0){
								//compare against names array to get index
								int a = 0;
								for(a; a < 7; a++){
									inputCmp = strncmp(nextRoom, names[fileRooms[a]], 5);
									if(inputCmp == 0){
										//update current room information and path and steps
										curLoc = a;
										path[steps] = curLoc;
										steps++;
										found = 1;
									}
								}
							}
						}
						//bad input, reprompt user
						if(found != 1){
							printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
						}
					}
				}while(found != 1);
			}
			//room 7
			else{
				//display room 7's connections
				int i = 0;
				for(i; i < numConnections7; i++){
					if(i == numConnections7-1){
						printf("%s.\n", names[room7Connections[i]]);
					}
					else{
						printf("%s, ", names[room7Connections[i]]);
					}
				}
				// get user input
				do{
					//prompt user
					printf("WHERE TO? >");
					memset(nextRoom, '\0', sizeof(nextRoom));
					//get user input
					getline(&nextRoom, &nextSize, stdin);
					printf("\n");
					found = 0;
					//check for time input
					if(strncmp(nextRoom, "time", 4) == 0){
						pthread_mutex_unlock(&myMutex);
						result_code = pthread_join(myThreadID, NULL);
						assert(0 == result_code);
						
						//recreate thread after join
						result_code = pthread_create(&myThreadID, NULL, writeTime, NULL);
						assert(0 == result_code); 
					}
					//check from room input
					else{
						//compare againts connections
						int k = 0;
						for(k; k < numConnections7; k++){
							inputCmp = strncmp(nextRoom, names[room7Connections[k]], 5);
							if(inputCmp == 0){
								//compare against names array to get index
								int a = 0;
								for(a; a < 7; a++){
									inputCmp = strncmp(nextRoom, names[fileRooms[a]], 5);
									if(inputCmp == 0){
										//update current location, path and steps
										curLoc = a;
										path[steps] = curLoc;
										steps++;
										found = 1;
									}
								}
							}
						}
						//bad input, reprompt user
						if(found != 1){
							printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN\n\n");
						}
					}
				}while(found != 1);
			}
		}
	}
	free(nextRoom);
	//test
	//exit program
	return 0;

}
