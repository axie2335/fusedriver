// based on cs3650 starter code

#include <assert.h>
#include <bsd/string.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "storage.h"
#include "directory.h"

// implementation for: man 2 access
// Checks if a file exists.
int nufs_access(const char *path, int mask) {
  int rv = 0;
  rv = storage_find(path);
  return rv;
}

// Gets an object's attributes (type, permissions, size, etc).
// Implementation for: man 2 stat
// This is a crucial function.
int nufs_getattr(const char *path, struct stat *st) {
  int rv = 0;
  rv = storage_stat(path, st);
  return rv;
}

// implementation for: man 2 readdir
// lists the contents of a directory
int nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                 off_t offset, struct fuse_file_info *fi) {
  int rv;
  char temp[strlen(path)+15];
  slist_t* list = storage_list(path);
  while(list != NULL){
    struct stat st;
    memset(temp, 0, strlen(path)+DIR_NAME_LENGTH);
    strcpy(temp, path);
    if(strcmp(path, "/")) {
      strcat(temp, "/");
    }
    strcat(temp, list->data);
    int stats = storage_stat(temp, &st);
    filler(buf, list->data, &st, 0);
    list = list->next;
  }
  s_free(list);
  return 0;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
// Note, for this assignment, you can alternatively implement the create
// function.
int nufs_mknod(const char *path, mode_t mode, dev_t rdev) {
  int rv = -1;
  rv = storage_mknod(path, mode);
  return rv;
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int nufs_mkdir(const char *path, mode_t mode) {
  int rv = -1;
  rv = nufs_mknod(path, mode | 040000, 0);
  return rv;
}

int nufs_unlink(const char *path) {
  int rv = -1;
  rv = storage_unlink(path);
  return rv;
}

int nufs_link(const char *from, const char *to) {
  int rv = -1;
  return rv;
}

int nufs_rmdir(const char *path) {
  int rv = -1;
  rv = storage_unlink(path);
  return rv;
}

// implements: man 2 rename
// called to move a file within the same filesystem
int nufs_rename(const char *from, const char *to) {
  int rv = -1;
  rv = storage_rename(from, to);
  return rv;
}

int nufs_chmod(const char *path, mode_t mode) {
  int rv = -1;
  rv = storage_chmod(path, mode);
  return rv;
}

int nufs_truncate(const char *path, off_t size) {
  int rv = 0;
  rv = storage_truncate(path);
  return rv;
}


// This is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
// You can just check whether the file is accessible.
int nufs_open(const char *path, struct fuse_file_info *fi) {
  int rv = -1;
  rv = storage_find(path);
  return rv;
}

// Actually read data
int nufs_read(const char *path, char *buf, size_t size, off_t offset,
              struct fuse_file_info *fi) {
  int rv = 6;
  rv = storage_read(path, buf, size, offset);
  return rv;
}

// Actually write data
int nufs_write(const char *path, const char *buf, size_t size, off_t offset,
               struct fuse_file_info *fi) {
  int rv = -1;
  rv = storage_write(path, buf, size, offset);
  return rv;
}

// Update the timestamps on a file or directory.
int nufs_utimens(const char *path, const struct timespec ts[2]) {
  int rv = -1;
  return rv;
}

// Extended operations
int nufs_ioctl(const char *path, int cmd, void *arg, struct fuse_file_info *fi,
               unsigned int flags, void *data) {
  int rv = -1;
  return rv;
}

void nufs_init_ops(struct fuse_operations *ops) {
  memset(ops, 0, sizeof(struct fuse_operations));
  ops->access = nufs_access;
  ops->getattr = nufs_getattr;
  ops->readdir = nufs_readdir;
  ops->mknod = nufs_mknod;
  ops->mkdir = nufs_mkdir;
  ops->link = nufs_link;
  ops->unlink = nufs_unlink;
  ops->rmdir = nufs_rmdir;
  ops->rename = nufs_rename;
  ops->chmod = nufs_chmod;
  ops->truncate = nufs_truncate;
  ops->open = nufs_open;
  ops->read = nufs_read;
  ops->write = nufs_write;
  ops->utimens = nufs_utimens;
  ops->ioctl = nufs_ioctl;
};

struct fuse_operations nufs_ops;

int main(int argc, char *argv[]) {
  assert(argc > 2 && argc < 6);
  argc--;
  printf("TODO: mount %s as data file\n", argv[argc]);
  storage_init(argv[argc]);
  nufs_init_ops(&nufs_ops);
  return fuse_main(argc, argv, &nufs_ops, NULL);
}
