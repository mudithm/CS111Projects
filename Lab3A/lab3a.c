/*
NAME: Mudith Mallajosyula
EMAIL: mudithm@g.ucla.edu
ID: 404937201
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "ext2_fs.h"
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>


// global disk image file descriptor
static int image_fd = -1;

// important variables for traversing the disk image
static __u32 numGroups = 0;
static __u32 block_size = 0;
static __u32 blocks_per_group = 0;
static __u32 inodes_per_group = 0;
static __u32 blocks_count = 0;
static __u32 inodes_count = 0;
static __u32 log_block_size = 0;

// group information for the disk image
static struct ext2_group_desc *group_descriptors = NULL;

// inode information for the disk image
static __u8 *master_inode_bitmap = NULL;


// free dynamically allocated arrays
void free_dynamic_arrays()
{
    if (group_descriptors != NULL)
        free(group_descriptors);
    if (master_inode_bitmap != NULL)
        free(master_inode_bitmap);
}

void __u32_to_time(__u32 time, char *output_buffer, int size)
{
    // create local time
    time_t temp_time = time;
    struct tm delta = *gmtime(&temp_time);
    strftime(output_buffer, size, "%m/%d/%y %H:%M:%S", &delta);
}

// read data from the superblock, and output it as CSV
void read_superblock()
{
    // buffer struct to hold superblock data
    struct ext2_super_block *superblockBuffer = malloc(sizeof(struct ext2_super_block));

    // malloc check
    if (superblockBuffer == NULL)
    {
        fprintf(stderr, "Malloc failed for Superblock read!\n");
        exit(2);
    }

    // read from superblock, with an offset of 1024 bytes
    ssize_t read_val = pread(image_fd, superblockBuffer, sizeof(struct ext2_super_block), 1024);

    // check if read was successful
    if (read_val < 0)
    {
        fprintf(stderr, "Error reading from the superblock.\n");
        free(superblockBuffer);
        exit(2);
    }

    // set values for the number of groups and the block size, etc
    blocks_count = superblockBuffer->s_blocks_count;
    inodes_count = superblockBuffer->s_inodes_count;
    blocks_per_group = superblockBuffer->s_blocks_per_group;
    inodes_per_group = superblockBuffer->s_inodes_per_group;
    numGroups = 1 + (blocks_count - 1) / blocks_per_group;
    log_block_size = superblockBuffer->s_log_block_size;
    block_size = EXT2_MIN_BLOCK_SIZE << log_block_size;

    // Output relevant data as CSV, as follows:
    /*
    SUPERBLOCK
    total number of blocks (decimal)
    total number of i-nodes (decimal)
    block size (in bytes, decimal)
    i-node size (in bytes, decimal)
    blocks per group (decimal)
    i-nodes per group (decimal)
    first non-reserved i-node (decimal)
    */

    printf("SUPERBLOCK,%u,%u,%u,%u,%u,%u,%u\n",
           blocks_count,        // total number of blocks
           inodes_count,        // total number of inodes
           block_size, // block size
           superblockBuffer->s_inode_size,          // size of inodes
           blocks_per_group,    // number of blocks per group
           inodes_per_group,    // number of inodes per group
           superblockBuffer->s_first_ino            // first non-reserved inode
          );

    free(superblockBuffer);
}



