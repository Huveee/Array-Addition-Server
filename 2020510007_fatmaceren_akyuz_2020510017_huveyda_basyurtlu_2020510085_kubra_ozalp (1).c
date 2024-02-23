#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <stdbool.h>
#include <pthread.h>

////////////////////////////READ ME///////////////////////////////////////////////////////////////////////
//OUR ARRAY IS MSB
//OUR CODE CHECK IF THE INPUT STRING SIZE OVER 100 CHARS OR NOT.

const int PORT_NUMBER = 60000;
const int DATA_SIZE = 100;
char errorMessage[500]; //For error messages that receiving as parameter for receiveArrayInput function

int validateInput(char *inputString) {
    //Check if input is valid
    int valid = 1;    
    int stringLength = strlen(inputString);
    // Check for consecutive spaces
    int consecutiveSpaceCount = 0;
    for (int i = 0; i < stringLength; i++) {
        //Check if user enter tab key
        if(inputString[i] == '	'){
            sprintf(errorMessage, "ERROR: Your String includes tab character (\"   \")!!: \n");
            valid = 0;
            return valid;
        }
        //Check if user enter more than 3 space key
        if (inputString[i] == ' ') {
            consecutiveSpaceCount++;
            if (consecutiveSpaceCount >= 4) {
                valid = 0;
                sprintf(errorMessage, "ERROR: Your String includes tab character (\"   \")!!: \n");
                return valid;
            }
        } else {
            consecutiveSpaceCount = 0;
        }
    }
    char *token = strtok((char *)inputString, " ");
   
    int tokenCount = 0;

    while (token != NULL) {
        printf("Token: %s\n",token);
        //if it's the 101st token
        if(tokenCount == 100){
            sprintf(errorMessage, "Please enter at most 100 numbers!!: \n");
            valid = 0;
            return valid;
        }
        
        //Check the length of the token
        if (strlen(token) < 1 || strlen(token) > 3) {
            valid = 0; //Found improper length
            sprintf(errorMessage, "Please enter number between 1 and 999!!: \n");
            return valid;
        }
        
        //Check token if it includes only numbers or not
        for (int i = 0; i < strlen(token); i++) {
        
            if (!isdigit(token[i])) {
                if(token[i] == '-'){
                    sprintf(errorMessage, "ERROR: The inputted integer array contains negative numbers. You must input only positive integers! \n");
                }
                else{
                    sprintf(errorMessage, "ERROR: The inputted integer array contains non-integer characters. You must input only integers and empty spaces to separate inputted integers! \n");
                }
                valid = 0; // Found a char except a number
                return valid;
            }
        }

       

        //Take the next token
        //strtok function give the same result for any number of space character.
        token = strtok(NULL, " ");
        tokenCount++;
    }
    return valid;
    
}
// Structure to pass parameters to the thread function
typedef struct {
    int* FIRST_ARRAY;
    int* SECOND_ARRAY;
    int* RESULT_ARRAY;
    int* CARRY_ARRAY;
    int size;
    int index;
} ThreadArgs;
//The void* type is a generic pointer type that can point to data of any type.
//The pthread_create function allows only one parameter to be passed to the thread function, 
//and this parameter must be of type void*.
//For this reason, a struct is often used to collect the parameters 
//that want to be passed to the thread function in a single pointer.
void* addArraysThread(void* args) {
    ThreadArgs* threadArgs = (ThreadArgs*)args;

    // Add corresponding elements of FIRST_ARRAY and SECOND_ARRAY
    int sum = threadArgs->FIRST_ARRAY[threadArgs->index] + threadArgs->SECOND_ARRAY[threadArgs->index];

    // Check for carry and update RESULT_ARRAY
    if (sum > 999) {
        threadArgs->RESULT_ARRAY[threadArgs->index + 1] = sum % 1000; //Write actual value
        threadArgs->CARRY_ARRAY[threadArgs->index] = sum / 1000; //Save carry
    } else {
        threadArgs->RESULT_ARRAY[threadArgs->index + 1] = sum;
        threadArgs->CARRY_ARRAY[threadArgs->index] = 0; //No carrying
    }

    pthread_exit(NULL);
}
void receiveArrayInput(int serverSocket, int clientSocket, char* inputArray, int* dataArray, int* validCount, const char* inputMessage, const char* errorMessage) {
    send(clientSocket, inputMessage, strlen(inputMessage), 0);
    memset(inputArray, 0, DATA_SIZE); // Clear the input array

    char *receivedString = NULL;
    size_t bufferSize = 0;

    // Receive input data
    ssize_t bytesRead = getline(&receivedString, &bufferSize, fdopen(clientSocket, "r"));

    if (bytesRead == -1) {
        perror("Data reception error");
        // Handle the error as needed
    } else {
        receivedString[strcspn(receivedString, "\n")] = '\0';
        receivedString[bytesRead - 2] = '\0';
    }


    int stringLength = strlen(receivedString);
    
    //THIS PART CHECK IF THE INPUT STRING SIZE OVER 100 CHARS OR NOT. IF IT'S OVER LIMIT CLOSE THE CONNECTION.
    if (stringLength > DATA_SIZE) {
        const char* overStringLimitMessage = "Please enter at most 100 characters!!: \n";
        send(clientSocket, overStringLimitMessage, strlen(overStringLimitMessage), 0);
        // Handle the error as needed
        // Close sockets
        close(clientSocket);
        close(serverSocket);
    } 
   /*
    else if(strstr(receivedString, "    ") != NULL || strstr(receivedString, "\t") != NULL){
        const char* includeTabInStringMessage = "ERROR: Your String includes tab character (\"   \")!!: \n";
        send(clientSocket, includeTabInStringMessage, strlen(includeTabInStringMessage), 0);
        // Close sockets
        close(clientSocket);
        close(serverSocket);
    }
   */
    else {
        strcpy(inputArray, receivedString);
        free(receivedString);

        if (validateInput(inputArray)) {
            // Process the valid input data
            int validCountTemp = 0;
            char currentNumber[4]; // Assuming a number has at most 3 characters
            int currentNumberIndex = 0;
            memset(dataArray, 0, sizeof(int) * DATA_SIZE);

            for (int i = 0; i < bytesRead; i++) {
                char currentChar = inputArray[i];

                if (currentChar == ' ' || currentChar == '\0' || currentChar == '\n') {
                    // Encountered space, null character, or newline character
                    // Complete the current number and process it
                    if (currentNumberIndex > 0) {
                        currentNumber[currentNumberIndex] = '\0';
                        int num = atoi(currentNumber);

                        // Check the length of the number
                        if (num >= 1 && num <= 999) {
                            // If valid, convert to integer and add to the array
                            dataArray[validCountTemp] = num;
                            validCountTemp++;
                        } else {
                            // If not valid, handle the error (if needed)
                        }

                        // Reset the current number
                        currentNumberIndex = 0;
                    }
                } else {
                    // Not space, null, or newline character; add to the current number
                    if (currentNumberIndex < sizeof(currentNumber) - 1) {
                        currentNumber[currentNumberIndex] = currentChar;
                        currentNumberIndex++;
                    }
                }
            }

            *validCount = validCountTemp;
        } else {
            // Handle the validation error (if needed)
            send(clientSocket, errorMessage, strlen(errorMessage), 0);
            // Close sockets
            close(clientSocket);
            close(serverSocket);
        }
    }
}
int main() {
    char INPUT_STRING [DATA_SIZE];
    int FIRST_ARRAY[DATA_SIZE];
    int SECOND_ARRAY[DATA_SIZE];
    int RESULT_ARRAY[DATA_SIZE];
    int CARRY_ARRAY[DATA_SIZE + 1];

     // Initialize arrays with zeros
    memset(RESULT_ARRAY, 0, sizeof(RESULT_ARRAY));
    memset(CARRY_ARRAY, 0, sizeof(CARRY_ARRAY));

    int first_valid_count;
    int second_valid_count;
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrSize = sizeof(struct sockaddr_in);

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Initialize server address struct
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(PORT_NUMBER);

    // Bind the socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 5) < 0) {
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
    }

    

    // Accept connection from client
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrSize);
    if (clientSocket < 0) {
        perror("Error accepting connection");
        exit(EXIT_FAILURE);
    }
    
    const char* welcomeMessage = "Hello, this is Array Addition Server!\n";
    send(clientSocket, welcomeMessage, strlen(welcomeMessage), 0);

    //function call for first input string
    receiveArrayInput(serverSocket, clientSocket, INPUT_STRING, FIRST_ARRAY, &first_valid_count,
                      "Please enter the first array for addition: \n",
                      errorMessage);

    //function call for second input string
    receiveArrayInput(serverSocket, clientSocket, INPUT_STRING, SECOND_ARRAY, &second_valid_count,
                      "Please enter the second array for addition: \n",
                      errorMessage);
    
    if(first_valid_count != second_valid_count){
        const char* invalidCountMessage = "The number of integers are different for both "
                                                    "arrays. You must send equal number of integers for both "
                                                        "arrays!: \n";
        send(clientSocket, invalidCountMessage, strlen(invalidCountMessage), 0);
        // Close sockets
        close(clientSocket);
        close(serverSocket);
    }
    else{
    // Thread variables
        pthread_t threads[first_valid_count];
        // Thread argument structures
        ThreadArgs threadArgs[first_valid_count];

        // Create threads for array addition
        for (int i = first_valid_count-1; i >= 0; i--) {
            threadArgs[i].FIRST_ARRAY = FIRST_ARRAY;
            threadArgs[i].SECOND_ARRAY = SECOND_ARRAY;
            threadArgs[i].RESULT_ARRAY = RESULT_ARRAY;
            threadArgs[i].CARRY_ARRAY = CARRY_ARRAY;
            threadArgs[i].size = first_valid_count;
            threadArgs[i].index = i;

            // Create thread
            if (pthread_create(&threads[i], NULL, addArraysThread, (void*)&threadArgs[i]) != 0) {
                perror("Error creating thread");
                exit(EXIT_FAILURE);
            }
        }

        // Wait for all threads to finish
        for (int i = first_valid_count-1; i >= 0; i--) {
            if (pthread_join(threads[i], NULL) != 0) {
                perror("Error joining thread");
                exit(EXIT_FAILURE);
            }
        }
        

        // Add carries to RESULT_ARRAY
        for (int i = 0; i <= first_valid_count; ++i) {
            RESULT_ARRAY[i] += CARRY_ARRAY[i];
        }
        int arrayLength = sizeof(RESULT_ARRAY) / sizeof(RESULT_ARRAY[0]);

        const char* resultArrayMessage = "The result of array addition are given below:\n";
        send(clientSocket, resultArrayMessage, strlen(resultArrayMessage), 0);


        int resultArrayLength = first_valid_count + 1; // +1 for potential carry
        for (int i = 0; i < resultArrayLength; i++) {

            char buffer[101];

            // Convert the integer value into a character array and place this array in the buffer
            if(RESULT_ARRAY[0]==1){
                snprintf(buffer, sizeof(buffer), "%d ", RESULT_ARRAY[i]);
            }
            else{
                snprintf(buffer, sizeof(buffer), "%d ", RESULT_ARRAY[i+1]);
                if(i == resultArrayLength-2){//to get rid of the zero at the end of the array
                    i++;
                }
            }
            //Send data in buffer to client
            send(clientSocket, buffer, strlen(buffer), 0);
        }
        const char* closeConnectionMessage = "\nThank you for Array Addition Server! Good Bye!\n";
        send(clientSocket, closeConnectionMessage, strlen(closeConnectionMessage), 0);
    }
    
    // Close sockets
    close(clientSocket);
    close(serverSocket);

    return 0;
}