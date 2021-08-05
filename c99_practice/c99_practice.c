#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#define KEY_SIZE 8 
#define KEY_PC1_SIZE 7 
#define KEY_HEXSTR_LEN (KEY_SIZE * 2)

static const char HEX_STR_CHARS[] = "0123456789AaBbCcDdEeFf";

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
  FILE *file = fopen(filename, "r");
  if(!file)
    return 0;

  const long file_size = get_file_size(file);
  *ret = (char*)malloc(sizeof(char) * (unsigned long)file_size);
  if(!ret)
    return 0;

  const unsigned long actual_size = fread(*ret, 1, (unsigned long)file_size, file);
  printf("%s read size %lu buff size %lu\n", filename, actual_size, file_size); 
  fclose(file);

//  ret = buffer;
  return actual_size;
}

static void print_buffer(const char * const buffer, unsigned long size)
{
  for(unsigned long i = 0; i < size; ++i)
   printf("%c", buffer[i]);

  printf("\n"); 
}

static void print_as_hexstr(uint8_t *buffer, size_t size)
{
  for(size_t i = 0; i < size; ++i)
    printf("%2x ", buffer[i]);

  printf("\n");
}

static int is_valid_hex_str(const char * const buffer, size_t size)
{
  int is_in = 0;
  for(size_t i = 0; i < size; ++i)
  {
    is_in = 0;
    for(size_t j = 0; j < sizeof(HEX_STR_CHARS); ++j)
    {
      if(buffer[i] == HEX_STR_CHARS[j])
      {
        is_in = 1;
        break;
      }
    }

    if(!is_in)
      break;
  }

  return is_in;
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

static void print_bin(const uint8_t * const buffer, size_t size, size_t bit_word_len)
{
  // this would have been much simpler if I wouldn't need to print 7 bit bytes from time to time

  const size_t str_len = size * 8 * sizeof(char);
  char *str = (char*)malloc(str_len);
  for(size_t i = 0; i < size; ++i)
  {
    const uint8_t bt = buffer[i];
    const size_t idx = i * 8;

    str[idx + 0] = bt & 0x80 ? '1' : '0';
    str[idx + 1] = bt & 0x40 ? '1' : '0';
    str[idx + 2] = bt & 0x20 ? '1' : '0';
    str[idx + 3] = bt & 0x10 ? '1' : '0';
    str[idx + 4] = bt & 0x08 ? '1' : '0';
    str[idx + 5] = bt & 0x04 ? '1' : '0';
    str[idx + 6] = bt & 0x02 ? '1' : '0';
    str[idx + 7] = bt & 0x01 ? '1' : '0'; 
  }

  for(size_t i = 0; i < str_len; ++i)
  {
    if(i % bit_word_len == 0 && i != 0)
      printf(" ");
    
    printf("%c", str[i]);
  }

  free(str);
  printf("\n"); 
}

static void print_bin_with_title(const char *title, const uint8_t * const buffer, size_t size, size_t bit_word_len)
{
  printf("%s ", title);
  print_bin(buffer, size, bit_word_len);
}

static void key_pc1(const uint8_t * const buffer, uint8_t *ret)
{
  /*
   *
   * 57   49    41   33    25    17    9
     1    58    50   42    34    26   18
     10    2    59   51    43    35   27
     19   11     3   60    52    44   36
     63   55    47   39    31    23   15
     7    62    54   46    38    30   22
     14    6    61   53    45    37   29
     21   13     5   28    20    12    4
   *
   */
  // division is not needed but it's a little bit more clear for me whats going on
  // it's "hello world" program anyway

  ret[0] |= buffer[57 / 8]      & 0x80;
  ret[0] |= buffer[49 / 8] >> 1 & 0x40;
  ret[0] |= buffer[41 / 8] >> 2 & 0x20;
  ret[0] |= buffer[33 / 8] >> 3 & 0x10;
  ret[0] |= buffer[25 / 8] >> 4 & 0x08;
  ret[0] |= buffer[17 / 8] >> 5 & 0x04;
  ret[0] |= buffer[9  / 8] >> 6 & 0x02;
 
  ret[0] |= buffer[1  / 8] >> 7 & 0x01;
  ret[1] |= buffer[58 / 8] << 2 & 0x80;
  ret[1] |= buffer[50 / 8]      & 0x40;
  ret[1] |= buffer[42 / 8] >> 1 & 0x20;
  ret[1] |= buffer[34 / 8] >> 2 & 0x10;
  ret[1] |= buffer[26 / 8] >> 3 & 0x08;
  ret[1] |= buffer[18 / 8] >> 4 & 0x04;

  ret[1] |= buffer[10 / 8] >> 5 & 0x02;
  ret[1] |= buffer[2  / 8] >> 6 & 0x01;
  ret[2] |= buffer[59 / 8] << 1 & 0x80;
  ret[2] |= buffer[51 / 8] << 1 & 0x40;
  ret[2] |= buffer[43 / 8]      & 0x20;
  ret[2] |= buffer[35 / 8] >> 1 & 0x10;
  ret[2] |= buffer[27 / 8] >> 2 & 0x08;

  ret[2] |= buffer[19 / 8] >> 3 & 0x04;
  ret[2] |= buffer[11 / 8] >> 4 & 0x02;
  ret[2] |= buffer[3  / 8] >> 5 & 0x01;
  ret[3] |= buffer[60 / 8] << 3 & 0x80;
  ret[3] |= buffer[52 / 8] << 2 & 0x40;
  ret[3] |= buffer[44 / 8] << 1 & 0x20;
  ret[3] |= buffer[36 / 8]      & 0x10;

  ret[3] |= buffer[63 / 8] << 2 & 0x08;
  ret[3] |= buffer[55 / 8] << 1 & 0x04;
  ret[3] |= buffer[47 / 8]      & 0x02;
  ret[3] |= buffer[39 / 8] >> 1 & 0x01;
  ret[4] |= buffer[31 / 8] << 6 & 0x80;
  ret[4] |= buffer[23 / 8] << 5 & 0x40;
  ret[4] |= buffer[15 / 8] << 4 & 0x20;

  ret[4] |= buffer[7  / 8] << 3 & 0x10;
  ret[4] |= buffer[62 / 8] << 1 & 0x08;
  ret[4] |= buffer[54 / 8]      & 0x04;
  ret[4] |= buffer[46 / 8] >> 1 & 0x02;
  ret[4] |= buffer[38 / 8] >> 2 & 0x01;
  ret[5] |= buffer[30 / 8] << 5 & 0x80;
  ret[5] |= buffer[22 / 8] << 4 & 0x40;

  ret[5] |= buffer[14 / 8] << 3 & 0x20;
  ret[5] |= buffer[6  / 8] << 2 & 0x10;
  ret[5] |= buffer[61 / 8]      & 0x08;
  ret[5] |= buffer[53 / 8] >> 1 & 0x04;
  ret[5] |= buffer[45 / 8] >> 2 & 0x02;
  ret[5] |= buffer[37 / 8] >> 3 & 0x01;
  ret[6] |= buffer[29 / 8] << 4 & 0x80;

  ret[6] |= buffer[21 / 8] << 3 & 0x40;
  ret[6] |= buffer[13 / 8] << 2 & 0x20;
  ret[6] |= buffer[5  / 8] << 1 & 0x10;
  ret[6] |= buffer[28 / 8] >> 1 & 0x08;
  ret[6] |= buffer[20 / 8] >> 2 & 0x04;
  ret[6] |= buffer[12 / 8] >> 3 & 0x02;
  ret[6] |= buffer[4  / 8] >> 4 & 0x01;
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

  // ------------------------------  + 1 cause line feed
  if(key_file_size != KEY_HEXSTR_LEN + 1)
  {
    printf("key file size is required to be hex string consisting 16 character\n");
    goto end;
  }

  if(!is_valid_hex_str(key_file_buffer, KEY_HEXSTR_LEN))
  {
    printf("%s does not contain valid hex str\n", argv[1]);
    goto end;
  }

  to_lower(key_file_buffer, KEY_HEXSTR_LEN);
  //print_buffer(key_file_buffer, key_file_size);

  uint8_t key_bytes[KEY_SIZE] = {0};
  hex_str_to_bytes(key_file_buffer, KEY_HEXSTR_LEN, key_bytes);

  //print_as_hexstr(key_bytes, sizeof key_bytes);
  print_bin_with_title("K =", key_bytes, KEY_SIZE, 8);

  uint8_t key_pc1_bytes[KEY_PC1_SIZE] = {0};//{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  key_pc1(key_bytes, key_pc1_bytes);
  print_bin_with_title("K+ =", key_pc1_bytes, KEY_PC1_SIZE, 7);
    
end:
  if(key_file_buffer)
    free(key_file_buffer);

  return 0;
}