// read data from the block group descriptor table, and output it as CSV
void read_groups()
{
    // buffer struct to hold block group descriptor data
    group_descriptors = malloc(numGroups * sizeof(struct ext2_group_desc));

    // malloc check
    if (group_descriptors == NULL)
    {
        fprintf(stderr, "Malloc failed for Block Group Descriptor Table read!\n");
        exit(2);
    }

    // read from group discriptiontable, with an offset of 1024 bytes + 1 block (right after the superblock)
    ssize_t read_val = pread(image_fd, group_descriptors, numGroups * sizeof(struct ext2_group_desc), 1024 + block_size);

    // check if read was successful
    if (read_val < 0)
    {
        fprintf(stderr, "Error reading from block group descriptor table.\n");
        free_dynamic_arrays();
        exit(2);
    }

    // Keep track of the number of blocks remaining, in case there are fewer
    // blocks in the group than the blocks_per_group

    __u32 total_blocks_left = blocks_count;
    __u32 group_blocks_left = blocks_per_group;

    // Do the same as above for inodes

    __u32 total_inodes_left = inodes_count;
    __u32 group_inodes_left = inodes_per_group;

    // travserse through the block group descriptors and
    // output relevant data as CSV, as follows:

    /*
    GROUP
    group number (decimal, starting from zero)
    total number of blocks in this group (decimal)
    total number of i-nodes in this group (decimal)
    number of free blocks (decimal)
    number of free i-nodes (decimal)
    block number of free block bitmap for this group (decimal)
    block number of free i-node bitmap for this group (decimal)
    block number of first block of i-nodes in this group (decimal)
    */

    for (unsigned long i = 0; i < numGroups; i++)
    {
        // check if fewer blocks remain than a full group
        if (total_blocks_left < blocks_per_group)
            group_blocks_left = total_blocks_left;

        // check if fewer inodes remain than a full group
        if (total_inodes_left < inodes_per_group)
            group_inodes_left = total_inodes_left;

        printf("GROUP,%lu,%u,%u,%u,%u,%u,%u,%u\n",
               i, 					// group number
               group_blocks_left,	// total number of blocks in this group
               group_inodes_left,	//	total numbre of inodes in this group
               group_descriptors[i].bg_free_blocks_count, // number of free blocks in group
               group_descriptors[i].bg_free_inodes_count, // number of free inodes in group
               group_descriptors[i].bg_block_bitmap, // block number of block bitmap
               group_descriptors[i].bg_inode_bitmap, // block number of inode bitmap
               group_descriptors[i].bg_inode_table	// block number of first inode block
              );

        // update the number of blocks and inodes remaining
        total_blocks_left -= blocks_per_group;
        total_inodes_left -= inodes_per_group;
    }

}


// read data from the free block bitmap, and output it as CSV
void read_free_block(struct ext2_group_desc gp, unsigned long groupno)
{
    // Output relevant data as CSV, as follows:

    /*
    BFREE
    number of the free block (decimal)
    */

    // unsigned char, to store single bytes of the bitmap
    __u8 block_byte;
    ssize_t read_val = -1;

    // mask equivalent to 0x00000001
    __u8 mask_byte = 1;
    for (unsigned long i = 0; i < block_size; i++)
    {
        // read byte from block bitmap, starting at block specified by bg_block_bitmap
        read_val = pread(image_fd, &block_byte, 1, block_size * gp.bg_block_bitmap + i);

        // check for valid read
        if (read_val < 0)
        {
            fprintf(stderr, "Failed to read from block bitmap.\n");
            free_dynamic_arrays();
            exit(2);
        }

        // reset mask
        mask_byte = 1;

        // check each bit of the byte against the mask.
        // 0 means there is a free block.
        for (unsigned int j = 0; j < 8; j++)
        {
            if (! (block_byte & mask_byte) )
                printf("BFREE,%lu\n", groupno * blocks_per_group + 8 * i + j + 1); // free block number
            mask_byte <<= 1;
        }
    }
}




// handle any directories that are indirectly referenced
void read_indirect_block_directory(struct ext2_inode *parent_inode, __u32 parent_inode_number, __u32 block_address, int level_of_indirectness, __u32 byte_offset)
{
    // number of entries that can exist in the indirect block
    __u32 num_refs = block_size / sizeof(__u32);
    __u32 refs[num_refs];

    // net offset of the block address
    int offset = (1024 + (block_address - 1) * block_size);

    // set the values of the reference array to 0
    memset(refs, 0, num_refs * sizeof(__u32));

    // read the full block containing the indirect references
    ssize_t read_val = pread(image_fd, refs, block_size, offset);
    if (read_val < 0)
    {
        fprintf(stderr, "Failed to read from indirect block\n");
        free_dynamic_arrays();
        exit(2);
    }

    // ensure level of indirectness is between 1 and 3
    if (level_of_indirectness < 0 || level_of_indirectness > 3)
    {
        fprintf(stderr, "Tried to read from an indirect entry with level less than 1 or more than 3\n");
        free_dynamic_arrays();
        exit(2);
    }

    // byte array for the block
    unsigned char block_bytes[block_size];
    struct ext2_dir_entry *block_entry;

    // read the references from the reference array
    for (unsigned int i = 0; i < num_refs; i++)
    {
        // if there is an entry presesnt,
        if (refs[i] != 0)
        {
            // if this is singly indirect
            if (level_of_indirectness == 1)
                ;
            else // doubly or triply indirect
            {
                read_indirect_block_directory(parent_inode, parent_inode_number, refs[i], level_of_indirectness - 1, byte_offset);
            }

            read_val = pread(image_fd, block_bytes, block_size, (1024 + (refs[i] - 1) * block_size));
            if (read_val < 0)
            {
                fprintf(stderr, "Error reading from indirect directory entry\n");
                free_dynamic_arrays();
                exit(2);
            }

            block_entry = (struct ext2_dir_entry *) block_bytes;

            while ((byte_offset < parent_inode->i_size) && block_entry->file_type)
            {
                if (block_entry->inode != 0) {
                    // make sure the name is a proper string
                    char name[block_entry->name_len + 1];
                    memcpy(name, block_entry->name, block_entry->name_len);
                    name[block_entry->name_len] = 0;

                    printf("DIRENT,%u,%u,%u,%u,%u,'%s'\n",
                           parent_inode_number, // inode number of the directory
                           byte_offset,	// number of bytes from the start of the directory
                           block_entry->inode,	// inode number of the directory entry
                           block_entry->rec_len,	// total length of the directory entry
                           block_entry->name_len,	//	length of the entry's name
                           name // name of the directory entry
                          );
                }

                byte_offset += block_entry->rec_len;
                block_entry = (void*) block_entry + block_entry->rec_len;
            }


        }
    }


}


