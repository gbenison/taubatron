
/* FIXME how about using a hash instead?? */
struct _word
{
  gchar *chars;
  guint  id;
};

typdef struct _word word_t;
typdef struct _dictionary dictionary_t;

typedef void (*DictionaryCallback) (word_t *word, gpointer data);

word_t* dictionary_append       (dictionary_t* dict, gchar* word);
word_t* dictionary_lookup_chars (dictionary_t* dict, gchar* word);
word_t* dictionary_lookup_id    (dictionary_t* dict, guint id);
void    dictionary_for_each     (dictionary_t*,
				 DictionaryCallback cb,
				 gpointer data);


