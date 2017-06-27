#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *helium_heap_addr[BUFSIZ];
int helium_heap_size[BUFSIZ];
int helium_heap_top = 0;
// the size of the found address. Should reset everytime before checking.
int helium_heap_target_size=-1;

struct A {
  int a;
  struct A *b;
};

void input_4(struct A*var) {
  int size;
  scanf("%d",&size);
  if (size!=0) {
    var=(struct A*)malloc(sizeof(struct A)*size);
  }
}

void input_2(struct A *var) {
  int size;
  scanf("%d",&size);
  if (size!=0) {
    var=(struct A*)malloc(sizeof(struct A)*size);
    helium_heap_addr[helium_heap_top]=var;
    helium_heap_size[helium_heap_top]=size;
    helium_heap_top++;
    scanf(var->a);
    var->b=NULL;
    input_4(var->b);
  }
}

void input_1(struct A**var) {
  int size;
  scanf("%d", &size);
  if (size>0) {
    var=(struct A**)malloc(sizeof(struct A*)*size);
    printf("malloc size for addr: %p is %d\n", (void*)var, size);
    helium_heap_addr[helium_heap_top]=var;
    helium_heap_size[helium_heap_top]=size;
    helium_heap_top++;
    for (int i=0;i<size;i++) {
      input_2(var[i]);
    }
  }
}

void output_2(char *var) {
  
}


void output_1(char **var) {
  if (var == NULL) {
    printf("isnull_var = %d\n", 1);
    fflush(stdout);
  } else {
    printf("isnull_var = %d\n", 0);
    fflush(stdout);

    helium_heap_target_size = -1;
    for (int i=0;i<helium_heap_top;i++) {
      if (var == helium_heap_addr[i]) {
        helium_heap_target_size = helium_heap_size[i];
        break;
      }
    }
    if (helium_heap_target_size != -1) {
      printf("int_var_heapsize = %d\n", helium_heap_target_size);
      fflush(stdout);
      for (int i=0,helium_heap_target_size_tmp=helium_heap_target_size;i<helium_heap_target_size_tmp;i++) {
        output_2(var[i]);
      }
    }
  }
}



int main() {
  struct A **var;
  var=NULL;
  input_1(var);
  output_1(var);
}