// read data from directory entry, and output it as CSV
void read_directory_entry(struct ext2_inode parent_inode, __u32 parent_inode_number, __u32 block_address)
{
    // Output relevant data as CSV, as follows:

    /*
    DIRENT
    parent inode number (decimal) ... the I-node number of the directory
            that contains this entry
    logical byte offset (decimal) of this entry within the directory
    inode number of the referenced file (decimal)
    entry length (decimal)
    name length (decimal)
    name (string, surrounded by single-quotes). Don't worry about escaping,
            we promise there will be no single-quotes or commas in any of
            the file names.
    */

    struct ext2_dir_entry temp_dir;

    // byte offset of the block address
    unsigned long offset = (1024 + (block_address - 1) * block_size);

    // number of bytes read in the directory
    unsigned long bytes_read = 0;
    ssize_t read_val = -1;
    while (bytes_read < block_size) {
        read_val = pread(image_fd, &temp_dir, sizeof(struct ext2_dir_entry), offset + bytes_read);
        if (read_val < 0)
        {
            fprintf(stderr, "Error reading directory entry\n");
            free_dynamic_arrays();
            exit(2);
        }

        if (temp_dir.inode != 0) {
            // make sure the name is a proper string
            char name[temp_dir.name_len + 1];
            memcpy(name, temp_dir.name, temp_dir.name_len);
            name[temp_dir.name_len] = 0;

            printf("DIRENT,%u,%lu,%u,%u,%u,'%s'\n",
                   parent_inode_number, // inode number of the directory
                   bytes_read,	// number of bytes from the start of the directory
                   temp_dir.inode,	// inode number of the directory entry
                   temp_dir.rec_len,	// total length of the directory entry
                   temp_dir.name_len,	//	length of the entry's name
                   name // name of the directory entry
                  );
        }
        // add total length of dir entry to bytes read
        bytes_read += temp_dir.rec_len;
    }


    // check if the directory entry also contains indirect references
    if (parent_inode.i_block[EXT2_IND_BLOCK] != 0)
        read_indirect_block_directory(&parent_inode, parent_inode_number, parent_inode.i_block[EXT2_IND_BLOCK], 1, bytes_read);
    if (parent_inode.i_block[EXT2_DIND_BLOCK] != 0)
        read_indirect_block_directory(&parent_inode, parent_inode_number, parent_inode.i_block[EXT2_DIND_BLOCK], 2, bytes_read);
    if (parent_inode.i_block[EXT2_TIND_BLOCK] != 0)
        read_indirect_block_directory(&parent_inode, parent_inode_number, parent_inode.i_block[EXT2_TIND_BLOCK], 3, bytes_read);
}

