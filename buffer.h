#ifndef __BUFFER_H
#define __BUFFER_H

#include <stdint.h>
#include <stddef.h>

typedef void *buffer_handle_t;

enum
{
    BUFFER_OK = 0,
    BUFFER_ERROR = -1,
    BUFFER_ERROR_MEM = -2,
    BUFFER_ERROR_SIZE = -3,
};

/*
    create a buffer. return NULL if failed
*/
buffer_handle_t buffer_create();
/*
    delete a buffer. 
*/
int buffer_delete(buffer_handle_t handle);
/*
    reutrn size of the buffer
*/
size_t buffer_length(buffer_handle_t handle);
/*
    Append data to buffer.
    If data is NULL, append zeros to buffer.
*/
int buffer_append(buffer_handle_t handle, uint8_t *data, size_t data_len);
/*
    reset read/write position to offset
*/
int buffer_reset_offset(buffer_handle_t handle, size_t offset);
/*
    try to read dst_len bytes of data from buffer at internal offset to dst.
    return number of bytes read.
*/
size_t buffer_read(buffer_handle_t handle, uint8_t *dst, size_t dst_len);
/*
    Write src_len bytes of data to buffer at internal offset from src. 
*/
int buffer_write(buffer_handle_t handle, uint8_t *src, size_t src_len);
/*
    copy a slice of buffer out. Return NULL if failed. 
    Notice: the returned slice needs to be freed outside
*/
uint8_t *buffer_slice(buffer_handle_t handle, size_t start, size_t end);
/*
    create a copy of the buffer in a byte array.
    Notice: the returned byte array needs to be freed outside
*/
uint8_t *buffer_to_bytes(buffer_handle_t handle);
/*
    copy src_len bytes from src to buffer at offset.
*/
int buffer_copy_to_buffer(buffer_handle_t handle, uint8_t *src, size_t src_len, size_t offset);
/*
    copy dst_len bytes from buffer at offset to dst.
*/
int buffer_copy_from_buffer(buffer_handle_t handle, uint8_t *dst, size_t dst_len, size_t offset);

#endif