#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include "buffer.h"

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

typedef struct buffer_node_s
{
    size_t data_len;
    uint8_t *data;
    struct buffer_node_s *next;
} buffer_node_t;

typedef struct
{
    size_t offset;
    size_t length;
    buffer_node_t *entry;
    buffer_node_t *tail;
    buffer_node_t *offset_node;
    size_t in_node_offset;
} buffer_t;

buffer_handle_t buffer_create()
{
    buffer_t *buffer = (buffer_t *)malloc(sizeof(buffer_t));
    if (buffer == NULL)
    {
        return NULL;
    }
    memset(buffer, 0, sizeof(buffer_t));
    buffer->entry = NULL;
    buffer->length = 0;
    buffer->offset = 0;
    buffer->offset_node = NULL;
    buffer->in_node_offset = 0;
    return (buffer_handle_t)buffer;
}

int buffer_delete(buffer_handle_t handle)
{
    if (handle == NULL)
    {
        return BUFFER_ERROR;
    }
    buffer_t *buffer = (buffer_t *)handle;
    buffer_node_t *node = buffer->entry;
    while (node != NULL)
    {
        buffer_node_t *node_to_delete = node;
        node = node->next;
        free(node_to_delete);
    }
    free(buffer);
    return BUFFER_OK;
}

size_t buffer_length(buffer_handle_t handle)
{
    if (handle == NULL)
    {
        return 0;
    }
    buffer_t *buffer = (buffer_t *)handle;
    return buffer->length;
}

int buffer_reset_offset(buffer_handle_t handle, size_t offset)
{
    if (handle == NULL)
    {
        return BUFFER_ERROR;
    }
    buffer_t *buffer = (buffer_t *)handle;
    if (offset < buffer->length)
    {
        buffer->offset = offset;
        size_t skip_len = 0;
        buffer_node_t *node = buffer->entry;
        while (true)
        {
            size_t atom_skip_len = MIN(node->data_len, buffer->offset - skip_len);
            skip_len += atom_skip_len;
            if (skip_len == buffer->offset)
            {
                if (atom_skip_len == node->data_len)
                {
                    // skip the current node
                    buffer->offset_node = node->next;
                    buffer->in_node_offset = 0;
                }
                else
                {
                    buffer->offset_node = node;
                    buffer->in_node_offset = atom_skip_len;
                }
                break;
            }
            else
            {
                node = node->next;
            }
        }
    }
    else
    {
        // set to end of the buffer
        buffer->offset = buffer->length;
        buffer->offset_node = NULL;
        buffer->in_node_offset = 0;
        return BUFFER_ERROR_SIZE;
    }
    return BUFFER_OK;
}

int buffer_unshift(buffer_handle_t handle,const uint8_t *data, size_t data_len)
{
    if(handle == NULL)
    {
        return BUFFER_ERROR;
    }
    buffer_t *buffer = (buffer_t *)handle;
    // prepare the new node
    buffer_node_t *new_node = (buffer_node_t *)malloc(sizeof(buffer_node_t) + data_len);
    if(new_node == NULL)
    {
        return BUFFER_ERROR_MEM;
    }
    memset(new_node, 0, sizeof(buffer_node_t));
    new_node->data = (uint8_t *)(new_node + 1);
    new_node->data_len = data_len;
    new_node->next = NULL;
    if (data == NULL)
    {
        memset(new_node->data, 0, new_node->data_len);
    }
    else
    {
        memcpy(new_node->data, data, new_node->data_len);
    }
    // update buffer
    buffer->length += data_len;
    new_node->next = buffer->entry;
    buffer->entry = new_node;
    if(buffer->tail == NULL)
    {
        buffer->tail = new_node;
    }
    // update offset
    // offset does not change !!
    return BUFFER_OK;
}

int buffer_append(buffer_handle_t handle, const uint8_t *data, size_t data_len)
{
    if (handle == NULL)
    {
        return BUFFER_ERROR;
    }
    buffer_t *buffer = (buffer_t *)handle;
    // prepare the new node
    buffer_node_t *new_node = (buffer_node_t *)malloc(sizeof(buffer_node_t) + data_len);
    if (new_node == NULL)
    {
        return BUFFER_ERROR_MEM;
    }
    memset(new_node, 0, sizeof(buffer_node_t));
    new_node->data = (uint8_t *)(new_node + 1);
    new_node->data_len = data_len;
    new_node->next = NULL;
    if (data == NULL)
    {
        memset(new_node->data, 0, new_node->data_len);
    }
    else
    {
        memcpy(new_node->data, data, new_node->data_len);
    }
    // update buffer
    buffer->length += data_len;
    if (buffer->entry == NULL)
    {
        buffer->entry = new_node;
    }
    if (buffer->tail != NULL)
    {
        buffer->tail->next = new_node;
    }
    buffer->tail = new_node;
    // update offset
    if (buffer->offset_node == NULL)
    {
        buffer->offset_node = new_node;
        buffer->in_node_offset = 0;
    }
    return BUFFER_OK;
}

