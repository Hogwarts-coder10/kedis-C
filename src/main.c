#include <CDSA/vector.h>
#include <stdio.h>

int main(void) {
  cdsa_vector *vec = cdsa_create_vector(sizeof(int));
  int a = 42, b = 16;

  cdsa_push_vector(vec, &a);
  cdsa_push_vector(vec, &b);
  printf("First item: %d\n", *(int *)get_vector(vec, 0));
  printf("Total size: %zu\n", cdsa_size_vector(vec));

  cdsa_free_vector(vec);
  return 0;
}
