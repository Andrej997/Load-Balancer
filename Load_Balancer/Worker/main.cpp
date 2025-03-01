#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#define WIN32_LEAN_AND_MEAN

#pragma region IncludeLibrary
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <string.h>
#pragma endregion

#pragma region DefaultValue
#define DEFAULT_BUFLEN 4048 * 2
#define DEFAULT_PORT 27016
#pragma endregion

#pragma region IncludeHeaderFiles
#include "communicationFuncs.h"
#include "structs.h"
#include "list.h"
#include "workerToLoadBalancer.h"
#pragma endregion

Node *headMessages;

int __cdecl main(int argc, char **argv)
{
	#pragma region InitVariable
	int msgCount = 0;  // number of messages that this worker contains
	headMessages = NULL;
	int iResult = -1;  // variable used to store function return value
	int iResultSecond = -1;  // variable used to store function return value
	//char recvbuf[DEFAULT_BUFLEN];
	SOCKET connectSocket; // socket used to communicate with server
	#pragma endregion

	connectSocket = SetConnectedSocket(DEFAULT_PORT);
	if (connectSocket == 1) {
		printf("Error with connect socket. Press enter to exit!\n");
		getchar();
		return 0;
	}
	printf("Worker is started...\nPress enter to exit...\n");

	while (true) {
		if (_kbhit()) {
			break;
		}
		#pragma region Set
		FD_SET set;
		FD_ZERO(&set);
		FD_SET(connectSocket, &set);
		FD_SET recvset;
		FD_ZERO(&recvset);
		FD_SET(connectSocket, &recvset);
		#pragma endregion

		timeval timeVal;
		timeVal.tv_sec = 0;
		timeVal.tv_usec = 0;

		iResult = select(0, &recvset, &set, NULL, &timeVal);

		if (iResult == SOCKET_ERROR) { //error
			printf("Select failed with error: %d\n", WSAGetLastError());
			break;
		}
		else if (iResult == 0) {
			printf("I'm waiting...\n");
			continue;
		}
		else if (FD_ISSET(connectSocket, &recvset)) { // recv
			int currentLength = 0;
			char recvbuf[DEFAULT_BUFLEN];
			iResult = recv(connectSocket, recvbuf, DEFAULT_BUFLEN, 0);
			if (iResult > 0) {
				if (*(char*)recvbuf == 'r') {  // za reorganizaciju
					printf("Worker recv: %s\n", recvbuf);
					int numOfMgs = *(int*)(recvbuf + 1);
					if (numOfMgs > 1) {
						int a = 0;

						char *reorMessage = ConvertToString(headMessages, numOfMgs, &a);

						iResult = select(0, NULL, &set, NULL, &timeVal);
						if (FD_ISSET(connectSocket, &set)) {
							//SetNonblocking(&connectSocket);
							iResult = send(connectSocket, reorMessage, a + 1, 0);
							if (iResult == SOCKET_ERROR)
							{
								printf("send failed with error: %d\n", WSAGetLastError());
								//closesocket(connectSocket);
								//WSACleanup();
								//return 1;
							}
						}
						printf("Return %d message...\n", numOfMgs);
						Node* temp = headMessages;
						int brojPoruka = 0;
						if (temp != NULL)
							brojPoruka++;
						while (temp->next != NULL) {
							brojPoruka++;
							//printf("%s\n", temp->message);
							temp = temp->next;
						}
						printf("Total number of messages: %d \n", brojPoruka);
					}
				}
				else if (*(char*)recvbuf != 'O') {	//aaaaaaaaaa
					
					int lengthCurrentMessage = 0;

					while (currentLength < iResult) {
						lengthCurrentMessage = *(int*)(recvbuf + currentLength);
						//currentLength += lengthCurrentMessage + 4;
						
						printf("Message received from server(");
						Message *message = (Message*)malloc(sizeof(Message));
						message->size = lengthCurrentMessage;
						message->message = (char*)malloc(message->size);
						message->clientId = *(int*)(recvbuf + 4);
						++msgCount;

						printf("%d bytes): ", message->size - 17);
						for (int i = 0; i < message->size + 4; i++) {
							message->message[i] = *(char*)(recvbuf + i + currentLength);
							if(i > 21)
								printf("%c", message->message[i]);
						}
						message->message[message->size + 4] = NULL;
						currentLength += lengthCurrentMessage + 5;

						printf("\n");
						//printf("ClientId : %d\n", message->clientId);
						//printf("\t\tCurrent messages count : %d\n", msgCount);
						AddAtEnd(&headMessages, message);
						Node* temp = headMessages;
						int brojPoruka = 0;
						if (temp != NULL)
							brojPoruka++;
						while (temp->next != NULL) {
							brojPoruka++;
							temp = temp->next;
						}
						printf("Total number of messages: %d \n", brojPoruka);

						union {
							int id;
							char buff[4];
						}MyU;
						MyU.id = message->clientId;
						char* messageOK = (char*)malloc(1 + sizeof(int));
						messageOK[0] = 's';
						messageOK[1] = MyU.buff[0];
						messageOK[2] = MyU.buff[1];
						messageOK[3] = MyU.buff[2];
						messageOK[4] = MyU.buff[3];
						iResultSecond = send(connectSocket, messageOK, (int)strlen(messageOK) + 1, 0);
						if (iResultSecond > 0) {
							printf("Sent OK.\n");
						}
						else if (iResultSecond == SOCKET_ERROR) {
							printf("send failed with error: %d\n", WSAGetLastError());
							break;
							/*closesocket(connectSocket);
							WSACleanup();
							return 1;*/
						}
						//free(message->message);
						//free(message);
						free(messageOK);
					}

					
				}
			}
			else if (iResult == 0) { // connection was closed gracefully
				printf("Connection with server closed.\n");
				break;
			}
			else {  // there was an error during recv
				printf("recv failed with error: %d\n", WSAGetLastError());
				printf("LB crashed!\nPress enter to exit");
				getchar();
				break;
			}
		}
	}

	//free(message);
	if(headMessages != NULL)
		FreeMessages(&headMessages, msgCount);
	closesocket(connectSocket);
	WSACleanup();
	printf("Worker is successfully shutdown...\nPress enter to exit...\n");
	getchar();
	return 0;
}