size_t buffer_read(buffer_handle_t handle, uint8_t *dst, size_t dst_len)
{
    if (handle == NULL)
    {
        return 0;
    }
    buffer_t *buffer = (buffer_t *)handle;
    size_t read_len = 0;
    buffer_node_t *node = buffer->offset_node;
    while (read_len < dst_len && buffer->offset < buffer->length)
    {
        size_t atom_read_len = MIN(dst_len - read_len, node->data_len - buffer->in_node_offset);
        memcpy(dst + read_len, node->data + buffer->in_node_offset, atom_read_len);
        read_len += atom_read_len;
        // update offset
        buffer->offset += atom_read_len;
        if (atom_read_len + buffer->in_node_offset == node->data_len)
        {
            // this node is read out
            buffer->offset_node = node->next;
            buffer->in_node_offset = 0;
        }
        else
        {
            buffer->in_node_offset += atom_read_len;
        }
        node = node->next;
    }
    return read_len;
}

int buffer_write(buffer_handle_t handle, uint8_t *src, size_t src_len)
{
    if (handle == NULL)
    {
        return BUFFER_ERROR;
    }
    buffer_t *buffer = (buffer_t *)handle;
    if (buffer->offset + src_len > buffer->length)
    {
        return BUFFER_ERROR_SIZE;
    }
    buffer_node_t *node = buffer->offset_node;
    size_t write_len = 0;
    while (write_len < src_len)
    {
        size_t atom_write_len = MIN(src_len - write_len, node->data_len - buffer->in_node_offset);
        memcpy(node->data + buffer->in_node_offset, src + write_len, atom_write_len);
        write_len += atom_write_len;
        // update offset
        buffer->offset += atom_write_len;
        if (buffer->in_node_offset + atom_write_len == node->data_len)
        {
            buffer->offset_node = node->next;
            buffer->in_node_offset = 0;
        }
        else
        {
            buffer->in_node_offset += atom_write_len;
        }
        node = node->next;
    }
    return BUFFER_OK;
}

int buffer_copy_to_buffer(buffer_handle_t handle, uint8_t *src, size_t src_len, size_t offset)
{
    if (handle == NULL || src == NULL || src_len == 0)
    {
        return BUFFER_ERROR;
    }
    buffer_t *buffer = (buffer_t *)handle;
    if (buffer->length < offset + src_len)
    {
        return BUFFER_ERROR_SIZE;
    }
    // find start node and start in node pos
    buffer_node_t *node = buffer->entry;
    size_t pos = 0;
    size_t in_node_pos = 0;
    while (pos < offset)
    {
        size_t atom_pos = MIN(node->data_len, offset - pos);
        pos += atom_pos;
        if (pos == offset)
        {
            if (atom_pos == node->data_len)
            {
                node = node->next;
                in_node_pos = 0;
            }
            else
            {
                in_node_pos = atom_pos;
            }
            break;
        }
        node = node->next;
    }
    // copy data till the end
    size_t src_pos = 0;
    while (pos < offset + src_len)
    {
        size_t atom_pos = MIN(node->data_len - in_node_pos, offset + src_len - pos);
        memcpy(node->data + in_node_pos, src + src_pos, atom_pos);
        pos += atom_pos;
        src_pos += atom_pos;
        in_node_pos = 0;
        node = node->next;
    }
    return BUFFER_OK;
}

int buffer_copy_from_buffer(buffer_handle_t handle, uint8_t *dst, size_t dst_len, size_t offset)
{
    if (handle == NULL || dst == NULL || dst_len == 0)
    {
        return BUFFER_ERROR;
    }
    buffer_t *buffer = (buffer_t *)handle;
    if (buffer->length < offset + dst_len)
    {
        return BUFFER_ERROR_SIZE;
    }
    // find start node and start in node pos
    buffer_node_t *node = buffer->entry;
    size_t pos = 0;
    size_t in_node_pos = 0;
    while (pos < offset)
    {
        size_t atom_pos = MIN(node->data_len, offset - pos);
        pos += atom_pos;
        if (pos == offset)
        {
            if (atom_pos == node->data_len)
            {
                node = node->next;
                in_node_pos = 0;
            }
            else
            {
                in_node_pos = atom_pos;
            }
            break;
        }
        node = node->next;
    }
    // copy data till the end
    size_t dst_pos = 0;
    while (pos < offset + dst_len)
    {
        size_t atom_pos = MIN(node->data_len - in_node_pos, offset + dst_len - pos);
        memcpy(dst + dst_pos, node->data + in_node_pos, atom_pos);
        pos += atom_pos;
        dst_pos += atom_pos;
        in_node_pos = 0;
        node = node->next;
    }
    return BUFFER_OK;
}

uint8_t *buffer_slice(buffer_handle_t handle, size_t start, size_t end)
{
    if (handle == NULL || start >= end)
    {
        return NULL;
    }
    buffer_t *buffer = (buffer_t *)handle;
    if (buffer->length < end)
    {
        return NULL;
    }
    uint8_t *slice = (uint8_t *)malloc(end - start);
    if (slice == NULL)
    {
        return NULL;
    }
    if (buffer_copy_from_buffer(handle, slice, end - start, start) != BUFFER_OK)
    {
        free(slice);
        return NULL;
    }
    return slice;
}

uint8_t *buffer_to_bytes(buffer_handle_t handle)
{
    if (handle == NULL)
    {
        return NULL;
    }
    buffer_t *buffer = (buffer_t *)handle;
    if (buffer->length == 0)
    {
        return NULL;
    }
    uint8_t *bytes = (uint8_t *)malloc(buffer->length);
    if (bytes == NULL)
    {
        return NULL;
    }
    buffer_node_t *node = buffer->entry;
    size_t offset = 0;
    while (node != NULL)
    {
        memcpy(bytes + offset, node->data, node->data_len);
        offset += node->data_len;
        node = node->next;
    }
    return bytes;
}


