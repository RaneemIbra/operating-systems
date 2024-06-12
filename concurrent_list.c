#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "concurrent_list.h"

//define the struct of the node and add the mutex lock so that we can lock it and unlock it whenever we want a thread save implementation
struct node {
  int value;
  node* next;
  pthread_mutex_t lock;
};

//define the list struct
struct list {
  node* start;
};

//a given function that prints each node on it self
void print_node(node* node)
{
  // DO NOT DELETE
  if(node)
  {
    printf("%d ", node->value);
  }
}

//a simple function to create a list of nodes
list* create_list()
{
  //the function starts by allocating the list
  list* L1 = (list*) malloc(sizeof(list));

  //if the malloc failed we have to exit with a failure
  if(!L1){
    exit(EXIT_FAILURE);
  }

  //if we are here then the malloc worked and we should initialize the start feild of the list to null and then we return the list
  L1->start = NULL;
  return L1;
}

//a function that will take a list as an argument and deletes it completely with keeping in mind thread safety
void delete_list(list* list)
{
  //check if the list is null, if yes then we terminate, if not we continue
  if (!list) {
      return;
  }

  //define pointers that will go over the list and start freeing the memory
  node* head = list->start;
  node* iterator;

  //start iterating the list
  while (head) {

    //lock the node where the first pointer is pointing at so that other threads can't access it during the run of the current thread
    pthread_mutex_lock(&head->lock);

    //move the iterator to the next node
    iterator = head->next;

    //lock the next node if it is not null for the safety of the node
    if(iterator){
      pthread_mutex_lock(&iterator->lock);
    }

    //unlock the node where we are currently and then destroy it
    pthread_mutex_unlock(&head->lock);
    pthread_mutex_destroy(&head->lock);

    //free the memory that was allocated
    free(head);

    //move the head to be the node after
    head = iterator;
  }

  //after we freed every node, now it is time to free the list
  free(list);
}

//a function that will insert a node with the given value to the place corresponding to the value, because the list is sorted according to the values of each node
void insert_value(list* list, int value)
{
  //start by checking if the list is null, also terminate if it is, else continue
  if(!list){
    return;
  }

  //allocate the node we want to insert into the list, if the malloc failed, then exit with code failure
  node* n1 = (node*)malloc(sizeof(node));
  if(!n1){
    exit(EXIT_FAILURE);
  }

  //give the node the value from the arguments, initialize the mutex obejct, and lock the node for safety from other threads accessing
  n1->value = value;
  pthread_mutex_init(&n1->lock, NULL);
  pthread_mutex_lock(&n1->lock);

  //if the list is empty then we have only 1 node and it is n1, we unlock the node before we finish and then we return
  if(!list->start){
    n1->next = NULL;
    list->start = n1;
    pthread_mutex_unlock(&n1->lock);
    return;
  }

  //if we are here that means the list consists of one or more nodes, therefore we define iterators that will go over the list and will find where to insert the node
  node* iterator1 = list->start;
  node* iterator2 = NULL;

  //lock the first node before we start
  pthread_mutex_lock(&iterator1->lock);

  //now we start iterating until the first iterator is either null or in the correct position in where we need to insert the node
  while(iterator1 && iterator1->value < value){
    //the second iterator will be our previous node in the list, and if it is not null then we want to unlock it because we are done working with it and we can let other threads access it
    if(iterator2){
      pthread_mutex_unlock(&iterator2->lock);
    }

    //move the iterators one step forward
    iterator2 = iterator1;
    iterator1 = iterator1->next;

    //if the first iterator is still pointing to a valid node then we lock the node
    if(iterator1){
      pthread_mutex_lock(&iterator1->lock);
    }
  }

  //check if the previous node is null, if it is then we need to put the node we allocated in the head of the list
  if(!iterator2){
    n1->next = list->start;
    list->start = n1;
  }

  //otherwise we need to put the node we allocated between the first iterator and the second iterator, then we have to unlock it, so that other threads can access it
  else{
    n1->next = iterator1;
    iterator2->next = n1;
    pthread_mutex_unlock(&iterator2->lock);
  }

  //don't forget to unlock the last node we locked which is what iterator1 is poointing to
  if(iterator1){
    pthread_mutex_unlock(&iterator1->lock);
  }

  //lastly we want to unlcok the node we allocated so that it can be accessed and modified by other threads
  pthread_mutex_unlock(&n1->lock);
}

