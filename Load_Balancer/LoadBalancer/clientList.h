#pragma once

void AddAtEnd(Node **head, Client *new_data);
void deleteNode(Node **head_ref, int key);
void FreeList(Node *head);
Node* FindClient(Node *head, int clientId);

void AddAtEnd(Node **head, Client *new_data) {
	Node* new_node = (struct Node*) malloc(sizeof(struct Node));

	new_node->client = (Client*)malloc(sizeof(Client));
	new_node->client = new_data;
	new_node->next = NULL;

	if (*head == NULL) {
		*head = new_node;
		return;
	}

	Node *last = *head;
	while (last->next != NULL)
		last = last->next;
	last->next = new_node;
	return;
}

void deleteNode(Node **head_ref, int key)
{
	Node* temp = *head_ref, *prev = NULL;

	if (temp != NULL && temp->client->acceptedSocket == key) {
		*head_ref = temp->next;   
		free(temp);               
		return;
	}

	while (temp != NULL && temp->client->acceptedSocket != key) {
		prev = temp;
		temp = temp->next;
	}

	if (temp == NULL) 
		return;

	prev->next = temp->next;

	free(temp);  
}

void FreeList(Node *head) {
	Node *temp;
	while (head != NULL) {
		temp = head;
		//CloseHandle(temp->client->thread);
		head = head->next;
		free(temp->client->ipAdr);
		temp->client->ipAdr = NULL;
		free(temp);
		temp = NULL;
	}
	return;
}

// find client by Id to send him succsess message
Node* FindClient(Node *head, int clientId) {
	Node *temp = head;
	while (temp != NULL) {
		if (temp->client->id == clientId)
			break;
		temp = temp->next;
	}
	return temp;
}