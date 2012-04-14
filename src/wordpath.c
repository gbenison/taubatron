
#include <glib.h>
#include <string.h>
#include <igraph/igraph.h>
#include <libguile.h>
#include "dictionary.h"
#include "rule.h"

#define TAUBATRON_ERROR taubatron_error_quark()
static GQuark
taubatron_error_quark (void)
{
  return g_quark_from_static_string("taubatron-error-quark");
}

enum taubatron_errors {
  TAUBATRON_0,
  TAUBATRON_NOT_FOUND
};

extern gchar* mie_words[];
extern gchar* en_words[];
extern gchar* alice_words[];
extern gchar* dickens_words[];

static gchar **standard_word_list = dickens_words;

/*
 * Global variables
 */
dictionary_t *dict;
igraph_t graph;

/*
 * edge_collector-- convenience structure for
 * accumulating many edges and then adding in batch
 * to a graph object
 */
typedef struct _edge_collector edge_collector_t;

static edge_collector_t* edge_collector_new(int n_edges);
static void edge_collector_push(edge_collector_t *edge_collector,
				igraph_integer_t A,
				igraph_integer_t B);
static void edge_collector_send_to_graph(edge_collector_t *edge_collector,
					 igraph_t *graph);
static void edge_collector_finalize(edge_collector_t *self);

struct _edge_collector
{
  igraph_vector_t edge_vector;
  int n_edges;
};

static edge_collector_t*
edge_collector_new(int n_edges)
{
  edge_collector_t *result = g_new0(edge_collector_t, 1);
  igraph_vector_init(&(result->edge_vector), 2 * n_edges);
  result->n_edges = 0;
  return result;
}

static void
edge_collector_push(edge_collector_t *edge_collector,
		    igraph_integer_t A,
		    igraph_integer_t B)
{
  int idx = edge_collector->n_edges * 2;
  VECTOR(edge_collector->edge_vector)[idx] = A;
  VECTOR(edge_collector->edge_vector)[idx + 1] = B;
  ++edge_collector->n_edges;
}

static void
edge_collector_send_to_graph(edge_collector_t *edge_collector, igraph_t *graph)
{
  igraph_vector_resize(&(edge_collector->edge_vector),
		       edge_collector->n_edges * 2);
  igraph_add_edges(graph, &(edge_collector->edge_vector), 0);
}

static void
edge_collector_finalize(edge_collector_t *self)
{
  /* FIXME leak */
}

static void
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
 * Re-order the letters of 'word' such that they're alphabetized.
 */
static void
letter_sort_cb(word_t *word, gpointer data)
{
  /* in-place bubble-sort the word */
  int n_chars = strlen(word->chars);
  int i;
  for (i = n_chars; i > 0; --i)
    {
      int j;
      for (j = 0; j < i - 1; ++j)
	if (word->chars[j + 1] < word->chars[j])
	  {
	    gchar tmp = word->chars[j];
	    word->chars[j] = word->chars[j + 1];
	    word->chars[j + 1] = tmp;
	  }
    }
  /* validate */
  for (i = 1; i < n_chars; ++i)
    g_assert(word->chars[i - 1] <= word->chars[i]);
}

/*
 * Add an edge to 'graph' for every pair of words (A, B) in 'dict'
 * satisfying the property:
 * B is an anagram of A.
 */
