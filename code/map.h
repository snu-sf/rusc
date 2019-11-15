#ifndef MAP_HEADER_FILE_H
#define MAP_HEADER_FILE_H

typedef struct node {
  int key;
  int val;
  struct node * next;
} node_t;

extern node_t** Map_new();
extern void Map_insert(node_t** head, int key, int val);
extern int Map_find(node_t** head, int key);

#endif
