#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "buffer.h"

int main(int argc, char** argv)
{
    buffer_handle_t buffer = buffer_create();
    char test_string_A[] = "12345678";
    char test_string_B[] = "ABCDEFGH";
    buffer_append(buffer,test_string_A,strlen(test_string_A));
    buffer_append(buffer,test_string_B,strlen(test_string_B));
    buffer_append(buffer,test_string_A,strlen(test_string_A));
    buffer_unshift(buffer,test_string_A,strlen(test_string_A));
    buffer_append(buffer,test_string_B,strlen(test_string_B));
    buffer_append(buffer,test_string_A,strlen(test_string_A));
    buffer_append(buffer,test_string_B,strlen(test_string_B));
    buffer_append(buffer,test_string_A,strlen(test_string_A));
    buffer_append(buffer,test_string_B,strlen(test_string_B));
    buffer_unshift(buffer,test_string_A,strlen(test_string_A));
    size_t prev_len = buffer_length(buffer);
    printf("buffer len: %ld\n",prev_len);
    buffer_append(buffer,NULL,20);
    buffer_reset_offset(buffer,prev_len);
    char test_string_C[] = "abcdefghij";
    buffer_write(buffer,test_string_C,5);
    buffer_write(buffer,test_string_A,5);
    buffer_write(buffer,test_string_B,5);
    buffer_write(buffer,test_string_C,5);
    char output_string[20] = {0};
    size_t read_len = 0;
    buffer_reset_offset(buffer,0);
    do
    {
        memset(output_string,0,sizeof(output_string));
        read_len = buffer_read(buffer,output_string,10);
        printf("%ld: %s\n",read_len,output_string);
    } while (read_len != 0);
    buffer_reset_offset(buffer,20);
    do
    {
        memset(output_string,0,sizeof(output_string));
        read_len = buffer_read(buffer,output_string,10);
        printf("%ld: %s\n",read_len,output_string);
    } while (read_len != 0);
    buffer_reset_offset(buffer,16);
    do
    {
        memset(output_string,0,sizeof(output_string));
        read_len = buffer_read(buffer,output_string,10);
        printf("%ld: %s\n",read_len,output_string);
    } while (read_len != 0);
    buffer_reset_offset(buffer,3);
    char test_string_D[] = "!@#$^&*(";
    while(true)
    {
        int ret = buffer_write(buffer,test_string_D,strlen(test_string_D));
        if(ret != BUFFER_OK)
        {
            break;
        }
    }
    buffer_reset_offset(buffer,0);
    do
    {
        memset(output_string,0,sizeof(output_string));
        read_len = buffer_read(buffer,output_string,10);
        printf("%ld: %s\n",read_len,output_string);
    } while (read_len != 0);
    buffer_delete(buffer);
    return 0;
}