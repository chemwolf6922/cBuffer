#include <stdio.h>
#include <stdint.h>
#include "buffer.h"

int main(int argc, char** argv)
{
    buffer_handle_t buffer = buffer_create();
    buffer_delete(buffer);
    return 0;
}