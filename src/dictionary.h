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

#include <glib.h>

struct _word
{
  gchar *chars;
  guint  id;
};

typedef struct _word word_t;
typedef struct _dictionary dictionary_t;

typedef void (*DictionaryCallback) (word_t *word, gpointer data);

word_t* word_copy               (word_t *src);

dictionary_t* dictionary_new    (void);
word_t* dictionary_append       (dictionary_t* dict, gchar* word);
word_t* dictionary_lookup_chars (dictionary_t* dict, gchar* word);
word_t* dictionary_lookup_id    (dictionary_t* dict, guint id);
gint    dictionary_get_max_id   (dictionary_t* dict);
void    dictionary_for_each     (dictionary_t*,
				 DictionaryCallback cb,
				 gpointer data);


