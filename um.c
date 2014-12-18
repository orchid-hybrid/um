#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef uint32_t platter;

typedef struct {
  platter id;
  platter length;
  platter *elt;
} array;

typedef struct {
  platter r[8];
  platter finger;
  platter arrays;
  array *a;
} um;

platter reverse_endian(platter p) {
  // http://stackoverflow.com/questions/2182002/convert-big-endian-to-little-endian-in-c-without-using-provided-func
  p = ((p << 8) & 0xFF00FF00 ) | ((p >> 8) & 0xFF00FF ); 
  return (p << 16) | (p >> 16);
}

void um_init(um *u, long length, void *buffer) {
  int i;
  
  u->r[0] = 0;
  u->r[1] = 0;
  u->r[2] = 0;
  u->r[3] = 0;
  u->r[4] = 0;
  u->r[5] = 0;
  u->r[6] = 0;
  u->r[7] = 0;
  
  u->finger = 0;
  
  u->arrays = 1;
  
  u->a = malloc(sizeof(array)*u->arrays);
  u->a[0].id = 0;
  u->a[0].length = length;
  u->a[0].elt = buffer;

  for(i = 0; i < length; i++) {
    u->a[0].elt[i] = reverse_endian(u->a[0].elt[i]);
  }
}

int um_instruction_decode(um *u, platter p, int debug) {
  platter op;
  platter a,b,c;
  platter value;

  op = p >> (32-4);

  //if(debug) puts("");
  if(debug) fprintf(stderr, "[%02d] ", op);

  a = (p & 0b111000000) >> 6;
  b = (p & 0b000111000) >> 3;
  c = (p & 0b000000111) >> 0;
  
  switch(op) {
  case 0:
    if(debug) fprintf(stderr, "cmov %d %d %d\n", a, b, c);
    if(u->r[c]) {
      u->r[a] = u->r[b];
    }
    u->finger++;
    break;
  case 1:
    if(debug) fprintf(stderr, "arr %d %d %d\n", a, b, c);
    if(u->r[b] >= u->arrays) {
      printf("Array out of bounds! %d %d", u->r[b], u->arrays);
      return 1;
    }
    if(u->r[c] >= u->a[u->r[b]].length) {
      printf("Array index out of bounds! %d %d", u->r[c], u->a[u->r[b]].length);
      return 1;
    }
    //if(debug) fprintf(stderr, "its %u\n", u->r[a]);
    u->r[a] = u->a[u->r[b]].elt[u->r[c]];
    u->finger++;
    break;
  case 2:
    if(debug) fprintf(stderr, "arr %d %d %d\n", a, b, c);
    if(u->r[a] >= u->arrays) {
      printf("Array out of bounds! %d %d", u->r[a], u->arrays);
      return 1;
    }
    if(u->r[b] >= u->a[u->r[a]].length) {
      printf("Array index out of bounds! %d %d", u->r[b], u->a[u->r[a]].length);
      return 1;
    }
    if(debug) fprintf(stderr, "its %u\n", u->r[a]);
    u->a[u->r[a]].elt[u->r[b]] = u->r[c];
    u->finger++;
    break;
  case 3:
    if(debug) fprintf(stderr, "add %d %d %d\n", a, b, c);
    u->r[a] = u->r[b] + u->r[c];
    u->finger++;
    break;
  case 4:
    if(debug) fprintf(stderr, "mul %d %d %d\n", a, b, c);
    u->r[a] = u->r[b] * u->r[c];
    u->finger++;
    break;
  case 5:
    if(debug) fprintf(stderr, "div %d %d %d\n", a, b, c);
    u->r[a] = u->r[b] / u->r[c];
    u->finger++;
    break;
  case 6:
    if(debug) fprintf(stderr, "nand %d %d %d\n", a, b, c);
    u->r[a] = ~(u->r[b] & u->r[c]);
    u->finger++;
    break;
  case 7:
    if(debug) fprintf(stderr, "halt %d %d %d\n", a, b, c);
    return 1;
    break;
  case 8:
    if(debug) fprintf(stderr, "alloc %d %d %d\n", a, b, c);
    { platter i;
      i = u->arrays;
      u->arrays++;
      if(u->arrays >= 0xffffffff) {
        fprintf(stderr, "Ran out of space for arrays!");
        return 1;
      }
      u->a = realloc(u->a, sizeof(array)*u->arrays);
      u->a[i].id = i;
      u->a[i].length = u->r[c];
      u->a[i].elt = calloc(u->r[c], sizeof(platter));
      
      u->r[b] = u->a[i].id;
      u->finger++;
    }
    break;
  case 9:
    if(debug) fprintf(stderr, "aband %d %d %d\n", a, b, c);
    { platter i;
      i = u->r[c];
      if(i > u->arrays) {
        fprintf(stderr, "Out of bounds array abandonment! %d", i, u->arrays);
        return 1;
      }
      u->a[i].id = 0;
      u->a[i].length = 0;
      free(u->a[i].elt);
      u->a[i].elt = NULL;
      
      u->finger++;
    }
    break;
  case 10:
    if(debug) fprintf(stderr, "out %d %d %d\n", a, b, c);
    if(u->r[c] <= 255) {
      putchar(u->r[c]);
    }
    else {
      printf("Error! Tried to output %d\n", u->r[c]);
      return 1;
    }
    u->finger++;
    break;
  case 12:
    if(debug) fprintf(stderr, "prg %d %d %d\n", a, b, c);
    if(debug) fprintf(stderr, "(%d) (%d) (%d)\n", u->r[a], u->r[b], u->r[c]);
    if(u->r[b] >= u->arrays) {
      printf("Array out of bounds! %d %d", u->r[b], u->arrays);
      return 1;
    }
    if(u->r[b] != 0) {
      if(u->a[0].length < u->a[u->r[b]].length) {
        free(u->a[0].elt);
        u->a[0].elt = malloc(sizeof(platter)*u->a[u->r[b]].length);
      }
      memcpy(u->a[0].elt, u->a[u->r[b]].elt, sizeof(platter)*u->a[u->r[b]].length);
      u->a[0].length = u->a[u->r[b]].length;
    }
    u->finger = u->r[c];
    break;
    
  case 13:
    a = p >> (32-4-3) & 0b111;
    value = p & 0b00000001111111111111111111111111;
    if(debug) fprintf(stderr, "orth %d %d\n", a, value);
    u->r[a] = value;
    u->finger++;
    break;
  default:
    fprintf(stderr, "Unknown operator %d\n", op);
    return 1;
  }

  return 0;
}

int main(int argc, char **argv) {
  FILE *fptr;
  long file_size;
  void *buffer;

  um u;
  
  if(argc != 2) {
    puts("Pass a umz file");
    return EXIT_FAILURE;
  }
  
  fptr = fopen(argv[1], "r");
  if(!fptr) {
    puts("Cannot open umz file");
    return EXIT_FAILURE;
  }
  
  fseek(fptr, 0, SEEK_END);
  file_size = ftell(fptr);
  fseek(fptr, 0, SEEK_SET);

  if(file_size%4 != 0) {
    puts("File size is not a mutliple of 4");
    return EXIT_FAILURE;
  }

  buffer = malloc(file_size);
  fread(buffer, file_size, 1, fptr);

  um_init(&u, file_size/4, buffer);
  while(u.finger <= u.a[0].length
        && !um_instruction_decode(&u, u.a[0].elt[u.finger], 0)) {}
  
  return EXIT_SUCCESS;
}
