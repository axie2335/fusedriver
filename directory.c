#include <assert.h>
#include <string.h>

#include "directory.h"

#include "blocks.h"
#include "inode.h"
#include "slist.h"
#include "bitmap.h"

int rootinode = 0;

/**
 * Initializes root directory of the filesystem
 */
void directory_init() {
  void *bbm = get_blocks_bitmap();
  if (!bitmap_get(bbm, 0)) {
    rootinode = alloc_inode();
    assert(rootinode == 1);
    inode_t *root = get_inode(rootinode);
    root->mode = 040755;
    root->size = 0;
    root->block = alloc_block();
    root->iblock = 0;
    directory_put(root, ".", rootinode);
    bitmap_put(get_inode_bitmap(), rootinode, 1);
  } else {
    rootinode = 1;
  }
}

/**
 * Finds a file in a directory
 *
 * @param di Directory inode to search
 * @param name Name of file requested
 *
 * @return int Inum of requested file, -1 if DNE.
 */
int directory_lookup(inode_t *di, const char *name) {
  assert(di->mode & 040000); //inode should be a directory
  dirent_t *base = (dirent_t *)blocks_get_block(di->block);
  for (int i = 0; i < DIRENT_COUNT; i++) {
    dirent_t *dirent = base + i;
    if (dirent->inum) {
      if (!strcmp(dirent->name, name)) {
        return dirent->inum;
      }
    }
  }
  return -1;
}

/**
 * Adds a new file to a directory
 *
 * @param di Directory inode to modify
 * @param name Name of file requested
 * @param inum Inum of the file to add
 *
 * @return int 0 on success, -1 on failure.
 */
int directory_put(inode_t *di, const char *name, int inum) {
  assert(di->mode & 040000); //inode should be a directory
  dirent_t *base = (dirent_t *)blocks_get_block(di->block);
  for (int i = 0; i < DIRENT_COUNT; i++) {
    dirent_t *dirent = base + i;
    if (!strlen(dirent->name)) {
      dirent->inum = inum;
      memset(dirent->name, 0, DIR_NAME_LENGTH);
      memcpy(dirent->name, name, strlen(name));
      inode_t *node = get_inode(dirent->inum);
      return 0;
    }
  }
  return -1;
}

/**
 * Unlinks file from directory
 *
 * @param di Directory inode to modify
 * @param inum Inum of file to unlink
 *
 * @return int 0 on success, -1 if DNE.
 */
int directory_unlink(inode_t *di, int inum) {
  assert(di->mode & 040000); //inode should be a directory
  dirent_t *entries = (dirent_t *)blocks_get_block(di->block);
  for(int i = 0; i<DIRENT_COUNT; i++){ //iterates over dirent entries, unlinks first node.
    dirent_t *entry = entries + i;
    if(entry->inum == inum){
      inode_t *node = get_inode(inum);
      memset(entry, 0, sizeof(dirent_t));
      return 0;
    }
  }
  return -1; //file not found
}

/**
 * Deletes a file from the directory
 *
 * @param di Directory inode to search
 * @param name Name of file to modify
 *
 * @return int 0 on success, -1 if DNE.
 */
int directory_delete(inode_t *di, const char *name) {
  assert(di->mode & 040000); //inode should be a directory
  dirent_t *entries = (dirent_t *)blocks_get_block(di->block);
  for (int i = 0; i < DIRENT_COUNT; i++) {
    dirent_t *entry = entries + i;
    if (!strcmp(name, entry->name)) {
      inode_t *node = get_inode(entry->inum);
      free_inode(entry->inum);
      memset(entry, 0, sizeof(dirent_t));
      return 0;
    }
  }
  return -1;
}

/**
 * Lists all files in a directory
 *
 * @param di Directory inode to search
 *
 * @return slist_t List of names of files in the directory
 */
slist_t *directory_list(inode_t* di) {
  assert(di->mode & 040000); //inode should be a directory
  slist_t* list = NULL;
  dirent_t *entries = (dirent_t *) blocks_get_block(di->block);
  for(int i = 0; i<DIRENT_COUNT; i++){
    dirent_t *entry = entries + i;
    if(!strlen(entry->name)) continue;
    list = s_cons(entry->name, list);
  }
  return list;
}

/**
 * Lists all files in a directory
 *
 * @param di Directory inode to search
 *
 * @return slist_t List of names of files in the directory
 */
int directory_find(const char *path) {
  if (!strcmp(path, "/")) {
    return rootinode;
  }
  int parent = directory_find_parent(path);
  if (parent < 0) {
    return -1;
  }
  slist_t *list = s_explode(path, '/');
  int inum = directory_lookup(get_inode(parent), s_get_last(list));
  s_free(list);
  return inum;
}

/**
 * Finds the inum of the parent directory of the current path
 *
 * @param path Path of the item
 *
 * @return int Inum of the directory, -1 if DNE.
 */
int directory_find_parent(const char *path) {
  slist_t *list = s_explode(path, '/');
  list = list->next;
  int inum = rootinode;
  inode_t *node = get_inode(rootinode);
  while (list->next) {
    inum = directory_lookup(node, list->data);
    if (inum == -1) {
      return -1;
    }
    node = get_inode(inum);
    list = list->next;
  }
  s_free(list);
  return inum;
}

