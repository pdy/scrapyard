#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#define KEY_BYTE_SIZE 8 

// static const char *HEX_STR_CHARS = "0123456789AaBbCcDdEeFf";

static void usage(void)
{
  printf("c99_practice <file_with_hex_str_key> <binary_file>\n");
}

static long get_file_size(FILE *file)
{
  fseek(file, 0, SEEK_END);
  const long ret = ftell(file);
  fseek(file, 0, SEEK_SET); 

  return ret;
}

static unsigned long read_file(const char * const filename, char **ret)
{
  FILE *file = fopen(filename, "rb");
  if(!file)
    return 0;

  const long file_size = get_file_size(file);
  *ret = (char*)malloc(sizeof(char) * (unsigned long)file_size);
  if(!ret)
    return 0;

  const unsigned long actual_size = fread(*ret, 1, (unsigned long)file_size, file);
  printf("%s size %lu \n", filename, actual_size); 
  fclose(file);

//  ret = buffer;
  return actual_size;
}

static void print_buffer(const char * const buffer, unsigned long size)
{
  for(unsigned long i = 0; i < size && buffer[i]; ++i)
   printf("%c", buffer[i]);

  printf("\n"); 
}

static void print_as_hexstr(uint8_t *buffer, size_t size)
{
  for(size_t i = 0; i < size; ++i)
    printf("%2x ", buffer[i]);

  printf("\n");
}

static uint8_t hex_char_map(char chr)
{
  switch(chr)
  {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a': return 10;
    case 'b': return 11;
    case 'c': return 12;
    case 'd': return 13;
    case 'e': return 14;
    case 'f': return 15;
  }

  return 0;
}

static void hex_str_to_bytes(const char * const buffer, unsigned long size, uint8_t *ret)
{
  char byte[2] = {0};
  size_t byteTbCnt = 0;

  for(unsigned long i = 0; i < size; ++i)
  {
    byte[byteTbCnt] = buffer[i];
    if(byteTbCnt == 1)
    {
      byteTbCnt = 0;

      const uint8_t lsb = hex_char_map(byte[1]);
      const uint8_t msb = hex_char_map(byte[0]);

      *ret |= lsb;
      *ret |= ((msb << 4) & 0xf0);

      ++ret;
      continue;
    }

    byteTbCnt += 1;
  }
}

static void to_lower(char *buffer, size_t size)
{
  for(size_t i = 0; i < size && buffer[i]; ++i)
    buffer[i] = (char)tolower((int)buffer[i]);
}

static void print_bin(const uint8_t * const buffer, size_t size)
{
  char str[9] = {0};
  str[8] = '\0';
  for(size_t i = 0; i < size && buffer[i]; ++i)
  {
    const uint8_t bt = buffer[i];

    str[0] = bt & 0x80 ? '1' : '0';
    str[1] = bt & 0x40 ? '1' : '0';
    str[2] = bt & 0x20 ? '1' : '0';
    str[3] = bt & 0x10 ? '1' : '0';
    str[4] = bt & 0x08 ? '1' : '0';
    str[5] = bt & 0x04 ? '1' : '0';
    str[6] = bt & 0x02 ? '1' : '0';
    str[7] = bt & 0x01 ? '1' : '0';

    printf("%s ", str);
  }

  printf("\n"); 
}

int main(int argc, char **argv)
{
  if(argc != 3)
  {
    usage();
    return 0;
  }
 
  char *key_file_buffer = NULL;
  const unsigned long key_file_size = read_file(argv[1], &key_file_buffer);
  if(!key_file_size || !key_file_buffer)
  {
    printf("Err readng key file size %lu\n", key_file_size);
    goto end;
  }

  // times 2 cause hex string +1 cause line feed
  if(key_file_size != KEY_BYTE_SIZE * 2 + 1)
  {
    printf("key file size is required to be hex string consisting 16 character\n");
    goto end;
  }

  to_lower(key_file_buffer, key_file_size - 1);
  print_buffer(key_file_buffer, key_file_size);

  uint8_t key_bytes[KEY_BYTE_SIZE] = {0};
  hex_str_to_bytes(key_file_buffer, key_file_size - 1, key_bytes);

  print_as_hexstr(key_bytes, sizeof key_bytes);
  print_bin(key_bytes, sizeof key_bytes);
    
end:
  if(key_file_buffer)
    free(key_file_buffer);

  return 0;
}
