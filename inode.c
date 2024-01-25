#include <string.h>
#include <assert.h>

#include "inode.h"
#include "blocks.h"
#include "bitmap.h"
#include "directory.h"

/**
 * Gets inode of inum
 *
 * @return inode_t* Pointer to inode
 */
inode_t *get_inode(int inum) {
  assert(inum >= 0 && inum <= 255);
  inode_t *table = (inode_t *)get_inode_table();
  inode_t *node = table + inum;
  return node;
}

/**
 * Allocates new inode
 *
 * @return int Inum to new inode, or -1 if failure.
 */
int alloc_inode() {
  // inode 0 used as unininitialized inode
  for (int i = 1; i < INODE_COUNT; ++i) {
    void *ibm = get_inode_bitmap();
    if (!bitmap_get(ibm, i)) {
      bitmap_put(ibm, i, 1);
      return i;
    }
  }
  return -1;
}

/**
 * Frees inode from memory
 *
 * @param inum Node object to be freed
 */
void free_inode(int inum) {
  inode_t *node = get_inode(inum);
  if(node->mode & 040000){
    dirent_t *entries = (dirent_t *)blocks_get_block(node->block);
    for(int i = 0; i < DIRENT_COUNT; i++){
      dirent_t *entry = entries + i;
      if(!strlen(entry->name)) continue;
      free_inode(entry->inum);
    }
    memset(entries, 0, BLOCK_SIZE);
  }
  free_block(node->block);
  if (node->iblock) {
    int *iblock = blocks_get_block(node->iblock);
    for(int i = 0; i < DIRENT_COUNT; i++) {
      if(*(iblock + i)){
        free_block(*(iblock + i));
      }
    }
  }
  memset(node, 0, sizeof(inode_t));
  bitmap_put(get_inode_bitmap(), inum, 0);
}

/**
 * Grows inode to fit data of desired size
 *
 * @param node Node object to be grown
 * @param size Desired final size of the node
 */
void grow_inode(inode_t *node, int size) {
  int curblocks = node->size / BLOCK_SIZE + 1;
  if (!(node->size % BLOCK_SIZE)) {
    curblocks--;
  }
  if (curblocks == 0) {
    curblocks = 1;
  }
  int newblocks = size / BLOCK_SIZE + 1;
  if (!(size % BLOCK_SIZE)) {
    newblocks--;
  }
  int i = 0;
  while (newblocks > curblocks) {
    if (!node->iblock) {
      node->iblock = alloc_block();
      continue;
    }
    int *iblock = blocks_get_block(node->iblock);
    while(*(iblock + i)) { // find next unused block index
      i++;
    }
    *(iblock + i) = alloc_block();
    curblocks++;
  }
  bitmap_print(get_blocks_bitmap(), 256);
}

/**
 * Truncates inode (removes all blocks except default)
 *
 * @param node Node object to be truncated
 *
 */
void truncate_inode(inode_t *node) {
  int curblocks = 1;
  if(!node->iblock) return; // inode is already smallest size
  int *iblock = blocks_get_block(node->iblock);
  for(int i = 0; i< BLOCK_SIZE / sizeof(int); i++){
    if (*(iblock + i)) {
      free_block(*(iblock + i));
      memset(iblock + i, 0, sizeof(int));
    }
  }
  free_block(node->iblock);
  memset(iblock, 0, BLOCK_SIZE);
}

/**
 * Gets bnum (block number) of nth block of inode
 *
 * @param node Inode to access
 * @param file_bnum Nth block of inode
 *
 * @return int Bnum (block number).
 */
int inode_get_bnum(inode_t *node, int file_bnum) {
  if (file_bnum == 0) {
    return node->block;
  }
  int inum = file_bnum - 1;
  int *base = (int *) blocks_get_block(node->iblock);
  return base[inum];
}