static void
apply_anagram_rule(dictionary_t *dict, igraph_t *graph)
{
  GList *all_words = NULL;
  dictionary_for_each (dict, (DictionaryCallback)copy_words_cb, &all_words);
  g_list_foreach(all_words, (GFunc)letter_sort_cb, NULL);
  all_words = g_list_sort(all_words, (GCompareFunc)alphabetize);

  /* duplicates are matches */
  edge_collector_t *edge_collector =
    edge_collector_new(g_list_length(all_words));
  GList *cur;
  for (cur = all_words; cur != NULL; cur = cur->next)
    {
      word_t *cur_word = cur->data;
      GList *head;
      for (head = cur->next; head != NULL; head = head->next)
	{
	  word_t *head_word = head->data;
	  if (strcmp(head_word->chars, cur_word->chars) != 0)
	    break;
	  edge_collector_push(edge_collector, cur_word->id, head_word->id);

	  /* DEBUG dump result */
	  /*
	  g_printf("anagram: %s -- %s\n",
		   dictionary_lookup_id(dict, cur_word->id)->chars,
		   dictionary_lookup_id(dict, head_word->id)->chars);
	  */
	}
    }
  edge_collector_send_to_graph(edge_collector, graph);
  edge_collector_finalize(edge_collector);

  /* FIXME free word objects */
  g_list_free(all_words);
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

      edge_collector_t *edge_collector =
	edge_collector_new(g_list_length(deletions));

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
	      edge_collector_push(edge_collector, A_word->id, B_word->id);
	      B = B->next;
	    }
	  else if (relation > 0) /* A greater */
	    B = B->next;
	  else if (relation < 0) /* B greater */
	    A = A->next;
	}

      edge_collector_send_to_graph(edge_collector, graph);
      edge_collector_finalize(edge_collector);

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

      edge_collector_t *edge_collector =
	edge_collector_new(g_list_length(deletions));

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

	      edge_collector_push(edge_collector, cur_word->id, head_word->id);
	      head = head->next;
	    }
	  cur = head;
	}

      edge_collector_send_to_graph(edge_collector, graph);
      edge_collector_finalize(edge_collector);

      g_list_free(deletions);
      delete_idx++;
    }
}

/*
 * Abort with error if
 * dictionary_lookup_chars(dict, chars) fails
 */
static word_t*
dictionary_lookup_chars_check(dictionary_t *dict, gchar *chars, GError **error)
{
  word_t *result = dictionary_lookup_chars(dict, chars);
  if (result == NULL)
    {
      g_set_error(error,
		  TAUBATRON_ERROR,
		  TAUBATRON_NOT_FOUND,
		  "Word '%s' is not in the dictionary",
		  chars);
      return NULL;
    }
  else
    return result;
}

/*
 * Find shortest path from word A to word B
 */
SCM
taubatron_find_path(SCM start, SCM finish)
{
  static SCM tt_error_key = NULL;
  if (!tt_error_key)
    tt_error_key = scm_from_locale_symbol("taubatron-error");
  
  /* This is an incredibly hacky way of making an empty list. */
  /*  SCM result = scm_cdr(scm_list_1(scm_from_int(55))); */
  SCM result = SCM_EOL;
  
  /* FIXME "The C string must be freed with 'free' eventually,
     maybe by using scm_dynwind_free" */
  char *chars_A = scm_to_locale_string(start);
  char *chars_B = scm_to_locale_string(finish);
  
  /* look up target words in the dictionary */
  GError *lookup_error = NULL;
  word_t *word_A = dictionary_lookup_chars_check(dict, chars_A, &lookup_error);
  if (word_A == NULL)
    {
      scm_error(tt_error_key, "find_path", lookup_error->message,
		SCM_EOL, SCM_EOL);
      return NULL;
    }
  word_t *word_B = dictionary_lookup_chars_check(dict, chars_B, &lookup_error);
  if (word_B == NULL)
    {
      scm_error(tt_error_key, "find_path", lookup_error->message,
		SCM_EOL, SCM_EOL);
      return NULL;
    }

  g_assert(word_A != NULL);
  g_assert(word_B != NULL);

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
      result = scm_cons(scm_from_locale_string(word->chars), result);
    }

  return (scm_reverse(result));
}

/*
 * Entry point for guile module
 */
void
taubatron_init()
{

  dict = dictionary_new();

  /* populate the dictionary with words */
  gchar **cur;
  for (cur = standard_word_list; *cur != NULL; ++cur)
    dictionary_append (dict, *cur);

  /* Initialize the graph */
  int n_vertices = dictionary_get_max_id(dict) + 1;
  igraph_empty(&graph, n_vertices, FALSE);

  /* apply the rules */
  apply_substitution_rule(dict, &graph);
  apply_deletion_rule(dict, &graph);
  apply_anagram_rule(dict, &graph);

  scm_c_define_gsubr("find-path", 2, 1, 0, taubatron_find_path);

}