// read data from indirect block, and output it as CSV
void read_indirect_block(__u32 parent_inode_number, __u32 block_address, int level_of_indirectness, __u32 total_offset)
{
    // Output relevant data as CSV, as follows:

    /*
    INDIRECT
    I-node number of the owning file (decimal)
    (decimal) level of indirection for the block being scanned ... 1
            for single indirect, 2 for double indirect, 3 for triple
    logical block offset (decimal) represented by the referenced block.
            If the referenced block is a data block, this is the logical
            block offset of that block within the file. If the referenced
            block is a single- or double-indirect block, this is the same
            as the logical offset of the first data block to which it refers.
    block number of the (1, 2, 3) indirect block being scanned (decimal) . . .
            not the highest level block (in the recursive scan), but the
            lower level block that contains the block reference reported
            by this entry.
    block number of the referenced block (decimal)
    */

    // number of entries that can exist in the indirect block
    __u32 num_refs = block_size / sizeof(__u32);
    __u32 refs[num_refs];

    int offset = (1024 + (block_address - 1) * block_size);

    // set the values of the reference array to 0
    memset(refs, 0, num_refs * sizeof(__u32));

    // read the full block containing the indirect references
    ssize_t read_val = pread(image_fd, refs, block_size, offset);
    if (read_val < 0)
    {
        fprintf(stderr, "Failed to read from indirect block\n");
        free_dynamic_arrays();
        exit(2);
    }

    if (level_of_indirectness < 0 || level_of_indirectness > 3)
    {
        fprintf(stderr, "Tried to read an indirect entry of less than 1 or more than 3\n");
        free_dynamic_arrays();
        exit(2);
    }

    // read the references from the reference array
    for (unsigned int i = 0; i < num_refs; i++)
    {
        // if there is an entry presesnt,
        if (refs[i] != 0)
        {
            printf("INDIRECT,%u,%u,%u,%u,%u\n",
                   parent_inode_number,
                   level_of_indirectness,
                   total_offset + i,
                   block_address,
                   refs[i]
                  );
            // if this is singly indirect
            if (level_of_indirectness == 1)
                total_offset += 0;
            else // doubly or triply indirect
            {
                if (level_of_indirectness == 2)
                    total_offset += 12 * i;
                else
                    total_offset += 256 * i;
                read_indirect_block(parent_inode_number, refs[i], level_of_indirectness - 1, total_offset);
            }
        }
    }


}


// read data from inodes, and output it as CSV
void read_inode_summary(struct ext2_group_desc gp, unsigned long groupno, unsigned long offset_from_block, unsigned int bit_offset)
{
    // Output relevant data as CSV, as follows:

    /*
    INODE
    inode number (decimal)
    file type ('f' for file, 'd' for directory, 's' for symbolic link,
             '?" for anything else)
    mode (low order 12-bits, octal ... suggested format "%o")
    owner (decimal)
    group (decimal)
    link count (decimal)
    time of last I-node change (mm/dd/yy hh:mm:ss, GMT)
    modification time (mm/dd/yy hh:mm:ss, GMT)
    time of last access (mm/dd/yy hh:mm:ss, GMT)
    file size (decimal)
    number of (512 byte) blocks of disk space (decimal) taken up by this
            file
    */

    unsigned long inode_number = groupno * blocks_per_group + 8 * offset_from_block + bit_offset + 1;
    struct ext2_inode temp_inode;
    // get offset of the current block of inodes;
    unsigned long offset = (1024 + (gp.bg_inode_table - 1) * block_size);
    // add offset from this position
    offset += ((offset_from_block * 8) + bit_offset) * sizeof(struct ext2_inode);

    // read inode from offset location
    ssize_t read_val = pread(image_fd, &temp_inode, sizeof(struct ext2_inode), offset);
    if (read_val < 0)
    {
        fprintf(stderr, "Error reading inode!\n");
        free_dynamic_arrays();
        exit(2);
    }

    // if the mode is 0 or the link count is 0, return
    if (temp_inode.i_mode == 0 || temp_inode.i_links_count == 0)
        return;

    // determine the type of the inode
    char type = '?';

    // lower 12 bits of the mode
    __u16 mode = temp_inode.i_mode;
    if (S_ISDIR(mode))
    {   // inode describes a directory
        type = 'd';
    }
    else if (S_ISREG(mode))
    {   // inode describes a regular file
        type = 'f';
    }
    else if (S_ISLNK(mode))
    {   // inode describes a symbolic link
        type = 'd';
    }

    // get time values for the inode
    char creation_time[40], modification_time[40], access_time[40];
    __u32_to_time(temp_inode.i_ctime, creation_time, 40);
    __u32_to_time(temp_inode.i_mtime, modification_time, 40);
    __u32_to_time(temp_inode.i_atime, access_time, 40);


    //printf("-------\nNon-free inode: %lu\n", inode_number);
    //printf("Size: %u Type: %c Mode: %o Blocks: %u\n-------\n", temp_inode.i_size, type, temp_inode.i_mode & 4095, temp_inode.i_blocks);

    printf("INODE,%lu,%c,%o,%u,%u,%u,%s,%s,%s,%u,%u",
           inode_number,
           type,
           mode & 4095,
           temp_inode.i_uid,
           temp_inode.i_gid,
           temp_inode.i_links_count,
           creation_time,
           modification_time,
           access_time,
           temp_inode.i_size,
           temp_inode.i_blocks
          );

    //print out the block addresses of the members of i_block
    for (int i = 0; i < EXT2_N_BLOCKS; i++)
        printf(",%u", temp_inode.i_block[i]);

    printf("\n");

    // if the file is a directory
    if (type == 'd')
    {
        // iterate through the first 12 elements of the i_block array
        for (int i = 0; i < 12; i++)
        {
            if (temp_inode.i_block[i] != 0)
                read_directory_entry(temp_inode, inode_number, temp_inode.i_block[i]);
        }
    }


    // if there are singly indirect entries
    if (temp_inode.i_block[EXT2_IND_BLOCK] != 0)
        read_indirect_block(inode_number, temp_inode.i_block[EXT2_IND_BLOCK], 1, 12); // 0 + 12

    // if there are doubly indirect entries
    if (temp_inode.i_block[EXT2_DIND_BLOCK] != 0)
        read_indirect_block(inode_number, temp_inode.i_block[EXT2_DIND_BLOCK], 2, 268); // 0 + 12 + 256
    // if there are triply indirect entries
    if (temp_inode.i_block[EXT2_TIND_BLOCK] != 0)
        read_indirect_block(inode_number, temp_inode.i_block[EXT2_TIND_BLOCK], 3, 65804); // 0 + 12 + 256 + 65536
}


