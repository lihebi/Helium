#include <string.h>

char buf[1024];

void bar(char *a, char *b) {
  char *s=NULL;
  int c=0;
  /* legacy(); */
  /* for (char *p=a;p!='\0';p++) { */
  /*   *p='y'; */
  /*   if (p) break; */
  /* } */
  strcpy(buf, "short");
  strcat(a,b);
  if(strlen(b)<1024) {
    s=b;
  } else {
    s=a;
  }
  dep();
  if (c>8) {
    c=9;
  }
  strcpy(buf, s);
}

void foo(char *str) {
  char *newstr="hello";
  bar(newstr, str);
}

int main(int argc, char *argv[]) {
  if (argc==2) {
    foo(argv[1]);
  } else if (argc==3) {
    bar(argv[1], argv[2]);
  }
}

