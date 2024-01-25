// Inode manipulation routines.
//
// Feel free to use as inspiration. Provided as-is.

// based on cs3650 starter code
#ifndef INODE_H
#define INODE_H

#include <stdint.h>

#include "blocks.h"

#define INODE_COUNT 256

typedef struct inode {
  int mode;       // permission & type
  int size;       // bytes
  uint8_t block;  // single block pointer (if max file size <= 4K or directory)
  uint8_t iblock; // indirect block pointer
} inode_t;

/**
 * Gets inode of inum
 *
 * @return inode_t* Pointer to inode
 */
inode_t *get_inode(int inum);

/**
 * Allocates new inode
 *
 * @return int Inum to new inode, or -1 if failure.
 */
int alloc_inode();

/**
 * Frees inode from memory
 *
 * @param inum Node object to be freed
 */
void free_inode(int inum);

/**
 * Grows inode to fit data of desired size
 *
 * @param node Node object to be grown
 * @param size Desired final size of the node
 */
void grow_inode(inode_t *node, int size);

/**
 * Truncates inode (removes all blocks except default)
 *
 * @param node Node object to be truncated
 */
void truncate_inode(inode_t *node);

/**
 * Gets bnum (block number) of nth block of inode
 *
 * @param node Inode to access
 * @param file_bnum Nth block of inode
 *
 * @return int Bnum (block number).
 */
int inode_get_bnum(inode_t *node, int file_bnum);


#endif
