#include <stdio.h>

#define TOKEN_SZ 10
/* #define URI_SZ  */

void escape_absolute_uri (char *uri)
{
  int cp;
  int c;
  char *token[TOKEN_SZ];

  cp = 0;
  c = 0;

  token[0] = uri;
  // TOKEN_SZ >= sizeof(token)
  // TOKEN_SZ >= 
  while (uri[cp] != 0 && c < TOKEN_SZ) {
    if (uri[cp] == '?') {
      // invariant: c > sizeof(token) + 1
      ++c; // off by one
      printf("%d\n", c);
      // invariant: c > sizeof(token)
      /* BAD */
      token[c] = uri + cp + 1;
      uri[cp] = 0;
    }
    ++cp;
  }
  return;
}

int main ()
{
  char uri [] = "la?a??????????????????????????????";
  /* uri [URI_SZ-1] = 0; */
  escape_absolute_uri (uri);
  return 0;
}