//this function will remove a certain value from the list we have with keeping in mind thread safety
void remove_value(list* list, int value)
{
  //check if the list is null or empty, and if yes then we terminate
  if(!list || !list->start){
    return;
  }

  //we will need to iterate over the list just like before so we define the iterators, and then we lock the first for safety
  node* iterator1 = list->start;
  node* iterator2 = NULL;
  pthread_mutex_lock(&iterator1->lock);

  //iterate the list until the first iterator becomes null, or finds the value we are looking for
  while(iterator1 && iterator1->value != value){

    //if the second iterator isn't null then we need to unlock it because we are done working with it and we should let other threads access it freely
    if(iterator2){
      pthread_mutex_unlock(&iterator2->lock);
    }
    
    //move the iterators one step forward
    iterator2 = iterator1;
    iterator1 = iterator1->next;

    //lock the next node so that other threads can't modify it
    if(iterator1){
      pthread_mutex_lock(&iterator1->lock);
    }
  }

  //when we reach the final node with the second iterator, we will need to unlock it because we are done with it
  if(!iterator1){
    if(iterator2){
      pthread_mutex_unlock(&iterator2->lock);
    }
    return;
  }

  //if the second iterator is null, that means we are in the head of the list, therefore we move the head one step forward
  if(!iterator2){
    list->start = iterator1->next;
  }

  //else we are somewhere in the middle of the end, and we set the next of iterator2 to the next of iterator1, and that means that now the node that iterator1 is pointing to cannot be reached anymore (apart from using iterator1)
  else{
    iterator2->next = iterator1->next;
  }

  //now we need to unlock the node that iterator1 is pointing to and then destroy it because it is the node we wanted, and then we free the memory
  pthread_mutex_unlock(&iterator1->lock);
  pthread_mutex_destroy(&iterator1->lock);
  free(iterator1);

  //as a last step we should also unlock iterator2 since we locked it and never unlocked it again
  if(iterator2){
    pthread_mutex_unlock(&iterator2->lock);
  }
}

//a function that will print the list one node at a time according to the node function that was declared and implemented above
void print_list(list* list)
{
  //check if the list is null, if yes then we terminate
  if(!list){
    return;
  }

  //check if the list is empty, if yes then we print an empty line
  if(!list->start){
    printf("\n");
  }

  //just like before we will need the pointers to iterate the list
  node* head = list->start;
  node* iterator;

  //lock the head for protection from race conditions
  pthread_mutex_lock(&head->lock);

  //iterate the list node by node
  while(head){

    //move forward one step and if the iterator isn't null then lock the node
    iterator = head->next;
    if(iterator){
      pthread_mutex_lock(&iterator->lock);
    }

    //call the print function and print the node
    print_node(head);

    //unlock the node because we are done with it and we can let other threads access it
    pthread_mutex_unlock(&head->lock);

    //move the head forward to iterate the whole list
    head = iterator;
  }

  printf("\n");
}

//a function that will count the list nodes that satisfy a certain predicate
void count_list(list* list, int (*predicate)(int))
{
  //initialize a counter for the amount of the nodes that will satisfy the condition
  int count = 0;

  //check if the list is empty or null, and if yes then print the count which is 0 and then terminate
  if (!list || !list->start) {
    printf("%d items were counted\n", count);
    return;
  }

  //just like before, define the iterators and lock the head for safety
  node* head = list->start;
  node* iterator;
  pthread_mutex_lock(&head->lock);

  //iterate over the list so that we can start counting
  while(head){

    //move the iterator to the next node then lock that node for safety from race conditions
    iterator = head->next;
    if(iterator){
      pthread_mutex_lock(&iterator->lock);
    }

    //check if the current node satisfies the predicate, and if yes then increment the counter
    if(predicate(head->value)){
      count++;
    }

    //now we unlock the current node because we are done working with it and then we move it one step forward
    pthread_mutex_unlock(&head->lock);
    head = iterator;
  }

  printf("%d items were counted\n", count);
}
