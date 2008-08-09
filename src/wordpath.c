
#include <glib.h>
#include <string.h>
#include <igraph.h>
#include "dictionary.h"
#include "rule.h"

extern gchar* mie_words[];
extern gchar* en_words[];
extern gchar* alice_words[];

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

/*
 * Returns:
 * A copy of 'src', filtered to contain only
 * words of length greater than 'idx',
 * with character 'idx' deleted
 */
static GList*
map_words_delete(GList *src, gint idx)
{
  GList *result = NULL;
  GList *cur;
  for (cur = src; cur != NULL; cur = cur->next)
    {
      word_t *word = (word_t*)(cur->data);
      if (strlen(word->chars) > idx)
	{
	  word_t *deletion = word_copy(word);
	  result = g_list_prepend(result, deletion);
	  string_shorten(deletion->chars, idx);
	}
    }
  return result;
}


/*
 * Add an edge to 'graph' for every pair of words (A, B) in 'dict'
 * satisfying the property: B can be formed by deleting exactly one
 * letter from A.
 */
static void
apply_deletion_rule(dictionary_t *dict, igraph_t *graph)
{
  GList *all_words = NULL;
  dictionary_for_each (dict, (DictionaryCallback)copy_words_cb, &all_words);
  all_words = g_list_sort(all_words, (GCompareFunc)alphabetize);

  gint delete_idx = 0;
  while (1)
    {
      GList *deletions = map_words_delete(all_words, delete_idx);

      if (g_list_length(deletions) == 0)
	break;

      igraph_vector_t edge_vector;
      igraph_vector_init(&edge_vector, 2 * g_list_length(deletions));
      int vector_idx = 0;

      /* sort the deletion list */
      deletions = g_list_sort(deletions, (GCompareFunc)alphabetize);

      /* perform the comparisons */
      GList *A = all_words;
      GList *B = deletions;
      while (1)
	{
	  if (A == NULL)
	    break;
	  if (B == NULL)
	    break;

	  word_t *A_word = (word_t*)(A->data);
	  word_t *B_word = (word_t*)(B->data);

	  int relation = strcmp(A_word->chars, B_word->chars);

	  if (relation == 0)
	    {
	      VECTOR(edge_vector)[vector_idx] = A_word->id;
	      ++vector_idx;
	      VECTOR(edge_vector)[vector_idx] = B_word->id;
	      ++vector_idx;

	      B = B->next;
	    }
	  else if (relation > 0) /* A greater */
	    B = B->next;
	  else if (relation < 0) /* B greater */
	    A = A->next;
	}

      igraph_vector_resize(&edge_vector, (long int)vector_idx);
      igraph_add_edges(graph, &edge_vector, 0);

      igraph_vector_destroy(&edge_vector);
      g_list_free(deletions);
      delete_idx++;
    }
}

/*
 * Add an edge to 'graph' for every pair of words (A, B) in 'dict'
 * satisfying the property: B can be formed by changing exactly one
 * letter of A.
 */
static void
apply_substitution_rule(dictionary_t *dict, igraph_t *graph)
{
  GList *all_words = NULL;
  dictionary_for_each (dict, (DictionaryCallback)copy_words_cb, &all_words);

  gint delete_idx = 0;
  while (1)
    {
      GList *deletions = map_words_delete(all_words, delete_idx);

      if (g_list_length(deletions) == 0)
	break;

      /* sort the deletion list */
      deletions = g_list_sort(deletions, (GCompareFunc)alphabetize);

      igraph_vector_t edge_vector;
      igraph_vector_init(&edge_vector, 2 * g_list_length(deletions));
      int vector_idx = 0;

      /* report matches */
      GList *cur = deletions;
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

	      VECTOR(edge_vector)[vector_idx] = cur_word->id;
	      ++vector_idx;
	      VECTOR(edge_vector)[vector_idx] = head_word->id;
	      ++vector_idx;

	      head = head->next;
	    }
	  cur = head;
	}

      igraph_vector_resize(&edge_vector, (long int)vector_idx);
      igraph_add_edges(graph, &edge_vector, 0);

      igraph_vector_destroy(&edge_vector);
      g_list_free(deletions);
      delete_idx++;
    }
}

/*
 * FIXME
 * instructions for dumping a graph

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
  for (cur = en_words; *cur != NULL; ++cur)
    dictionary_append (dict, *cur);

  /* Initialize the graph */
  igraph_t graph;
  int n_vertices = dictionary_get_max_id(dict) + 1;
  igraph_empty(&graph, n_vertices, FALSE);

  /* apply the rules */
  apply_substitution_rule(dict, &graph);
  g_printf("Graph contains %d edges from substitution\n", (int)(igraph_ecount(&graph)));
  apply_deletion_rule(dict, &graph);
  g_printf("Graph contains %d edges, subst. + deletion\n", (int)(igraph_ecount(&graph)));

  /* look up target words in the dictionary */
  word_t *word_A = dictionary_lookup_chars_check(dict, chars_A);
  word_t *word_B = dictionary_lookup_chars_check(dict, chars_B);

  g_assert(word_A != NULL);
  g_assert(word_B != NULL);

  /* report number of paths */
  /*
  igraph_integer_t result;
  int err = igraph_edge_disjoint_paths (&graph,
					&result,
					(igraph_integer_t)(word_A->id),
					(igraph_integer_t)(word_B->id));

  g_printf("Found %d paths from %s --> %s\n", (int)result, word_A->chars, word_B->chars);
  */

  igraph_vector_t path;
  igraph_vector_init(&path, 0);

  igraph_vector_ptr_t path_ptr;
  igraph_vector_ptr_init(&path_ptr, 1);
  igraph_vector_ptr_set(&path_ptr, 0, &path);

  /* report the actual path */
  igraph_vs_t to;
  igraph_vs_1(&to, (igraph_integer_t)(word_B->id));
  igraph_get_shortest_paths(&graph,
			    &path_ptr,
			    (igraph_integer_t)(word_A->id),
			    to,
			    IGRAPH_ALL);

  int i;
  for (i = 0; ; ++i)
    {
      igraph_integer_t vertex = igraph_vector_e(&path, i);
      word_t *word = dictionary_lookup_id(dict, vertex);
      if (word == NULL)
	break;
      g_printf(" %s\n", word->chars);
    }

  return 0;

}
