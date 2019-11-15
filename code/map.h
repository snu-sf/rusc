#ifndef MAP_HEADER_FILE_H
#define MAP_HEADER_FILE_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct node {
  int key;
  int val;
  struct node * next;
} node_t;

node_t **new() {
  struct node ** head = malloc(sizeof(node_t **));
  *head = NULL;
  return head;
}
/* node_t *new() { */
/*   node_t * head = malloc(sizeof(node_t)); */
/*   head->key = -1; */
/*   head->val = -1; */
/*   head->next = NULL; */
/*   return head; */
/* } */

void insert(node_t ** head, int key, int val) {
    node_t * new_node;
    new_node = malloc(sizeof(node_t));

    new_node->key = key;
    new_node->val = val;
    new_node->next = *head;
    *head = new_node;
}

// -1 is None
/* int find(node_t * head, int key) { */
/*   while(head) { */
/*     if(head->key == key) return head->val; */
/*     head = head->next; */
/*   } */
/*   return -1; */
/* } */
int find(node_t ** head, int key) {
  node_t* cur = *head;
  /* printf("%d\n", cur); */
  while(cur) {
    if(cur->key == key) return cur->val;
    cur = cur->next;
  }
  return -1;
}

#endif
