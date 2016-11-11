#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *hhaddr[BUFSIZ];
int hhsize[BUFSIZ];
int hhtop = 0;
char hbuf[BUFSIZ];

void input_int(int *var) {
  scanf("%d", var);
}
void output_int(int var, const char *name) {
  printf("int_%s=%d\n", name, var); fflush(stdout);
}
void input_bool(bool *var) {
  int size;
  scanf("%d", size);
  if (size==0) (*var)=false;
  else (*var)=true;
}
void output_bool(bool var, const char *name) {
  printf("bool_%s=%d\n", name, var?1:0); fflush(stdout);
}
void input_char(char *var) {
  scanf("%c", var);
}
void output_char(char var, const char *name) {
  printf("char_%s=%c\n", name, var);
}
void input_char_1(char **var) {
  int size;
  scanf("%d",&size);
  if (size==0) {
    (*var)=NULL;
  } else if (size!=0) {
    (*var)=(char*)malloc(sizeof(char)*size);
    hhaddr[hhtop]=(*var);
    hhsize[hhtop]=size;
    hhtop++;
    scanf("%s", (*var));
  }
}

void input_char_2(char***var) {
  int size;
  scanf("%d",&size);
  if (size==0) {
    (*var)=NULL;
  } else if (size>0) {
    (*var)=(char**)malloc(sizeof(char*)*size);
    hhaddr[hhtop]=(*var);
    hhsize[hhtop]=size;
    hhtop++;
    for (int i=0;i<size;i++) {
      input_char_1(&(*var)[i]);
    }
  }
}

void input_char_3(char ****var) {
  int size;
  scanf("%d", &size);
  if (size>0) {
    (*var)=(char***)malloc(sizeof(char**)*size);
    hhaddr[hhtop]=(*var);
    hhsize[hhtop]=size;
    hhtop++;
    for (int i=0;i<size;i++) {
      input_char_2(&(*var)[i]);
    }
  }
}

void output_char_1(char *var, const char *name) {
  if (var == NULL) {
    printf("isnull_%s=%d\n", name, 1); fflush(stdout);
  } else {
    printf("isnull_%s=%d\n", name, 0); fflush(stdout);
    printf("int_%s.strlen=%ld\n", name, strlen(var)); fflush(stdout);
    printf("addr_%s=%p\n", name, (void*)var); fflush(stdout);
    int size = -1;
    for (int i=0;i<hhtop;i++) {
      if (var == hhaddr[i]) {
        size = hhsize[i]; break;
      }
    }
    if (size != -1) {
      printf("int_%s_heapsize=%d\n", name, size); fflush(stdout);
    }
  }
}



void output_char_2(char **var, const char *name) {
  if (var == NULL) {
    printf("isnull_%s = %d\n", name, 1); fflush(stdout);
  } else {
    printf("isnull_%s = %d\n", name, 0); fflush(stdout);
    int size=-1;
    for (int i=0;i<hhtop;i++) {
      if (var == hhaddr[i]) {
        size = hhsize[i]; break;
      }
    }
    if (size != -1) {
      printf("int_%s_heapsize = %d\n", name, size); fflush(stdout);
      for (int i=0;i<size;i++) {
        sprintf(hbuf, "%s[%d]", name, i);
        output_char_1(var[i], hbuf);
      }
    }
  }
}

void output_char_3(char ***var, const char *name) {
}
