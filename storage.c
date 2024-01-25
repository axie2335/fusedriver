#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "storage.h"

#include "slist.h"
#include "blocks.h"
#include "inode.h"
#include "directory.h"
#include "bitmap.h"

/**
 * Initializes filesystem with image
 *
 * @param path Path to image file
 */
void storage_init(const char *path) {
  blocks_init(path);
}

/**
 * Checks existence of item
 *
 * @param path Item to be found
 *
 * @return int 0 if exists, -1 if doesn't
 */
int storage_find(const char *path) {
  return directory_find(path) ? 0 : -1;
}

/**
 * Gets attributes of file (currently just mode and size)
 *
 * @param path Item to be found
 * @param stat Structure for data return
 *
 * @return int 0 on success, -2 if DNE
 */
int storage_stat(const char *path, struct stat *st) {
  int inum = directory_find(path);
  if(inum < 0 || inum > INODE_COUNT) {
    return -2; //ENOENT = 2
  }
  inode_t *node = get_inode(inum);
  st->st_mode = node->mode;
  st->st_size = node->size;
  return 0;
}

/**
 * Reads data from file
 *
 * @param path File to be read
 * @param buf Data buffer
 * @param size Size of data to be read
 * @param offset Offset to be read from
 *
 * @return int Bytes read
 */
int storage_read(const char *path, char *buf, size_t size, off_t offset) {
  int inum = directory_find(path);
  if(inum < 0) return -2; //ENOENT (file does not exist)
  inode_t *node = get_inode(inum);
  assert(!(node->mode & 040000)); //file should NOT be a directory

  int block = offset / BLOCK_SIZE;
  int blockoffset = offset % BLOCK_SIZE;
  size_t left = size;

  if (offset + left > node->size) {
    left = node->size - offset;
  }

  int endblock = (offset + left) / BLOCK_SIZE;
  if ((offset + left) % BLOCK_SIZE == 0) {
    endblock--;
  }
  size_t read = 0;

  for (int i = block; i <= endblock; i++) {
    char *blockptr = blocks_get_block(inode_get_bnum(node, i));
    int readsize = left > BLOCK_SIZE ? BLOCK_SIZE : left;
    if (blockoffset) {
      int readsize = readsize + blockoffset > BLOCK_SIZE ? BLOCK_SIZE - blockoffset : readsize;
      memcpy(buf, blockptr + blockoffset, readsize);
      read += readsize;
      left -= readsize;
      blockoffset = 0;
    } else {
      memcpy(buf + read, blockptr, readsize);
      read += readsize;
      left -= readsize;
    }
  }
  return (int)read;
}

/**
 * Writes data to file
 *
 * @param path File to be written to
 * @param buf Data buffer
 * @param size Size of data to write
 * @param offset Offset to write to
 *
 * @return int Bytes written
 */
int storage_write(const char *path, const char *buf, size_t size, off_t offset) {
  int inum = directory_find(path);
  if(inum < 0) return -2; //ENOENT (file does not exist)
  inode_t *node = get_inode(inum);
  assert(!(node->mode & 040000)); //file should NOT be a directory
  grow_inode(node, offset + size);

  int block = offset / BLOCK_SIZE;
  int blockoffset = offset % BLOCK_SIZE;
  size_t left = size;

  int endblock = (offset + left) / BLOCK_SIZE;
  if ((offset + left) % BLOCK_SIZE == 0) {
    endblock--;
  } 
  size_t written = 0;
  for (int i = block; i <= endblock; i++) {
    int bnum = inode_get_bnum(node, i);
    char *blockptr = blocks_get_block(bnum);
    int writesize = left > BLOCK_SIZE ? BLOCK_SIZE : left;
    if (blockoffset) {
      int writesize = writesize + blockoffset > BLOCK_SIZE ? BLOCK_SIZE - blockoffset : writesize;
      memcpy(blockptr + blockoffset, buf, writesize); 
      written += writesize;
      left -= writesize;
      blockoffset = 0;
    } else {
      memcpy(blockptr, buf + written, writesize);
      written += writesize;
      left -= writesize;
    }
  }
  node->size = size + offset;
  return (int)written;
}

/**
 * Creates node
 *
 * @param path Path of node to be created
 * @param mode Mode of node
 *
 * @return int 0 on success, -1 on failure.
 */
int storage_mknod(const char *path, mode_t mode) {
  int rv = -1;
  slist_t *list = s_explode(path, '/');
  char *last = s_get_last(list);

  if (strlen(last) > DIR_NAME_LENGTH) {
    return -1;
  }

  int inum = alloc_inode();
  int block = alloc_block();

  inode_t *node = get_inode(inum);
  node->block = block;
  node->mode = mode;
  node->size = 0;
  node->iblock = 0;
  int parentinum = directory_find_parent(path);

  inode_t *parent = get_inode(parentinum);

  rv = directory_put(parent, last, inum);

  s_free(list);

  return rv;
}

/**
 * Deletes item
 *
 * @param path Path of item to be deleted
 *
 * @return int 0 on success, -1 on failure.
 */
int storage_unlink(const char *path) {
  int rv = -1;
  int inum = directory_find_parent(path);
  inode_t *node = get_inode(inum);
  slist_t *list = s_explode(path, '/');
  rv = directory_delete(node, s_get_last(list));
  s_free(list);
  return rv;
}

/**
 * Renames item (allows moving to different path)
 *
 * @param from Path of item to be renamed
 * @param to Path of item after rename
 *
 * @return int 0 on success, -1 on failure.
 */
int storage_rename(const char *from, const char *to) {
  int inum = directory_find(from);
  inode_t *fromdirnode = get_inode(directory_find_parent(from));
  inode_t *todirnode = get_inode(directory_find_parent(to));
  slist_t *fromlist = s_explode(from, '/');
  slist_t *tolist = s_explode(to, '/');

  int sv = directory_unlink(fromdirnode, inum) | directory_put(todirnode, s_get_last(tolist), inum);

  s_free(fromlist);
  s_free(tolist);
  return sv;
}

/**
 * Change permissions of item
 *
 * @param path Path of item to be modified
 * @param mode Mode to set
 *
 * @return int 0 on success, -1 on failure.
 */
int storage_chmod(const char *path, mode_t mode) {
  int inum = directory_find(path);
  if (inum == -1) {
    return -1;
  }
  inode_t *node = get_inode(inum);
  node->mode = mode;
  return 0;
}

/**
 * Truncates file
 *
 * @param path Path of item to be modified
 * @param mode Mode to set
 *
 * @return int 0 on success, -1 on failure.
 */
int storage_truncate(const char *path) {
  int inum = directory_find(path);

  slist_t *list = s_explode(path, '/');
  char *last = s_get_last(list);
  if (strlen(last) > DIR_NAME_LENGTH) {
    return 0;
  }
  if (inum == -1) {
    return -1;
  }
  inode_t *node = get_inode(inum);
  assert(!(node->mode & 040000)); //file should NOT be directory
  truncate_inode(node);
  return 0;
}

/**
 * Lists directory contents
 *
 * @param path Path of directory
 *
 * @return slist_t* Pointer to list of items
 */
slist_t* storage_list(const char *path){
  int inum = directory_find(path);
  inode_t *di = get_inode(inum);
  return directory_list(di);
}