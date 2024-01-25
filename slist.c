/**
 * @file slist.c
 * @author CS3650 staff
 *
 * A simple linked list of strings.
 *
 * This might be useful for directory listings and for manipulating paths.
 */

#include <stdio.h>
#include <alloca.h>
#include <stdlib.h>
#include <string.h>

#include "slist.h"

/**
 * Cons a string to a string list.
 *
 * @param text String to cons on to a list
 * @param rest List of strings to cons onto.
 *
 * @return List starting with the given string in front of the original list.
 */
slist_t *s_cons(const char *text, slist_t *rest) {
  slist_t *xs = malloc(sizeof(slist_t));
  xs->data = strdup(text);
  xs->refs = 1;
  xs->next = rest;
  return xs;
}

/** 
 * Free the given string list.
 *
 * @param xs List of strings to free.
 */
void s_free(slist_t *xs) {
  if (xs == 0) {
    return;
  }

  xs->refs -= 1;

  if (xs->refs == 0) {
    s_free(xs->next);
    free(xs->data);
    free(xs);
  }
}

/**
 * Split the given on the given delimiter into a list of strings.
 *
 * Note, that the delimiter will not be included in any of the strings.
 *
 * @param text String to be split
 * @param delim A single character to use as the delimiter.
 *
 * @return a list containing all the substrings
 */
slist_t *s_explode(const char *text, char delim) {
  if (*text == 0) {
    return 0;
  }

  int plen = 0;
  while (text[plen] != 0 && text[plen] != delim) {
    plen += 1;
  }

  int skip = 0;
  if (text[plen] == delim) {
    skip = 1;
  }

  slist_t *rest = s_explode(text + plen + skip, delim);
  char *part = alloca(plen + 2);
  memcpy(part, text, plen);
  part[plen] = 0;

  return s_cons(part, rest);
}

/**
 * Gets the last string in a list of strings
 *
 * @param slist_t* List of strings to get from
 * 
 * @return char* Last string
 */
char *s_get_last(slist_t *list) {
  slist_t *temp = list;
  while (temp->next) {
    temp = temp->next;
  }
  return temp->data;
}
