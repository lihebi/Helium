// Output:
// PointerType::GetOutputCode: var contained type: CharTypelevel: 0
printf("int_var.strlen = %ld\n", strlen(var));
fflush(stdout);
printf("addr_var = %p\n", (void*)var);
fflush(stdout);
helium_heap_target_size = -1;
for (int =0;<helium_heap_top;++) {
  if (var == helium_heap_addr[]) {
    helium_heap_target_size = helium_heap_size[];
    break;
  }
}
if (helium_heap_target_size != -1) {
  printf("int_var_heapsize = %d\n", helium_heap_target_size);
fflush(stdout);
  for (int =0,helium_heap_target_size_tmp=helium_heap_target_size;<helium_heap_target_size_tmp;++) {
 }
}if (var == NULL) {
printf("isnull_var = 1\n");
fflush(stdout);
} else {
printf("isnull_var = 0\n");
fflush(stdout);
}

// Input:
// PointerType::GetInputCode: var
scanf("%d", &helium_size);
if (helium_size == 0) {
var = NULL;
} else {
var = (char*)malloc(sizeof(char)*helium_size);
printf("malloc size for addr: %p is %d\n", (void*)var, helium_size);
helium_heap_addr[helium_heap_top]=var;
helium_heap_size[helium_heap_top]=helium_size;
helium_heap_top++;
scanf("%s", var);
}

