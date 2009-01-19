
/******************************************************************************
* MODULE     : edit_complete.cpp
* DESCRIPTION: Tab completion
* COPYRIGHT  : (C) 1999  Joris van der Hoeven
*******************************************************************************
* This software falls under the GNU general public license version 3 or later.
* It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
* in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
******************************************************************************/

#include "edit_interface.hpp"
#include "hashset.hpp"
#include "analyze.hpp"
#include "connect.hpp"

/******************************************************************************
* Finding completions in text
******************************************************************************/

static void
find_completions (
  drd_info drd, tree t, hashset<string>& h, string prefix= "")
{
  if (is_atomic (t)) {
    string s= t->label;
    int i= 0, n= N(s);
    while (i<n) {
      if (is_iso_alpha (s[i])) {
	int start= i;
	while ((i<n) && (is_iso_alpha (s[i]))) i++;
	string r= s (start, i);
	if (starts (r, prefix) && (r != prefix))
	  h->insert (r (N(prefix), N(r)));
      }
      else skip_symbol (s, i);
    }
  }
  else {
    int i, n= N(t);
    for (i=0; i<n; i++)
      if (drd->is_accessible_child (t, i))
	find_completions (drd, t[i], h, prefix);
  }
}

static array<string>
find_completions (drd_info drd, tree t, string prefix= "") {
  hashset<string> h;
  find_completions (drd, t, h, prefix);
  return as_completions (h);
}

/******************************************************************************
* Completion mode
******************************************************************************/

bool
edit_interface_rep::complete_try () {
  tree st= subtree (et, path_up (tp));
  if (is_compound (st)) return false;
  string s= st->label, ss;
  int end= last_item (tp);
  array<string> a;
  if (inside (LABEL) || inside (REFERENCE)) {
    if (end != N(s)) return false;
    ss= copy (s);
    tree t= get_labels ();
    int i, n= N(t);
    for (i=0; i<n; i++)
      if (is_atomic (t[i]) && starts (t[i]->label, s))
	a << string (t[i]->label (N(s), N(t[i]->label)));
  }
  else {
    if ((end==0) || (!is_iso_alpha (s[end-1])) ||
	((end!=N(s)) && is_iso_alpha (s[end]))) return false;
    int start= end-1;
    while ((start>0) && is_iso_alpha (s[start-1])) start--;
    ss= s (start, end);
    a= find_completions (drd, et, ss);
  }
  if (N(a) == 0) return false;
  complete_start (ss, a);
  return true;
}

void
edit_interface_rep::complete_message () {
  int i, n= N(completions);
  string s= "Other completions: ";
  for (i=1; i<min(n,11); i++) {
    int j= (completion_pos + i) % n;
    if (i != 1) s << ", ";
    s << completion_prefix << completions[j];
  }
  set_message (s, "tab");
}

void
edit_interface_rep::complete_start (string prefix, array<string> compls) {
  // check consistency
  tree st= subtree (et, path_up (tp));
  if (is_compound (st)) return;
  string s= st->label;
  int end= last_item (tp);
  if ((end<N(prefix)) || (s (end-N(prefix), end) != prefix)) return;

  // perform first completion and switch to completion mode if necessary
  if (N (compls) == 1) {
    string s= compls[0];
    if (ends (s, "()")) // temporary fix for Pari
      insert_tree (s, path (N(s)-1));
    else insert_tree (s);
    completions= array<string> ();
  }
  else {
    completion_prefix= prefix;
    completions      = close_completions (compls);
    completion_pos   = 0;
    insert_tree (completions[0]);
    complete_message ();
    beep ();
    set_input_mode (INPUT_COMPLETE);
  }
}

bool
edit_interface_rep::complete_keypress (string key) {
  if ((key != "tab") && (key != "S-tab")) return false;
  tree st= subtree (et, path_up (tp));
  if (is_compound (st)) return false;
  string s= st->label;
  int end= last_item (tp);
  string old_s= completions [completion_pos];
  string test= completion_prefix * old_s;
  if ((end<N(test)) || (s (end-N(test), end) != test)) return false;

  if (key == "tab") completion_pos++;
  else completion_pos--;
  if (completion_pos < 0) completion_pos= N(completions)-1;
  if (completion_pos >= N(completions)) completion_pos= 0;
  string new_s= completions [completion_pos];
  remove (path_up (tp) * (end-N(old_s)), N(old_s));
  insert (path_up (tp) * (end-N(old_s)), new_s);
  complete_message ();
  return true;
}

/******************************************************************************
* Tab completion inside sessions
******************************************************************************/

static string cursor_symbol ("[tmcursor]");

static tree
put_cursor (tree t, path p) {
  if (is_atomic (t)) {
    string s= t->label;
    return s (0, p->item) * cursor_symbol * s (p->item, N(s));
  }
  else {
    if (p == path (0)) return tree (CONCAT, cursor_symbol, t);
    else if (p == path (1)) return tree (CONCAT, t, cursor_symbol);
    else {
      int i, n= N(t);
      tree u (t, n);
      for (i=0; i<n; i++)
	if (i == p->item) u[i]= put_cursor (t[i], p->next);
	else u[i]= t[i];
      return u;
    }
  }
}

bool
edit_interface_rep::session_complete_try (tree tt) {
  path p= reverse (obtain_ip (tt));
  tree st= subtree (et, p);
  if ((N(tp) <= N(p)) || (tp[N(p)] != 1)) return false;
  tree t= put_cursor (st[1], tail (tp, N(p)+1));
  // cout << t << LF;

  (void) eval ("(use-modules (utils plugins plugin-cmd))");
  string lan= get_env_string (PROG_LANGUAGE);
  string ses= get_env_string (PROG_SESSION);
  string s  = as_string (call ("verbatim-serialize", lan, tree_to_stree (t)));
  s= s (0, N(s)-1);

  int pos= search_forwards (cursor_symbol, s);
  if (pos == -1) return false;
  s= s (0, pos) * s (pos + N(cursor_symbol), N(s));
  // cout << s << ", " << pos << LF;

  string cmd= "(complete " * scm_quote (s) * " " * as_string (pos) * ")";
  tree r= connection_cmd (lan, ses, cmd);

  if (!is_tuple (r)) return false;
  int i, n= N(r);
  string prefix;
  array<string> compls;
  for (i=0; i<n; i++)
    if (is_atomic (r[i])) {
      string l= r[i]->label;
      if (is_quoted (l)) l= scm_unquote (l);
      if (prefix == "") prefix= l;
      else compls << l;
    }
  // cout << prefix << ", " << compls << LF;

  if ((prefix == "") || (N(compls) == 0)) return false;
  complete_start (prefix, compls);
  return true;
}
