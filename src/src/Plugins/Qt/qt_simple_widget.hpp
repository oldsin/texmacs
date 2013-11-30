
/******************************************************************************
* MODULE     : qt_simple_widget.hpp
* DESCRIPTION: A widget containing a TeXmacs canvas.
* COPYRIGHT  : (C) 2008  Massimiliano Gubinelli
*******************************************************************************
* This software falls under the GNU general public license version 3 or later.
* It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
* in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
******************************************************************************/

#ifndef QT_SIMPLE_WIDGET_HPP
#define QT_SIMPLE_WIDGET_HPP

#include "qt_view_widget.hpp"

class QTMWidget;

/*! A widget containing a TeXmacs canvas.
 
 This canvas can be used both for input or output of typesetted documents. 
 Editors (editor_rep), output-only widgets (box_widget_rep) and
 other classes are extensions to a "simple_widget", quite a misnomer...
 
 MEMORY POLICY: as usual, we give ownership of the QWidget to the caller of
 as_qwidget(), which in our case will be one of qt_tm_widget_rep or 
 qt_tm_embedded_rep.
  */
class qt_simple_widget_rep: public qt_widget_rep {

  typedef struct t_slot_entry {
    int seq;
    slot_id id;
    blackbox val;
    t_slot_entry() : seq(-1), id (slot_id__LAST), val (blackbox()) { }
    t_slot_entry(const t_slot_entry& other)
    : seq (other.seq), id (other.id), val (other.val) { };
    bool operator< (const t_slot_entry& b) const { return this->seq < b.seq; }
  } t_slot_entry;
  
  t_slot_entry sent_slots[slot_id__LAST];
  
  int sequencer;
  
public:
  qt_simple_widget_rep ();

  virtual bool is_editor_widget ();
  virtual void handle_get_size_hint (SI& w, SI& h);
  virtual void handle_notify_resize (SI w, SI h);
  virtual void handle_keypress (string key, time_t t);
  virtual void handle_keyboard_focus (bool has_focus, time_t t);
  virtual void handle_mouse (string kind, SI x, SI y, int mods, time_t t);
  virtual void handle_set_zoom_factor (double zoom);
  virtual void handle_clear (SI x1, SI y1, SI x2, SI y2);
  virtual void handle_repaint (renderer win, SI x1, SI y1, SI x2, SI y2);
  
    /// Storage of sent messages
  
  void save_send_slot (slot s, blackbox val);
  void reapply_sent_slots();
  virtual void      send (slot s, blackbox val);
  virtual blackbox query (slot s, int type_id);
  virtual widget    read (slot s, blackbox index);
  
  QTMWidget*         canvas () { return qobject_cast<QTMWidget*> (qwid); }
  QTMScrollView* scrollarea () { return qobject_cast<QTMScrollView*> (qwid); }
  
  virtual QAction* as_qaction();
  virtual QWidget* as_qwidget();
};

inline qt_simple_widget_rep* concrete_simple_widget (widget w) { 
  return static_cast<qt_simple_widget_rep*>(w.rep); 
}

// Export for TeXmacs' use
typedef qt_simple_widget_rep simple_widget_rep;

#endif // defined QT_SIMPLE_WIDGET_HPP