// read data from inode bitmap, and output it as CSV
void read_free_inode(struct ext2_group_desc gp, unsigned long groupno)
{
    // Output relevant data as CSV, as follows:

    /*
    IFREE
    number of the free I-node (decimal)
    */

    // unsigned char, to store single bytes of the bitmap
    __u8 inode_byte;
    ssize_t read_val = -1;

    // mask equivalent to 0x00000001
    __u8 mask_byte = 1;
    for (unsigned long i = 0; i < inodes_per_group / 8; i++)
    {
        // read byte from block bitmap, starting at block specified by bg_block_bitmap
        read_val = pread(image_fd, &inode_byte, 1, block_size * gp.bg_inode_bitmap + i);

        // add inode byte to the master bitmap
        master_inode_bitmap[groupno * inodes_per_group / 8 + i] = inode_byte;

        // check for valid read
        if (read_val < 0)
        {
            fprintf(stderr, "Failed to read from inode bitmap.\n");
            free_dynamic_arrays();
            exit(2);
        }

        // reset mask
        mask_byte = 1;

        // check each bit of the byte against the mask.
        // 0 means there is a free inode.
        for (unsigned int j = 0; j < 8; j++)
        {
            if (! (inode_byte & mask_byte) )
                printf("IFREE,%lu\n", groupno * blocks_per_group + 8 * i + j + 1); // free inode number
            else
                read_inode_summary(gp, groupno, i, j);
            mask_byte <<= 1;
        }
    }
}






int main(int argc, const char *argv[])
{
    // Check that a disk image filename was passed
    if (argc != 2)
    {
        fprintf(stderr, "Incorrect number of arguments, %d.\n", argc);
        exit(1);
    }

    // Check that the image filename is valid
    const char* disk_image = argv[1];
    image_fd = open(disk_image, O_RDONLY);
    if (image_fd < 0)
    {
        fprintf(stderr, "Unable to open disk image %s\n", disk_image);
        exit(2);
    }

    // read data from the superblock
    read_superblock();

    // read data from the block group descriptor table
    read_groups();

    // initialize master inode bitmap
    master_inode_bitmap = malloc(sizeof(__u32) * numGroups * (inodes_per_group / 8));
    if (master_inode_bitmap == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for the master inode bitmap\n");
        free_dynamic_arrays();
        exit(2);
    }


    for (unsigned long i = 0; i < numGroups; i++)
    {
        read_free_block(group_descriptors[i], i);
        read_free_inode(group_descriptors[i], i);
        //read_inode_summary(group_descriptors[i], i);
    }
    /*

        for (unsigned long i = 0; i < numGroups * (inodes_per_group / 8); i++)
        {
            printf("%u", master_inode_bitmap[i] & 1);
            printf("%u", master_inode_bitmap[i] >> 1 & 1);
            printf("%u", master_inode_bitmap[i] >> 2 & 1);
            printf("%u", master_inode_bitmap[i] >> 3 & 1);
            printf("%u", master_inode_bitmap[i] >> 4 & 1);
            printf("%u", master_inode_bitmap[i] >> 5 & 1);
            printf("%u", master_inode_bitmap[i] >> 6 & 1);
            printf("%u ", master_inode_bitmap[i] >> 7 & 1);
            if (i % 4 == 3)
                printf("\n");
        }
        printf("\n---\n");

        */

    free_dynamic_arrays();
    exit(0);
}