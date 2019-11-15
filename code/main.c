#include "map.h"
#include "user1.h"
#include "user2.h"

int main() {
  node_t* list = new();
  printf("%d\t%d\t%d\n", find(list, 1), find(list, 2), find(list, 3));
  insert(&list, 2, 20);
  printf("%d\t%d\t%d\n", find(list, 1), find(list, 2), find(list, 3));
  insert(&list, 3, 30);
  printf("%d\t%d\t%d\n", find(list, 1), find(list, 2), find(list, 3));
  insert(&list, 1, 10);
  printf("%d\t%d\t%d\n", find(list, 1), find(list, 2), find(list, 3));
  insert(&list, 3, 300);
  printf("%d\t%d\t%d\n", find(list, 1), find(list, 2), find(list, 3));

  init_f();
  init_g();
  /* printf("f(3) is: %d\n", f(3)); */
  /* printf("g(3) is: %d\n", g(3)); */
  printf("f(10) is: %d\n", f(10));
  printf("g(10) is: %d\n", g(10));
  printf("f(10) is: %d\n", g(10));
  printf("g(10) is: %d\n", f(10));
  return 0;
}
