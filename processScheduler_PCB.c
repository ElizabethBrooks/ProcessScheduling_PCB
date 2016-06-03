/*
Name: Elizabeth Brooks 
File: processScheduler_PCB
Date modified: June 2, 2016
*/
//Class imports
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//Global variables
static int runCounter=0, pcbCounter=0;
//Global constants
static const int TIME_QUANTUM=30, FILE_OFFSET=38;
//Structure for PCB processes by descriptor
struct ProcessesPCB {
    int pID, pBurst, pBase; //Initialize process ID, burst time, and base register as int
    long pLim; //Initialize process limit register as long
    char pName[17], pAStatus, pPr; //Initialize process name, activity status, and priority as character arrays
}; //End struct
int modDiff(const void *a, const void *b){ //Function for qsort to set process priorities
   return ((*(int*)a%1000)-(*(int*)b%1000));
} //End modDiff
//Class functions for performing selected PCB processes
void loadPCB(struct ProcessesPCB *processesInput, int pcbCounter, FILE *readP){ //Function to retrieve PCB processes
    char pBuffer[38]; //Declare buffer for reading processes
    int pCounter=0; //Process counter
    for(pCounter=0; pCounter<pcbCounter; pCounter++){ //Loop through and set processes in struct
        fread(pBuffer, FILE_OFFSET, 1, readP); //Read current position of binary file using FILE_OFFSET
        memcpy(&processesInput[pCounter].pName, &pBuffer[0], 16);
        processesInput[pCounter].pName[16]='\0';
        memcpy(&processesInput[pCounter].pID, &pBuffer[16], sizeof(int));
        memcpy(&processesInput[pCounter].pBase, &pBuffer[25], sizeof(int));
        memcpy(&processesInput[pCounter].pLim, &pBuffer[29], sizeof(long));
        memcpy(&processesInput[pCounter].pBurst, &pBuffer[21], sizeof(int));
        memcpy(&processesInput[pCounter].pPr, &pBuffer[37], sizeof(char));
        memcpy(&processesInput[pCounter].pAStatus, &pBuffer[20], sizeof(char));
        if(processesInput[pCounter].pAStatus==1) runCounter++; //Increment process counter based on current status
    } //End for
} //End loadPCB
void writePCB(struct ProcessesPCB *processesInput, int pcbCounter, FILE *writeP){ //Function to store PCB process data
    char wBuffer[38]; //Initilize buffer for writing current process info to the struct
    int wCounter=0; //Counter for writing process info to the struct
    for(wCounter=0; wCounter<pcbCounter; wCounter++){ //Loop and store current process info to the class struct
        fseek(writeP, wCounter * FILE_OFFSET, SEEK_SET); //Retrieve current process info from the binary PCB data file
        memcpy(&wBuffer[0], &processesInput[wCounter].pName, 16);
        memcpy(&wBuffer[16], &processesInput[wCounter].pID, sizeof(int));
        memcpy(&wBuffer[25], &processesInput[wCounter].pBase, sizeof(int));
        memcpy(&wBuffer[29], &processesInput[wCounter].pLim, sizeof(long));
        memcpy(&wBuffer[21], &processesInput[wCounter].pBurst, sizeof(int));
        memcpy(&wBuffer[37], &processesInput[wCounter].pPr, sizeof(char));
        memcpy(&wBuffer[20], &processesInput[wCounter].pAStatus, sizeof(char));
        fwrite(wBuffer, sizeof(wBuffer), 1, writeP); //Write the process data to the file
    } //End for
} //End writePCB
void printPCB(struct ProcessesPCB *processesInput, int pcbCounter){ //Function to display PCB process info
    int fCounter=0; //Counter for looping through process info
    for(fCounter=0; fCounter<pcbCounter; fCounter++){ //Loop through the current process' descriptions
        printf("%3d.", fCounter+1);
        printf("%16s", processesInput[fCounter].pName);
        printf("%12d", processesInput[fCounter].pID);
        printf("%7d", processesInput[fCounter].pBase);
        printf("%7lu", processesInput[fCounter].pLim);
        printf("%5d", processesInput[fCounter].pBurst);
        printf("%5d\n", processesInput[fCounter].pPr);
        printf("%3d", processesInput[fCounter].pAStatus);
    } //End for
} //End printPCB
void loadIndexPCB(struct ProcessesPCB *processesInput, int *pQueue){ //Function to set the current process index
    printf("Index:\n");
    int iCounter=0; //Counter for current process index
    for(iCounter=0; iCounter<pcbCounter; iCounter++){ //Loop throguh and load index data
        pQueue[iCounter]=processesInput[iCounter].pPr+(iCounter*1000);
    } //End for    
    qsort(pQueue, pcbCounter, sizeof(int), modDiff); //Sort based on priority    
    for(iCounter=0; iCounter<pcbCounter; iCounter++){ //Determine priority order
        pQueue[iCounter]=pQueue[iCounter]/1000;
    } //End for
} //End loadIndexPCB
void setPriorityPCB(struct ProcessesPCB *processesInput){ //Function to determine current process priority
    int prCounter, currentP; //Variables for setting process priorities
    for(prCounter=0; prCounter<pcbCounter; prCounter++){ //Loop through processes and set priorities
        if((&processesInput[prCounter].pAStatus!=0)&&(processesInput[prCounter].pPr>0)){ //Determine priority
            currentP=processesInput[prCounter].pPr-1;
            memcpy(&processesInput[prCounter].pPr, &currentP, sizeof(char));
        } //End if
    } //End for
} //End setPriorityPCB
int burstCPU(struct ProcessesPCB *processesInput, int counterInput){ //Function for setting the CPU burst
    int prStatus=processesInput[counterInput].pAStatus; //Set the current process status counter
    int prBurst=processesInput[counterInput].pBurst; //Set the current process burst time
    prBurst--; //Decrement burst counter
    if((prBurst==0)&&(prStatus!=0)){ //Attempt to perform burst
        runCounter--; //Decrement process run counter
        prStatus, prBurst=0; //Reinitialize the process status and burst counters
        memcpy(&processesInput[counterInput].pAStatus, &prStatus, sizeof(char));
        memcpy(&processesInput[counterInput].pBurst, &prBurst, sizeof(int));
        return prStatus; //If burst succeeds, return the process status
    } //End if
    memcpy(&processesInput[counterInput].pBurst, &prBurst, sizeof(int)); //Store current process burst time
    return prStatus; //Return current process status
} //End burstCPU
//The main method
int main(){ 
    int i=0, prStatus=0, jmpStatus=0, prIndex=0, fnStatus=0, fSize; //Initialize variables for looping through PCB processes    
    FILE *writeP; //Create new file writer
    FILE *readP; //Create new file reader
    writeP=fopen("current_processes.bin", "wb"); //Open specified file writing
    readP=fopen("processes.bin", "rb"); //Open specified file for reading
    fseek(readP, 0L, SEEK_END); //Set file index position to end
    fSize=ftell(readP); //Retrieve the current file position
    rewind(readP); //Return to the beginning of the file
    pcbCounter=(fSize/FILE_OFFSET); //Set the current process
    struct ProcessesPCB processesInput[pcbCounter]; //Initialize struct for PCB process data
    loadPCB(processesInput, pcbCounter, readP); //Call function to load process data
    int *pQueue=NULL; //Initialize queue for process priorities
    pQueue=((int*)realloc(pQueue, runCounter *sizeof(int))); //Cast the queue priority as int
    loadIndexPCB(processesInput, pQueue); //Load process indexes    
    while(!fnStatus){ //Perform processes based on specified round robin scheduling
        jmpStatus, prStatus=0; //Reinitialize tracker variables
        while(prStatus<TIME_QUANTUM){ //Loop for the amount of time specified by quantum
            if(processesInput[i].pAStatus!=0){ //Attempt process based on current status
                if(burstCPU(processesInput, i++)==0) printf("Process %d complete!\n", runCounter); //Determine if complete
                writePCB(processesInput, pcbCounter, writeP); //Store the current process data using class function
                printPCB(processesInput, pcbCounter); //Display the current process data using class function
                if(((i%2)==0)&&(i>0)) setPriorityPCB(processesInput);
                prStatus++; //Increment counter
                jmpStatus=0; //Reinitialize skip tracker variable
            }else{
                i++; //Increment counter for looping
                if(++jmpStatus==pcbCounter){ //Determine if complete
                    printf("Program complete!");
                    prStatus=TIME_QUANTUM;
                    fnStatus=1;
                    writePCB(processesInput, pcbCounter, writeP); //Use class function to write PCB data
                    return 0; //Exit program return
                } //End if
                if(((i%2)==0)&&(i>0)) setPriorityPCB(processesInput); //If even and not 0, set process priority
            } //End if, else
            i=(i%pcbCounter); //Reset program counter
        } //End first inner while        
        prStatus=0; //Reinitialize priority tracker
        while(prStatus<TIME_QUANTUM){
            if(prIndex==pcbCounter){
                printf("Program complete!");
                prStatus=TIME_QUANTUM;
                fnStatus=1;
                writePCB(processesInput, pcbCounter, writeP); //Use class function to store process data
                return 0; //End of program return
            } //End if
            if(processesInput[pQueue[prIndex]].pAStatus!=0){
                prStatus++;
                if(burstCPU(processesInput, pQueue[prIndex]) == 0) printf("Process complete! (Priority: %d)\n", runCounter);
                writePCB(processesInput, pcbCounter, writeP); //Use class function to store process data
                printPCB(processesInput, pcbCounter); //Use class function to display process data
            }else{
                prIndex++; //Incrememnt process index
            } //End if, else
            if(((i%2)==0)&&(i>0)) setPriorityPCB(processesInput); //If even and not 0, set process priority
        } //End second inner wile
    } //End outter while    
    printPCB(processesInput, pcbCounter); //Use class function to display process data
    writePCB(processesInput, pcbCounter, writeP); //Use class function to store process data
    //Close binary files
    fclose(writeP);
    fclose(readP);
    return 0; //End of program return
} //End main