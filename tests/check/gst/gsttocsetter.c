/* GStreamer GstTocSetter interface unit tests
 * Copyright (C) 2010, 2012 Alexander Saprykin <xelfium@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <gst/check/gstcheck.h>
#include <gst/gst.h>
#include <string.h>

#define ENTRY_ED1       "/edition1"
#define ENTRY_ED2       "/edition2"
#define ENTRY_ED3       "test-edition"

#define ENTRY_CH1       "/edition1/chapter1"
#define ENTRY_CH2       "/edition1/chapter2"
#define ENTRY_CH3       "/edition2/chapter3"
#define ENTRY_CH4       "/test-chapter"

#define ENTRY_SUB1      "/edition2/chapter3/subchapter1"

#define ENTRY_TAG       "EntryTag"
#define TOC_TAG         "TocTag"
#define INFO_NAME       "info"
#define INFO_FIELD      "info-test"
#define INFO_TEXT_EN    "info-text-entry"
#define INFO_TEXT_TOC   "info-text-toc"

#define CHECK_TOC_ENTRY(entry_c,type_c,uid_c)                            \
{                                                                        \
  gchar *tag_c;                                                          \
  const GValue *val;                                                     \
                                                                         \
  fail_unless_equals_string (entry_c->uid, uid_c);                       \
  fail_unless (entry_c->type == type_c);                                 \
  fail_unless (entry_c->tags != NULL);                                   \
  fail_unless (entry_c->pads == NULL);                                   \
                                                                         \
  fail_unless (entry_c->info != NULL);                                   \
  val = gst_structure_get_value (entry_c->info, INFO_FIELD);             \
  fail_unless (val != NULL);                                             \
  fail_unless_equals_string (g_value_get_string (val), INFO_TEXT_EN);    \
                                                                         \
  fail_unless (gst_tag_list_get_string (entry_c->tags,                   \
               GST_TAG_TITLE, &tag_c));                                  \
  fail_unless_equals_string (tag_c, ENTRY_TAG);                          \
}

#define CHECK_TOC(toc_t)                                                 \
{                                                                        \
  GstTocEntry *entry_t, *subentry_t;                                     \
  gchar *tag_t;                                                          \
  const GValue *val;                                                     \
  /* check TOC */                                                        \
  fail_unless (g_list_length (toc_t->entries) == 2);                     \
  fail_unless (toc_t->tags != NULL);                                     \
  fail_unless (gst_tag_list_get_string (toc_t->tags,                     \
               GST_TAG_TITLE, &tag_t));                                  \
  fail_unless_equals_string (tag_t, TOC_TAG);                            \
                                                                         \
  fail_unless (toc_t->info != NULL);                                     \
  val = gst_structure_get_value (toc_t->info, INFO_FIELD);               \
  fail_unless (val != NULL);                                             \
  fail_unless_equals_string (g_value_get_string (val), INFO_TEXT_TOC);   \
                                                                         \
  /* check edition1 */                                                   \
  entry_t = g_list_nth_data (toc_t->entries, 0);                         \
  fail_if (entry_t == NULL);                                             \
  fail_unless (g_list_length (entry_t->subentries) == 2);                \
  CHECK_TOC_ENTRY (entry_t, GST_TOC_ENTRY_TYPE_EDITION, ENTRY_ED1);      \
  /* check chapter1 */                                                   \
  subentry_t = g_list_nth_data (entry_t->subentries, 0);                 \
  fail_if (subentry_t == NULL);                                          \
  fail_unless (g_list_length (subentry_t->subentries) == 0);             \
  CHECK_TOC_ENTRY (subentry_t, GST_TOC_ENTRY_TYPE_CHAPTER, ENTRY_CH1);   \
  /* check chapter2 */                                                   \
  subentry_t = g_list_nth_data (entry_t->subentries, 1);                 \
  fail_if (subentry_t == NULL);                                          \
  fail_unless (g_list_length (subentry_t->subentries) == 0);             \
  CHECK_TOC_ENTRY (subentry_t, GST_TOC_ENTRY_TYPE_CHAPTER, ENTRY_CH2);   \
  /* check edition2 */                                                   \
  entry_t = g_list_nth_data (toc_t->entries, 1);                         \
  fail_if (entry_t == NULL);                                             \
  fail_unless (g_list_length (entry_t->subentries) == 1);                \
  CHECK_TOC_ENTRY (entry_t, GST_TOC_ENTRY_TYPE_EDITION, ENTRY_ED2);      \
  /* check chapter3 */                                                   \
  subentry_t = g_list_nth_data (entry_t->subentries, 0);                 \
  fail_if (subentry_t == NULL);                                          \
  fail_unless (g_list_length (subentry_t->subentries) == 1);             \
  CHECK_TOC_ENTRY (subentry_t, GST_TOC_ENTRY_TYPE_CHAPTER, ENTRY_CH3);   \
  /* check subchapter1 */                                                \
  subentry_t = g_list_nth_data (subentry_t->subentries, 0);              \
  fail_if (subentry_t == NULL);                                          \
  fail_unless (g_list_length (subentry_t->subentries) == 0);             \
  CHECK_TOC_ENTRY (subentry_t, GST_TOC_ENTRY_TYPE_CHAPTER, ENTRY_SUB1);  \
}

