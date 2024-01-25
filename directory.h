// Directory manipulation functions.
//
// Feel free to use as inspiration. Provided as-is.

// Based on cs3650 starter code
#ifndef DIRECTORY_H
#define DIRECTORY_H

#define DIR_NAME_LENGTH 15

#include "blocks.h"
#include "inode.h"
#include "slist.h"

#define DIRENT_COUNT BLOCK_SIZE / sizeof(dirent_t)

typedef struct dirent {
  char name[DIR_NAME_LENGTH];
  char _buffer[16-DIR_NAME_LENGTH];
  int inum;
  //char _reserved[8];
} dirent_t;

/**
 * Initializes root directory of the filesystem
 */
void directory_init();

/**
 * Finds a file in a directory
 *
 * @param di Directory inode to search
 * @param name Name of file requested
 *
 * @return int Inum of requested file, -1 if DNE.
 */
int directory_lookup(inode_t *di, const char *name);

/**
 * Adds a new file to a directory
 *
 * @param di Directory inode to modify
 * @param name Name of file requested
 * @param inum Inum of the file to add
 *
 * @return int 0 on success, -1 on failure.
 */
int directory_put(inode_t *di, const char *name, int inum);

/**
 * Unlinks file from directory
 *
 * @param di Directory inode to modify
 * @param inum Inum of file to unlink
 *
 * @return int 0 on success, -1 if DNE.
 */
int directory_unlink(inode_t *di, int inum);

/**
 * Deletes a file from the directory
 *
 * @param di Directory inode to search
 * @param name Name of file to modify
 *
 * @return int 0 on success, -1 if DNE.
 */
int directory_delete(inode_t *di, const char *name);

/**
 * Lists all files in a directory
 *
 * @param di Directory inode to search
 *
 * @return slist_t List of names of files in the directory
 */
slist_t *directory_list(inode_t* di);

/**
 * Lists all files in a directory
 *
 * @param di Directory inode to search
 *
 * @return slist_t List of names of files in the directory
 */
int directory_find(const char *path);

/**
 * Finds the inum of the parent directory of the current path
 *
 * @param path Path of the item
 *
 * @return int Inum of the directory, -1 if DNE.
 */
int directory_find_parent(const char *path);


#endif
