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

void* Map_new() {
  node_t ** head = malloc(sizeof(node_t *));
  *head = NULL;
  return head;
}

void Map_insert(node_t ** head, int key, int val) {
    node_t * new_node;
    new_node = malloc(sizeof(node_t));

    new_node->key = key;
    new_node->val = val;
    new_node->next = *head;
    *head = new_node;
}

int Map_find(node_t ** head, int key) {
  node_t* cur = *head;
  /* printf("%d\n", cur); */
  while(cur) {
    if(cur->key == key) return cur->val;
    cur = cur->next;
  }
  return -1;
}

#endif
