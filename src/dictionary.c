/*
 *  Copyright (C) 2008 Greg Benison
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "dictionary.h"

word_t*
word_copy (word_t *src)
{
  word_t *result = g_new0(word_t, 1);
  result->chars = g_strdup(src->chars);
  result->id = src->id;

  return result;
}

struct _dictionary
{
  GList *words;
};

dictionary_t*
dictionary_new(void)
{
  dictionary_t* result = g_new0(dictionary_t, 1);
}

word_t*
dictionary_append(dictionary_t* dict, gchar* word)
{
  static guint id = 1;
  word_t* entry = g_new0(word_t, 1);
  entry->id = id;
  entry->chars = g_strdup(word);
  ++id;

  dict->words = g_list_append(dict->words, entry);
}

void
dictionary_for_each     (dictionary_t *dict,
			 DictionaryCallback cb,
			 gpointer data)
{
  GList *cur = NULL;
  for (cur = dict->words; cur != NULL; cur = cur->next)
    cb((word_t*)(cur->data), data);
}

/*
 * Return the word in 'dict' matching 'id'
 */
word_t*
dictionary_lookup_id(dictionary_t* dict, guint id)
{
  GList *cur;
  for (cur = dict->words; cur != NULL; cur = cur->next)
    if (((word_t*)(cur->data))->id == id)
      return (word_t*)(cur->data);
  return NULL;
}


