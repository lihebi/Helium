#include <string>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


static int
myrand(int low, int high) {
  if (high < low) return -1; // FIXME -1 is good?
  double d = rand();
  d /= RAND_MAX;
  return low + (high-low)*d;
}

char
rand_char(char low, char high) {
  int dis = high-low;
  char c = low + myrand(0,dis);
  return c;
}

std::string
rand_int(int low, int high) {
  return std::to_string(myrand(low, high));
}

std::string
rand_str(int low_length, int high_length) {
  int bound = myrand(low_length, high_length);
  std::string result;
  for (int i=0;i<bound;i++) {
    result += rand_char('A', 'z');
  }
  return result;
}

/**
 * fill buf with random string of length size
 */
void rand_str(int size, char* buf) {
  for (int i=0;i<size;i++) {
    char c = rand_char('A', 'z');
    buf[i] = c;
  }
}

char global_buf[100];
int main(int argc, char** argv) {
  if (argc != 3) {
    std::cout <<"usage: "  << "\n";
    std::cout <<"executable <type-of-buf> <size-of-src>"  << "\n";
    std::cout <<"type of buffer:"  << "\n";
    std::cout <<"1: static buf"  << "\n";
    std::cout <<"2: stack buf"  << "\n";
    std::cout <<"3: heap buf"  << "\n";
    std::cout <<"4: global buf"  << "\n";
    exit(1);
  }
  // std::cout <<argv[0]  << "\n";
  // std::cout <<argv[1]  << "\n";
  // std::cout <<argv[2]  << "\n";
  static char s_buf[100];
  char buf[100];
  char *d_buf;
  d_buf = (char*)malloc(sizeof(char)*100);
  char *dst;
  int type = 0;
  sscanf(argv[1], "%d", &type);
  // std::cout <<"type size: " << type  << "\n";
  switch (type) {
  case 1: dst = s_buf; break;
  case 2: dst = buf; break;
  case 3: dst = d_buf; break;
  case 4: dst = global_buf; break;
  default: exit(1);
  }
  int size = 0;
  // scanf("%d", &size);
  sscanf(argv[2], "%d", &size);
  // std::cout <<"string length: " << size  << "\n";
  char *src = (char*)malloc(sizeof(char)*size);
  // char src[size];
  rand_str(size-1, src);
  strcpy(dst, src);
}
