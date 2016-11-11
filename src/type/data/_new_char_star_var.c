#include <stdio.h>

void *helium_heap_addr[BUFSIZ];
int helium_heap_size[BUFSIZ];
int helium_heap_top = 0;
// the size of the found address. Should reset everytime before checking.
int helium_heap_target_size=-1;

void input_1(char *var) {
  int size;
  scanf("%d",&size);
  if (size!=0) {
    var=(char*)malloc(sizeof(char)*size);
    helium_heap_addr[helium_heap_top]=var;
    helium_heap_size[helium_heap_top]=size;
    helium_heap_top++;
    scanf("%s", var);
  }
}

void output_1(char *var) {
  // HELIUM_OUTPUT_BEGIN

  if (var == NULL) {
    printf("isnull_var = %d\n", 1);
    fflush(stdout);
  } else {
    printf("isnull_var = %d\n", 0);
    fflush(stdout);
    printf("int_var.strlen = %ld\n", strlen(var));
    fflush(stdout);
    printf("addr_var = %p\n", (void*)var);
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
        // HeliumHeapCode body
      }
    }
  }
  // HELIUM_OUTPUT_END
}


int main() {
  char *var;
  var=NULL;
  input_1(var);
  output_1(var);
}