/* some minimal GstTocSetter object */
#define GST_TYPE_DUMMY_ENC gst_dummy_enc_get_type()

typedef GstElement GstDummyEnc;
typedef GstElementClass GstDummyEncClass;

static void gst_dummy_enc_add_interfaces (GType enc_type);

GType gst_dummy_enc_get_type (void);
GST_BOILERPLATE_FULL (GstDummyEnc, gst_dummy_enc, GstElement,
    GST_TYPE_ELEMENT, gst_dummy_enc_add_interfaces);

static void
gst_dummy_enc_add_interfaces (GType enc_type)
{
  static const GInterfaceInfo toc_setter_info = { NULL, NULL, NULL };

  g_type_add_interface_static (enc_type, GST_TYPE_TOC_SETTER, &toc_setter_info);
}

static void
gst_dummy_enc_base_init (gpointer g_class)
{
}

static void
gst_dummy_enc_class_init (GstDummyEncClass * klass)
{
}

static void
gst_dummy_enc_init (GstDummyEnc * enc, GstDummyEncClass * klass)
{
}

static GstToc *
create_toc (void)
{
  GstToc *toc;
  GstTocEntry *ed, *ch, *subch;

  toc = gst_toc_new ();
  gst_tag_list_add (toc->tags, GST_TAG_MERGE_APPEND, GST_TAG_TITLE,
      TOC_TAG, NULL);
  toc->info =
      gst_structure_new (INFO_NAME, INFO_FIELD, G_TYPE_STRING, INFO_TEXT_TOC,
      NULL);

  /* create edition1 */
  ed = gst_toc_entry_new (GST_TOC_ENTRY_TYPE_EDITION, ENTRY_ED1);
  gst_tag_list_add (ed->tags, GST_TAG_MERGE_APPEND, GST_TAG_TITLE,
      ENTRY_TAG, NULL);
  ed->info =
      gst_structure_new (INFO_NAME, INFO_FIELD, G_TYPE_STRING, INFO_TEXT_EN,
      NULL);

  /* append chapter1 to edition1 */
  ch = gst_toc_entry_new (GST_TOC_ENTRY_TYPE_CHAPTER, ENTRY_CH1);
  gst_tag_list_add (ch->tags, GST_TAG_MERGE_APPEND, GST_TAG_TITLE,
      ENTRY_TAG, NULL);
  ch->info =
      gst_structure_new (INFO_NAME, INFO_FIELD, G_TYPE_STRING, INFO_TEXT_EN,
      NULL);

  ed->subentries = g_list_append (ed->subentries, ch);

  /* append chapter2 to edition1 */
  ch = gst_toc_entry_new (GST_TOC_ENTRY_TYPE_CHAPTER, ENTRY_CH2);
  gst_tag_list_add (ch->tags, GST_TAG_MERGE_APPEND, GST_TAG_TITLE,
      ENTRY_TAG, NULL);
  ch->info =
      gst_structure_new (INFO_NAME, INFO_FIELD, G_TYPE_STRING, INFO_TEXT_EN,
      NULL);

  ed->subentries = g_list_append (ed->subentries, ch);

  /* append edition1 to the TOC */
  toc->entries = g_list_append (toc->entries, ed);

  /* create edition2 */
  ed = gst_toc_entry_new (GST_TOC_ENTRY_TYPE_EDITION, ENTRY_ED2);
  gst_tag_list_add (ed->tags, GST_TAG_MERGE_APPEND, GST_TAG_TITLE,
      ENTRY_TAG, NULL);
  ed->info =
      gst_structure_new (INFO_NAME, INFO_FIELD, G_TYPE_STRING, INFO_TEXT_EN,
      NULL);

  /* create chapter3 */
  ch = gst_toc_entry_new (GST_TOC_ENTRY_TYPE_CHAPTER, ENTRY_CH3);
  gst_tag_list_add (ch->tags, GST_TAG_MERGE_APPEND, GST_TAG_TITLE,
      ENTRY_TAG, NULL);
  ch->info =
      gst_structure_new (INFO_NAME, INFO_FIELD, G_TYPE_STRING, INFO_TEXT_EN,
      NULL);

  /* create subchapter1 */
  subch = gst_toc_entry_new (GST_TOC_ENTRY_TYPE_CHAPTER, ENTRY_SUB1);
  gst_tag_list_add (subch->tags, GST_TAG_MERGE_APPEND, GST_TAG_TITLE,
      ENTRY_TAG, NULL);
  subch->info =
      gst_structure_new (INFO_NAME, INFO_FIELD, G_TYPE_STRING, INFO_TEXT_EN,
      NULL);

  /* append subchapter1 to chapter3 */
  ch->subentries = g_list_append (ch->subentries, subch);

  /* append chapter3 to edition2 */
  ed->subentries = g_list_append (ed->subentries, ch);

  /* finally append edition2 to the TOC */
  toc->entries = g_list_append (toc->entries, ed);

  return toc;
}

