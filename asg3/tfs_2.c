//author: Kevin Mody

#include "tfs.h"


/* implementation of assigned functions */


/* you are welcome to use helper functions of your own */



/* tfs_delete()
 *
 * deletes a closed directory entry having the given file descriptor
 *   (changes the status of the entry to unused) and releases all
 *   allocated file blocks
 *
 * preconditions:
 *   (1) the file descriptor is in range
 *   (2) the directory entry is closed
 *
 * postconditions:
 *   (1) the status of the directory entry is set to unused
 *   (2) all file blocks have been set to free
 *
 * input parameter is a file descriptor
 *
 * return value is TRUE when successful or FALSE when failure
 */

unsigned int tfs_delete( unsigned int file_descriptor ){

  /* your code here */
  if (directory[file_descriptor].status == UNUSED)
    {
        return FALSE;
    }
    directory[file_descriptor].status = UNUSED;
    //loop through file allocation table
    unsigned char delete_index = directory[file_descriptor].first_block;
    while (file_allocation_table[delete_index] != LAST_BLOCK)
    {
        unsigned char prev = delete_index;
        delete_index = file_allocation_table[delete_index];
        file_allocation_table[prev] = FREE;
    }
    return TRUE;
}




/* tfs_read()
 *
 * reads a specified number of bytes from a file starting
 *   at the byte offset in the directory into the specified
 *   buffer; the byte offset in the directory entry is
 *   incremented by the number of bytes transferred
 *
 * depending on the starting byte offset and the specified
 *   number of bytes to transfer, the transfer may cross two
 *   or more file blocks
 *
 * the function will read fewer bytes than specified if the
 *   end of the file is encountered before the specified number
 *   of bytes have been transferred
 *
 * preconditions:
 *   (1) the file descriptor is in range
 *   (2) the directory entry is open
 *   (3) the file has allocated file blocks
 *
 * postconditions:
 *   (1) the buffer contains bytes transferred from file blocks
 *   (2) the specified number of bytes has been transferred
 *         except in the case that end of file was encountered
 *         before the transfer was complete
 *
 * input parameters are a file descriptor, the address of a
 *   buffer of bytes to transfer, and the count of bytes to
 *   transfer
 *
 * return value is the number of bytes transferred
 */

unsigned int tfs_read( unsigned int file_descriptor,
                       char *buffer,
                       unsigned int byte_count ){

  /* your code here */
  unsigned short og_byte_offset = directory[file_descriptor].byte_offset;
    unsigned int read_byt = 0;
    // Start at directory[file_descriptor].first_block

    unsigned char reading_index = directory[file_descriptor].first_block;
    // Traverse blocks
    while (read_byt < byte_count)
    {
        for (int i = 0; i < BLOCK_SIZE && read_byt < byte_count; i++)
        {
            // check if EOF has been reached
            char b = blocks[reading_index].bytes[og_byte_offset];
            og_byte_offset++;
            if (og_byte_offset == directory[file_descriptor].size)
            {
                return read_byt;
            }
            // Read into buffer
            buffer[read_byt++] = b;
        }
        reading_index = file_allocation_table[reading_index];
    }

    // Only read og_byte_offset from last_block into buffer
    return read_byt;

}

/* tfs_write()
 *
 * writes a specified number of bytes from a specified buffer
 *   into a file starting at the byte offset in the directory;
 *   the byte offset in the directory entry is incremented by
 *   the number of bytes transferred
 *
 * depending on the starting byte offset and the specified
 *   number of bytes to transfer, the transfer may cross two
 *   or more file blocks
 *
 * furthermore, depending on the starting byte offset and the
 *   specified number of bytes to transfer, additional file
 *   blocks, if available, will be added to the file as needed;
 *   in this case, the size of the file will be adjusted
 *   based on the number of bytes transferred beyond the
 *   original size of the file
 *
 * the function will read fewer bytes than specified if file
 *   blocks are not available
 *
 * preconditions:
 *   (1) the file descriptor is in range
 *   (2) the directory entry is open
 *
 * postconditions:
 *   (1) the file contains bytes transferred from the buffer
 *   (2) the specified number of bytes has been transferred
 *         except in the case that file blocks needed to
 *         complete the transfer were not available
 *   (3) the size of the file is increased as appropriate
 *         when transferred bytes extend beyond the previous
 *         end of the file
 *
 * input parameters are a file descriptor, the address of a
 *   buffer of bytes to transfer, and the count of bytes to
 *   transfer
 *
 * return value is the number of bytes transferred
 */

unsigned char get_new_block_index(unsigned char blk_index)
{
    unsigned char prevBlock = blk_index;
    blk_index = tfs_new_block();
    file_allocation_table[prevBlock] = blk_index;
    file_allocation_table[blk_index] = LAST_BLOCK;
    return blk_index;
}

unsigned int tfs_write(unsigned int file_descriptor,
                       char *buffer,
                       unsigned int byte_count)
{
    unsigned char fBlock = directory[file_descriptor].first_block;
    directory[file_descriptor].size += byte_count;
    unsigned int wrote_byt = 1;
    // if creating new file
    if (fBlock == FREE)
    {
        // Get new block for file
        unsigned int newFat = tfs_new_block();
        file_allocation_table[newFat] = LAST_BLOCK;
        directory[file_descriptor].first_block = newFat;
        // write up to 128 bytes to block[index].bytes
        // reassign newFat every 128 bytes
        while (wrote_byt < byte_count)
        {
            if (wrote_byt % BLOCK_SIZE == 0)
            {
                newFat = get_new_block_index(newFat);
            }
            int bytes_index = wrote_byt % BLOCK_SIZE;
            blocks[newFat].bytes[bytes_index] = buffer[wrote_byt];
            //printf("1. Currently in Block (%d) at offset (%d)\n", newFat, wrote_byt % BLOCK_SIZE);
            wrote_byt++;
        }
    }
        // else if writing to existing file,
    else
    {
        // then traverse thru FAT and find what "next" points to
        // at end of loop, fat[index_to_write] = first block with space
        unsigned char blk_index = fBlock;
        while (file_allocation_table[blk_index] != LAST_BLOCK)
        {
            blk_index = file_allocation_table[blk_index];
        }
        // write as much data as possible, add more blocks if needed

        unsigned int bytes_left = BLOCK_SIZE - directory[file_descriptor].byte_offset;
        
        // fill in rest of last block
        if (wrote_byt <= bytes_left)
        {
            for (unsigned int i = BLOCK_SIZE - bytes_left; i < BLOCK_SIZE && wrote_byt < byte_count; i++)
            {
                blocks[blk_index].bytes[i] = buffer[wrote_byt++];
                //printf("2. Currently in Block (%d) at offset (%d)\n", blk_index, wrote_byt % BLOCK_SIZE);
            }
        }

        // if there's more left over, create a new block
        while (wrote_byt < byte_count)
        {
            if ((wrote_byt - bytes_left) % BLOCK_SIZE == 0)
            {
                blk_index = get_new_block_index(blk_index);
            }
            int index = (wrote_byt - bytes_left) % BLOCK_SIZE;
            blocks[blk_index].bytes[index] = buffer[wrote_byt++];
            //printf("3. Currently in Block (%d) at offset (%d)\n", blk_index, index);
        }
    }
    directory[file_descriptor].byte_offset = wrote_byt % BLOCK_SIZE;
    return wrote_byt;
}
