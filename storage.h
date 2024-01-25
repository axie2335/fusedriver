// Disk storage abstracttion.
//
// Feel free to use as inspiration. Provided as-is.

// based on cs3650 starter code

#ifndef NUFS_STORAGE_H
#define NUFS_STORAGE_H

#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "slist.h"

/**
 * Initializes filesystem with image
 *
 * @param path Path to image file
 */
void storage_init(const char *path);

/**
 * Checks existence of item
 *
 * @param path Item to be found
 *
 * @return int 0 if exists, -1 if doesn't
 */
int storage_find(const char *path);

/**
 * Gets attributes of file (currently just mode and size)
 *
 * @param path Item to be found
 * @param stat Structure for data return
 *
 * @return int 0 on success, -2 if DNE
 */
int storage_stat(const char *path, struct stat *st);

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
int storage_read(const char *path, char *buf, size_t size, off_t offset);

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
int storage_write(const char *path, const char *buf, size_t size, off_t offset);
/**
 * Creates node
 *
 * @param path Path of node to be created
 * @param mode Mode of node
 *
 * @return int 0 on success, -1 on failure.
 */
int storage_mknod(const char *path, mode_t mode);

/**
 * Deletes item
 *
 * @param path Path of item to be deleted
 *
 * @return int 0 on success, -1 on failure.
 */
int storage_unlink(const char *path);

/**
 * Renames item (allows moving to different path)
 *
 * @param from Path of item to be renamed
 * @param to Path of item after rename
 *
 * @return int 0 on success, -1 on failure.
 */
int storage_rename(const char *from, const char *to);

/**
 * Change permissions of item
 *
 * @param path Path of item to be modified
 * @param mode Mode to set
 *
 * @return int 0 on success, -1 on failure.
 */
int storage_chmod(const char *path, mode_t mode);

/**
 * Truncates file
 *
 * @param path Path of item to be modified
 * @param mode Mode to set
 *
 * @return int 0 on success, -1 on failure.
 */
int storage_truncate(const char *path);

/**
 * Lists directory contents
 *
 * @param path Path of directory
 *
 * @return slist_t* Pointer to list of items
 */
slist_t* storage_list(const char *path);

#endif
