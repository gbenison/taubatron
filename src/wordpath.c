
#include <glib.h>
#include <string.h>
#include <igraph.h>
#include "dictionary.h"
#include "rule.h"

extern gchar* mie_words[];

void
copy_words_cb(word_t *word, GList **dest)
{
  *dest = g_list_prepend(*dest, word_copy(word));
}

static gint
alphabetize(word_t *A, word_t *B)
{
  return strcmp(A->chars, B->chars);
}

/*
 * Delete character 'idx' from 'str'; move subsequent
 * characters earlier
 */
static void
string_shorten(char* str, int idx)
{
  gint i;
  for (i = idx; i < strlen(str); ++i)
    str[i] = str[i + 1];
}

static void
apply_substitution_rule(dictionary_t *dict, igraph_t *graph)
{
  /*
   * A copy of the dictionary with one letter deleted from
   * each word, and the same ID's.
   */

  GList *all_words = NULL;
  dictionary_for_each (dict, (DictionaryCallback)copy_words_cb, &all_words);

  gint delete_idx = 0;
  while (1)
    {
      GList *deletions = NULL;

      GList *cur;
      for (cur = all_words; cur != NULL; cur = cur->next)
	{
	  word_t *word = (word_t*)(cur->data);
	  if (strlen(word->chars) > delete_idx)
	    {
	      word_t *deletion = word_copy(word);
	      deletions = g_list_append(deletions, deletion);
	      string_shorten(deletion->chars, delete_idx);
	    }
	}

      if (g_list_length(deletions) == 0)
	break;

      /* sort the deletion list */
      deletions = g_list_sort(deletions, (GCompareFunc)alphabetize);

      /* report matches */
      cur = deletions;
      while (cur != NULL)
	{
	  GList *head = cur->next;
	  while (1)
	    {
	      if (head == NULL)
		break;
	      word_t *cur_word = (word_t*)(cur->data);
	      word_t *head_word = (word_t*)(head->data);
	      if (strcmp(cur_word->chars, head_word->chars) != 0)
		break;

	      igraph_add_edge(graph, cur_word->id, head_word->id);

	      head = head->next;
	    }
	  cur = head;
	}

      g_list_free(deletions);
      delete_idx++;
    }
}

/*
 * FIXME
 * istructions for dumpig a graph

	      word_t *word_1 = dictionary_lookup_id(dict, cur_word->id);
	      word_t *word_2 = dictionary_lookup_id(dict, head_word->id);

	      g_printf("%s %s\n", word_1->chars, word_2->chars);
 *
 */

char *chars_A = NULL;
char *chars_B = NULL;

/*
 * Abort with error if
 * dictionary_lookup_chars(dict, chars) fails
 */
static word_t*
dictionary_lookup_chars_check(dictionary_t *dict, gchar *chars)
{
  word_t *result = dictionary_lookup_chars(dict, chars);
  if (result == NULL)
    g_error("The word '%s' is not in the dictionary, sorry\n", chars);

  return result;
}

int
main(int argc, char *argv[])
{
  if (argc != 3)
    g_error("Usage:\n%s <word-1> <word-2>\n", argv[0]);

  chars_A = argv[1];
  chars_B = argv[2];

  dictionary_t *dict = dictionary_new();

  /* populate the dictionary with words */
  gchar **cur;
  for (cur = mie_words; *cur != NULL; ++cur)
    dictionary_append (dict, *cur);

  /* Initialize the graph */
  igraph_t graph;
  int n_vertices = dictionary_get_max_id(dict) + 1;
  igraph_empty(&graph, n_vertices, FALSE);

  /* apply the rule */
  apply_substitution_rule(dict, &graph);
  g_printf("Graph contains %d edges\n", (int)(igraph_ecount(&graph)));

  /* look up target words in the dictionary */
  word_t *word_A = dictionary_lookup_chars_check(dict, chars_A);
  word_t *word_B = dictionary_lookup_chars_check(dict, chars_B);

  g_assert(word_A != NULL);
  g_assert(word_B != NULL);

  /* report number of paths */
  igraph_integer_t result;
  int err = igraph_edge_disjoint_paths (&graph,
					&result,
					(igraph_integer_t)(word_A->id),
					(igraph_integer_t)(word_B->id));

  g_printf("Found %d paths from %s --> %s\n", (int)result, word_A->chars, word_B->chars);

  return 0;

}