GST_START_TEST (test_set)
{
  GstToc *toc;
  GstTocEntry *entry, *ed;
  GstTocSetter *setter;
  GstElement *enc;

  enc = g_object_new (GST_TYPE_DUMMY_ENC, NULL);
  fail_unless (enc != NULL);

  setter = GST_TOC_SETTER (enc);

  toc = create_toc ();
  fail_unless (toc != NULL);

  gst_toc_setter_set_toc (setter, toc);

  gst_toc_free (toc);
  toc = gst_toc_setter_get_toc_copy (setter);

  CHECK_TOC (toc);

  /* test entry adding into the root TOC */
  entry = g_list_last (toc->entries)->data;
  toc->entries = g_list_remove (toc->entries, entry);

  gst_toc_setter_set_toc (setter, toc);
  gst_toc_setter_add_toc_entry (setter, "0", entry);

  gst_toc_free (toc);
  toc = gst_toc_setter_get_toc_copy (setter);

  CHECK_TOC (toc);

  /* test entry adding into the arbitrary entry */
  entry = gst_toc_find_entry (toc, ENTRY_CH2);
  fail_if (entry == NULL);

  ed = toc->entries->data;
  ed->subentries = g_list_remove (ed->subentries, entry);

  gst_toc_setter_add_toc_entry (setter, ed->uid, entry);

  CHECK_TOC (toc);

  gst_toc_free (toc);
  gst_toc_setter_reset_toc (setter);
  toc = gst_toc_setter_get_toc_copy (setter);

  fail_unless (toc == NULL);

  g_object_unref (enc);
}

GST_END_TEST static int spin_and_wait = 1;
static int threads_running = 0;

#define THREADS_TEST_SECONDS 1.5

static gpointer
test_threads_thread_func1 (gpointer data)
{
  GstToc *toc;
  GstTocSetter *setter = GST_TOC_SETTER (data);
  GTimer *timer;

  toc = create_toc ();
  timer = g_timer_new ();

  g_atomic_int_inc (&threads_running);
  while (g_atomic_int_get (&spin_and_wait))
    g_usleep (0);

  GST_INFO ("Go!");
  g_timer_start (timer);

  while (g_timer_elapsed (timer, NULL) < THREADS_TEST_SECONDS)
    gst_toc_setter_set_toc (setter, toc);

  gst_toc_free (toc);
  g_timer_destroy (timer);
  GST_INFO ("Done");

  return NULL;
}

static gpointer
test_threads_thread_func2 (gpointer data)
{
  GstToc *toc;
  GstTocSetter *setter = GST_TOC_SETTER (data);
  GTimer *timer;

  toc = create_toc ();
  timer = g_timer_new ();

  g_atomic_int_inc (&threads_running);
  while (g_atomic_int_get (&spin_and_wait))
    g_usleep (0);

  GST_INFO ("Go!");
  g_timer_start (timer);

  while (g_timer_elapsed (timer, NULL) < THREADS_TEST_SECONDS)
    gst_toc_setter_set_toc (setter, toc);

  gst_toc_free (toc);
  g_timer_destroy (timer);
  GST_INFO ("Done");

  return NULL;
}

static gpointer
test_threads_thread_func3 (gpointer data)
{
  GstTocSetter *setter = GST_TOC_SETTER (data);
  GTimer *timer;

  timer = g_timer_new ();

  g_atomic_int_inc (&threads_running);
  while (g_atomic_int_get (&spin_and_wait))
    g_usleep (0);

  GST_INFO ("Go!");
  g_timer_start (timer);

  while (g_timer_elapsed (timer, NULL) < THREADS_TEST_SECONDS) {
    gst_toc_setter_reset_toc (setter);
  }

  g_timer_destroy (timer);
  GST_INFO ("Done");

  return NULL;
}

GST_START_TEST (test_threads)
{
  GstTocSetter *setter;
  GThread *threads[3];

  setter = GST_TOC_SETTER (g_object_new (GST_TYPE_DUMMY_ENC, NULL));

  spin_and_wait = TRUE;
  threads[0] = g_thread_create (test_threads_thread_func1, setter, TRUE, NULL);
  threads[1] = g_thread_create (test_threads_thread_func2, setter, TRUE, NULL);
  threads[2] = g_thread_create (test_threads_thread_func3, setter, TRUE, NULL);

  while (g_atomic_int_get (&threads_running) < 3)
    g_usleep (10);

  g_atomic_int_set (&spin_and_wait, FALSE);

  g_thread_join (threads[0]);
  g_thread_join (threads[1]);
  g_thread_join (threads[2]);

  g_object_unref (G_OBJECT (setter));
}

GST_END_TEST static Suite *
gst_toc_setter_suite (void)
{
  Suite *s = suite_create ("GstTocSetter");
  TCase *tc_chain = tcase_create ("general");

  suite_add_tcase (s, tc_chain);
  tcase_add_test (tc_chain, test_set);
  tcase_add_test (tc_chain, test_threads);

  return s;
}

GST_CHECK_MAIN (gst_toc_setter);