#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "concurrent_list.h"

struct node {
  int value;
  node* next;
  pthread_mutex_t lock;
  // add more fields
};

struct list {
  // add fields
  node* start;
};

void print_node(node* node)
{
  // DO NOT DELETE
  if(node)
  {
    printf("%d ", node->value);
  }
}

list* create_list()
{
  list* L1 = (list*) malloc(sizeof(list));

  if(!L1){
    exit(EXIT_FAILURE);
  }

  L1->start = NULL;
  return L1; // REPLACE WITH YOUR OWN CODE
}

void delete_list(list* list)
{
  // add code here
  if (!list) {
      return;
  }

  struct node* head = list->start;
  struct node* iterator;

  while (head) {
      pthread_mutex_lock(&head->lock);
      iterator = head->next;
      if(iterator){
        pthread_mutex_lock(&iterator->lock);
      }
      pthread_mutex_unlock(&head->lock);
      pthread_mutex_destroy(&head->lock);
      free(head);
      head = iterator;
   }

  free(list);
}

void insert_value(list* list, int value)
{
  // add code here
  if(!list){
    return;
  }

  node* n1 = (node*)malloc(sizeof(node));
  if(!n1){
    exit(EXIT_FAILURE);
  }

  n1->value = value;
  pthread_mutex_init(&n1->lock, NULL);
  pthread_mutex_lock(&n1->lock);

  if(!list->start){
    n1->next = NULL;
    list->start = n1;
    pthread_mutex_unlock(&n1->lock);
    return;
  }

  node* iterator1 = list->start;
  node* iterator2 = NULL;

  pthread_mutex_lock(&iterator1->lock);
  while(iterator1 && iterator1->value < value){
    if(iterator2){
      pthread_mutex_unlock(&iterator2->lock);
    }
    iterator2 = iterator1;
    iterator1 = iterator1->next;
    if(iterator1){
      pthread_mutex_lock(&iterator1->lock);
    }
  }

  if(!iterator2){
    n1->next = list->start;
    list->start = n1;
  }
  else{
    n1->next = iterator1;
    iterator2->next = n1;
    pthread_mutex_unlock(&iterator2->lock);
  }

  if(iterator1){
    pthread_mutex_unlock(&iterator1->lock);
  }
  pthread_mutex_unlock(&n1->lock);
}

void remove_value(list* list, int value)
{
  // add code here
  if(!list || !list->start){
    return;
  }

  node* iterator1 = list->start;
  node* iterator2 = NULL;
  pthread_mutex_lock(&iterator1->lock);

  while(iterator1 && iterator1->value != value){
    if(iterator2){
      pthread_mutex_unlock(&iterator2->lock);
    }
    iterator2 = iterator1;
    iterator1 = iterator1->next;
    if(iterator1){
      pthread_mutex_lock(&iterator1->lock);
    }
  }

  if(!iterator1){
    if(iterator2){
      pthread_mutex_unlock(&iterator2->lock);
    }
    return;
  }

  if(!iterator2){
    list->start = iterator1->next;
  }
  else{
    iterator2->next = iterator1->next;
  }

  pthread_mutex_unlock(&iterator1->lock);
  pthread_mutex_destroy(&iterator1->lock);
  free(iterator1);

  if(iterator2){
    pthread_mutex_unlock(&iterator2->lock);
  }
}

void print_list(list* list)
{
  // add code here
  if(!list || !list->start){
    return;
  }

  node* head = list->start;
  node* iterator;

  pthread_mutex_lock(&head->lock);
  while(head){
    iterator = head->next;
    if(iterator){
      pthread_mutex_lock(&iterator->lock);
    }
    print_node(head);
    pthread_mutex_unlock(&head->lock);
    head = iterator;
  }

  printf("\n"); // DO NOT DELETE
}

void count_list(list* list, int (*predicate)(int))
{
  int count = 0; // DO NOT DELETE

  // add code here
  if (!list || !list->start) {
    printf("%d items were counted\n", count);
    return;
  }

  node* head = list->start;
  node* iterator;
  pthread_mutex_lock(&head->lock);

  while(head){
    iterator = head->next;
    if(iterator){
      pthread_mutex_lock(&iterator->lock);
    }
    if(predicate(head->value)){
      count++;
    }
    pthread_mutex_unlock(&head->lock);
    head = iterator;
  }

  printf("%d items were counted\n", count); // DO NOT DELETE
}
