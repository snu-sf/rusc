#include <stdio.h>
#include "Map.h"
#include "F.h"
#include "G.h"

int main() {
  void* list = Map_new();
  printf("%d\t%d\t%d\n", Map_find(list, 1), Map_find(list, 2), Map_find(list, 3));
  Map_insert(list, 2, 20);
  printf("%d\t%d\t%d\n", Map_find(list, 1), Map_find(list, 2), Map_find(list, 3));
  Map_insert(list, 3, 30);
  printf("%d\t%d\t%d\n", Map_find(list, 1), Map_find(list, 2), Map_find(list, 3));
  Map_insert(list, 1, 10);
  printf("%d\t%d\t%d\n", Map_find(list, 1), Map_find(list, 2), Map_find(list, 3));
  Map_insert(list, 3, 300);
  printf("%d\t%d\t%d\n", Map_find(list, 1), Map_find(list, 2), Map_find(list, 3));

  F_init();
  G_init();
  /* printf("f(3) is: %d\n", f(3)); */
  /* printf("g(3) is: %d\n", g(3)); */
  printf("f(10) is: %d\n", F_sum(10));
  printf("g(10) is: %d\n", G_sum(10));
  printf("f(10) is: %d\n", G_sum(10));
  printf("g(10) is: %d\n", F_sum(10));
  return 0;
}
