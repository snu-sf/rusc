#include <stdio.h>
#include <stdlib.h>
#include "map.h"

node_t** Map_new() {
  node_t** head = malloc(sizeof(node_t *));
  *head = NULL;
  return head;
}

void Map_insert(node_t** head, int key, int val) {
    node_t* new_node;
    new_node = malloc(sizeof(node_t));

    new_node->key = key;
    new_node->val = val;
    new_node->next = *head;
    *head = new_node;
}

int Map_find(node_t** head, int key) {
  node_t* cur = *head;
  /* printf("%d\n", cur); */
  while(cur) {
    if(cur->key == key) return cur->val;
    cur = cur->next;
  }
  return -1;
}
