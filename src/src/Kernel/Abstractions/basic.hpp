
/******************************************************************************
* MODULE     : basic.hpp
* DESCRIPTION: see basic.cpp
* COPYRIGHT  : (C) 1999  Joris van der Hoeven
*******************************************************************************
* This software falls under the GNU general public license version 3 or later.
* It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
* in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
******************************************************************************/

#ifndef BASIC_H
#define BASIC_H
#include "fast_alloc.hpp"
#include <math.h>

#ifdef HAVE_INTPTR_T
#ifdef OS_SUN
#include <inttypes.h>
#else
#include <stdint.h>
#endif
#else
typedef long intptr_t;
#endif

#ifdef OS_WIN32
#define LESSGTR
#else
#define LESSGTR <>
#endif

#define TM_DEBUG(x)
typedef int SI;
typedef unsigned int SN;
typedef short HI;
typedef unsigned short HN;
typedef char QI;
typedef unsigned char QN;
#ifdef OS_WIN32
typedef __int64 DI;
#else
typedef long long int DI;
#endif
typedef void* pointer;

/******************************************************************************
* debugging
******************************************************************************/

#if (defined OS_WIN32 || defined __SUNPRO_CC)
#define STACK_NEW_ARRAY(name,T,size) T* name= tm_new_array<T> (size)
#define STACK_DELETE_ARRAY(name) tm_delete_array (name)
#else
#define STACK_NEW_ARRAY(name,T,size) T name[size]
#define STACK_DELETE_ARRAY(name)
#endif

enum { DEBUG_FLAG_AUTO, DEBUG_FLAG_VERBOSE, DEBUG_FLAG_EVENTS,
       DEBUG_FLAG_STD, DEBUG_FLAG_IO, DEBUG_FLAG_BENCH,
       DEBUG_FLAG_HISTORY, DEBUG_FLAG_QT, DEBUG_FLAG_KEYBOARD,
       DEBUG_FLAG_PACKRAT, DEBUG_FLAG_FLATTEN, DEBUG_FLAG_CORRECT };
bool debug (int which, bool write_flag= false);
int  debug_off ();
void debug_on (int status);
class string;
void debug_set (string s, bool flag);
bool debug_get (string s);
#define DEBUG_AUTO (debug (DEBUG_FLAG_AUTO))
#define DEBUG_VERBOSE (debug (DEBUG_FLAG_VERBOSE))
#define DEBUG_EVENTS (debug (DEBUG_FLAG_EVENTS))
#define DEBUG_STD (debug (DEBUG_FLAG_STD))
#define DEBUG_IO (debug (DEBUG_FLAG_IO))
#define DEBUG_BENCH (debug (DEBUG_FLAG_BENCH))
#define DEBUG_HISTORY (debug (DEBUG_FLAG_HISTORY))
#define DEBUG_QT (debug (DEBUG_FLAG_QT))
#define DEBUG_KEYBOARD (debug (DEBUG_FLAG_KEYBOARD))
#define DEBUG_PACKRAT (debug (DEBUG_FLAG_PACKRAT))
#define DEBUG_FLATTEN (debug (DEBUG_FLAG_FLATTEN))
#define DEBUG_CORRECT (debug (DEBUG_FLAG_CORRECT))

void tm_failure (const char* msg);
#ifdef DEBUG_ASSERT
#include <assert.h>
#define ASSERT(cond,msg) { if (!(cond)) { tm_failure (msg); assert (cond); } }
#define FAILED(msg) { tm_failure (msg); assert (false); }
#else
#define ASSERT(cond,msg) { if (!(cond)) { tm_failure (msg); } }
#define FAILED(msg) { tm_failure (msg); }
#endif

/******************************************************************************
* miscellaneous routines
******************************************************************************/

inline SI min (SI i, SI j) { if (i<j) return i; else return j; }
inline SI max (SI i, SI j) { if (i>j) return i; else return j; }
inline DI min (DI i, DI j) { if (i<j) return i; else return j; }
inline DI max (DI i, DI j) { if (i>j) return i; else return j; }
inline double min (double i, double j) { if (i<j) return i; else return j; }
inline double max (double i, double j) { if (i>j) return i; else return j; }
inline int hash (int i) { return i; }
inline int hash (pointer ptr) {
  return ((int) ((intptr_t) ptr)) + (((int) ((intptr_t) ptr)) % 19); }
inline int copy (int x) { return x; }
inline SI as_int (double x) { return (SI) floor (x + 0.5); }

enum display_control { INDENT, UNINDENT, HRULE, LF };
tm_ostream& operator << (tm_ostream& out, display_control ctrl);

bool gui_is_x ();
bool gui_is_qt ();
bool os_win32 ();
bool os_mingw ();
bool os_macos ();
bool use_macos_fonts ();
const char* default_look_and_feel ();

template<typename T>
struct type_helper {
  static int id;
  static T   init;
};

int new_type_identifier ();
template<typename T> int type_helper<T>::id  = new_type_identifier ();
template<typename T> T   type_helper<T>::init= T ();

/******************************************************************************
* concrete and abstract base structures
******************************************************************************/

extern int concrete_count;
struct concrete_struct {
  int ref_count;
  inline concrete_struct (): ref_count (1) { TM_DEBUG(concrete_count++); }
  virtual inline ~concrete_struct () { TM_DEBUG(concrete_count--); }
};

extern int abstract_count;
struct abstract_struct {
  int ref_count;
  inline abstract_struct (): ref_count (0) { TM_DEBUG(abstract_count++); }
  virtual inline ~abstract_struct () { TM_DEBUG(abstract_count--); }
};

/******************************************************************************
* indirect structures
******************************************************************************/

#define INC_COUNT(R)      { (R)->ref_count++; }
#define DEC_COUNT(R)      { if(0==--((R)->ref_count)) tm_delete (R); }
#define INC_COUNT_NULL(R) { if ((R)!=NULL) (R)->ref_count++; }
#define DEC_COUNT_NULL(R) { if ((R)!=NULL && 0==--((R)->ref_count)) tm_delete (R); }

class tm_base {
  //  a base common class for all the texmacs objects
};

template <class T>
class tm_stats  {
public:
  static int alive;
  static int created;
  tm_stats () { alive++; created++; }
protected:
  ~tm_stats () { alive--; }
};

template <class T> int tm_stats<T>::alive (0);
template <class T> int tm_stats<T>::created (0);


template <class T> class tm_ptr; 
template <class T> class tm_null_ptr; 
template <class T> class tm_abs_ptr; 
template <class T> class tm_abs_null_ptr; 

template <class T>
class tm_obj : public tm_base, public tm_stats<T> {
	int ref_count;
  
protected:
	inline tm_obj (): ref_count (0) { TM_DEBUG(concrete_count++); }
  inline ~tm_obj () { TM_DEBUG(concrete_count--); }
	inline void inc_ref () { ref_count++; } 
	inline void dec_ref () { if (0 == --ref_count) static_cast<T*>(this)->destroy(); } 
  inline void destroy () { tm_delete (static_cast<T*>(this)); }
  
public:
   template <class TT> friend class tm_ptr;
  template <class TT> friend class tm_null_ptr;
  template <class TT> friend class tm_abs_ptr;
  template <class TT> friend class tm_abs_null_ptr;
};


template <class T>
class tm_ptr {
  T *rep_;
protected:	
  inline tm_ptr (T* p) : rep_ (p) { rep_->inc_ref(); }
  inline T* rep() const { return rep_; }
public:
  inline tm_ptr (const tm_ptr<T>& x) : rep_(x.rep_) { rep_->inc_ref(); }
  inline tm_ptr() { rep_->dec_ref(); }
  inline tm_ptr& operator=(tm_ptr<T> x) {  x.rep_->inc_ref();  rep_->dec_ref(); rep_=x.rep_; return *this; }
  inline T* operator->() { return rep_; }
};

template <class T> class tm_null_ptr;
template <class T>  bool is_nil (tm_null_ptr<T> p);

template <class T>
class tm_null_ptr {
  T *rep_;
protected:	
  inline tm_null_ptr (T* p) : rep_ (p) { if (rep_) rep_->inc_ref(); }
  inline T* rep() const { return rep_; }
public:
  inline tm_null_ptr () : rep_ (NULL) {  }
  inline tm_null_ptr (const tm_null_ptr<T>& x) : rep_(x.rep_) { if (rep_)  rep_->inc_ref(); }
  inline ~tm_null_ptr() { if (rep_)  rep_->dec_ref(); }
  inline tm_null_ptr& operator=(tm_null_ptr<T> x) {  if (x.rep_) x.rep_->inc_ref();  if (rep_) rep_->dec_ref(); rep_=x.rep_; return *this; }
  inline T* operator->() { return rep_; }
  friend bool is_nil<T> (tm_null_ptr<T> p);
};

template <class T>
inline bool is_nil (tm_null_ptr<T> p) { return (p.rep() == NULL); }


template <class T>
class tm_abs_null_ptr : public tm_null_ptr<T> {
public:
  inline tm_abs_null_ptr (T* p) : tm_null_ptr<T> (p) {  }
  inline tm_abs_null_ptr () : tm_null_ptr<T> () {  }
//  inline tm_abs_null_ptr (const tm_abs_null_ptr<T>& x) : tm_null_ptr<T> (x) {}
};

template <class T>
class tm_abs_ptr : public tm_ptr<T> {
public:
  inline tm_abs_ptr (T* p) : tm_ptr<T> (p) {  }
  inline tm_abs_ptr () : tm_ptr<T> () {  }
  //  inline tm_abs_null_ptr (const tm_abs_null_ptr<T>& x) : tm_null_ptr<T> (x) {}
};




// concrete
#define CONCRETE(PTR)               \
  PTR##_rep *rep;                   \
public:                             \
  inline PTR (const PTR&);          \
  inline ~PTR ();                   \
  inline PTR##_rep* operator -> (); \
  inline PTR& operator = (PTR x)
#define CONCRETE_CODE(PTR)                       \
  inline PTR::PTR (const PTR& x):                \
    rep(x.rep) { INC_COUNT (this->rep); }        \
  inline PTR::~PTR () { DEC_COUNT (this->rep); } \
  inline PTR##_rep* PTR::operator -> () {        \
    return rep; }                                \
  inline PTR& PTR::operator = (PTR x) {          \
    INC_COUNT (x.rep); DEC_COUNT (this->rep);    \
    this->rep=x.rep; return *this; }

// definition for 1 parameter template classes
#define CONCRETE_TEMPLATE(PTR,T)      \
  PTR##_rep<T> *rep;                  \
public:                               \
  inline PTR (const PTR<T>&);         \
  inline ~PTR ();                     \
  inline PTR##_rep<T>* operator -> (); \
  inline PTR<T>& operator = (PTR<T> x)
#define CONCRETE_TEMPLATE_CODE(PTR,TT,T)                          \
  template<TT T> inline PTR<T>::PTR (const PTR<T>& x):            \
    rep(x.rep) { INC_COUNT (this->rep); }                         \
  template<TT T> inline PTR<T>::~PTR() { DEC_COUNT (this->rep); } \
  template<TT T> inline PTR##_rep<T>* PTR<T>::operator -> () {    \
    return this->rep; }                                           \
  template<TT T> inline PTR<T>& PTR<T>::operator = (PTR<T> x) {   \
    INC_COUNT (x.rep); DEC_COUNT (this->rep);                     \
    this->rep=x.rep; return *this; }

// definition for 2 parameter template classes
#define CONCRETE_TEMPLATE_2(PTR,T1,T2)     \
  PTR##_rep<T1,T2> *rep;                   \
public:                                    \
  inline PTR (const PTR<T1,T2>&);          \
  inline ~PTR ();                          \
  inline PTR##_rep<T1,T2>* operator -> (); \
  inline PTR<T1,T2>& operator = (PTR<T1,T2> x)
#define CONCRETE_TEMPLATE_2_CODE(PTR,TT1,T1,TT2,T2)                           \
  template<TT1 T1,TT2 T2> inline PTR<T1,T2>::PTR (const PTR<T1,T2>& x):       \
    rep(x.rep) { INC_COUNT (this->rep); }                                     \
  template<TT1 T1,TT2 T2> inline PTR<T1,T2>::~PTR () { DEC_COUNT(this->rep);} \
  template<TT1 T1,TT2 T2> inline PTR##_rep<T1,T2>* PTR<T1,T2>::operator -> () \
    { return this->rep; }                                                     \
  template <TT1 T1,TT2 T2>                                                    \
  inline PTR<T1,T2>& PTR<T1,T2>::operator = (PTR<T1,T2> x) {                  \
    INC_COUNT (x.rep); DEC_COUNT (this->rep);                                 \
    this->rep=x.rep; return *this; }
// end concrete

// abstract
#define ABSTRACT(PTR) \
  CONCRETE(PTR);      \
  inline PTR (PTR##_rep*)
#define ABSTRACT_CODE(PTR) \
  CONCRETE_CODE(PTR) ;     \
  inline PTR::PTR (PTR##_rep* rep2): rep(rep2) { INC_COUNT (this->rep); }
#define ABSTRACT_TEMPLATE(PTR,T) \
  CONCRETE_TEMPLATE(PTR,T);      \
  inline PTR (PTR##_rep<T>*)
#define ABSTRACT_TEMPLATE_CODE(PTR,TT,T)                  \
  CONCRETE_TEMPLATE_CODE(PTR,TT,T);                       \
  template<TT T> inline PTR<T>::PTR (PTR##_rep<T>* rep2): \
    rep(rep2) { INC_COUNT (this->rep); }
// end abstract

/******************************************************************************
* null indirect structures
******************************************************************************/

// concrete_null
#define CONCRETE_NULL(PTR) \
  CONCRETE(PTR);           \
  inline PTR();            \
  friend bool is_nil /*LESSGTR*/ (PTR x)
#define CONCRETE_NULL_CODE(PTR)                         \
  inline PTR::PTR (): rep(NULL) {}                      \
  inline PTR::PTR (const PTR& x):                       \
    rep(x.rep) { INC_COUNT_NULL (this->rep); }          \
  inline PTR::~PTR() { DEC_COUNT_NULL (this->rep); }    \
  inline PTR##_rep* PTR::operator -> () {               \
    return this->rep; }                                 \
  inline PTR& PTR::operator = (PTR x) {                 \
    INC_COUNT_NULL (x.rep); DEC_COUNT_NULL (this->rep); \
    this->rep=x.rep; return *this; }                    \
  inline bool is_nil (PTR x) { return x.rep==NULL; }
#define CONCRETE_NULL_TEMPLATE(PTR,T) \
  CONCRETE_TEMPLATE(PTR,T);           \
  inline PTR();                       \
  friend bool is_nil LESSGTR (PTR<T> x)
#define CONCRETE_NULL_TEMPLATE_CODE(PTR,TT,T)                           \
  template<TT T> inline PTR<T>::PTR (): rep(NULL) {}                    \
  template<TT T> inline PTR<T>::PTR (const PTR<T>& x):                  \
    rep(x.rep) { INC_COUNT_NULL (this->rep); }                          \
  template<TT T> inline PTR<T>::~PTR () { DEC_COUNT_NULL (this->rep); } \
  template<TT T> inline PTR##_rep<T>* PTR<T>::operator -> () {          \
    return this->rep; }                                                 \
  template<TT T> inline PTR<T>& PTR<T>::operator = (PTR<T> x) {         \
    INC_COUNT_NULL (x.rep); DEC_COUNT_NULL (this->rep);                 \
    this->rep=x.rep; return *this; }                                    \
  template<TT T> inline bool is_nil (PTR<T> x) { return x.rep==NULL; }

#define CONCRETE_NULL_TEMPLATE_2(PTR,T1,T2) \
  CONCRETE_TEMPLATE_2(PTR,T1,T2);           \
  inline PTR();                             \
  friend bool is_nil LESSGTR (PTR<T1,T2> x)
#define CONCRETE_NULL_TEMPLATE_2_CODE(PTR,TT1,T1,TT2,T2)                  \
  template<TT1 T1, TT2 T2> inline PTR<T1,T2>::PTR (): rep(NULL) {}        \
  template<TT1 T1, TT2 T2> inline PTR<T1,T2>::PTR (const PTR<T1,T2>& x):  \
    rep(x.rep) { INC_COUNT_NULL (this->rep); }                            \
  template<TT1 T1, TT2 T2> inline PTR<T1,T2>::~PTR () {                   \
    DEC_COUNT_NULL (this->rep); }                                         \
  template<TT1 T1, TT2 T2> PTR##_rep<T1,T2>* PTR<T1,T2>::operator -> () { \
    return this->rep; }                                                   \
  template<TT1 T1, TT2 T2>                                                \
  inline PTR<T1,T2>& PTR<T1,T2>::operator = (PTR<T1,T2> x) {              \
    INC_COUNT_NULL (x.rep); DEC_COUNT_NULL (this->rep);                   \
    this->rep=x.rep; return *this; }                                      \
  template<TT1 T1, TT2 T2> inline bool is_nil (PTR<T1,T2> x) {               \
    return x.rep==NULL; }
// end concrete_null

// abstract_null
#define ABSTRACT_NULL(PTR) \
  CONCRETE_NULL (PTR);     \
  inline PTR (PTR##_rep*)
#define ABSTRACT_NULL_CODE(PTR)      \
  CONCRETE_NULL_CODE (PTR);          \
  inline PTR::PTR (PTR##_rep* rep2): \
    rep(rep2) { INC_COUNT_NULL (this->rep); }
#define ABSTRACT_NULL_TEMPLATE(PTR,T) \
  CONCRETE_NULL_TEMPLATE (PTR,T);     \
  inline PTR (PTR##_rep<T>*)
#define ABSTRACT_NULL_TEMPLATE_CODE(PTR,TT,T)              \
  CONCRETE_NULL_TEMPLATE_CODE (PTR,TT,T);                  \
  template<TT T> inline PTR<T>::PTR (PTR##_rep<T>* rep2):  \
    rep(rep2) { INC_COUNT (this->rep); }
#define ABSTRACT_NULL_TEMPLATE_2(PTR,T1,T2) \
  CONCRETE_NULL_TEMPLATE_2 (PTR,T1,T2);     \
  inline PTR (PTR##_rep<T1,T2>*)
#define ABSTRACT_NULL_TEMPLATE_2_CODE(PTR,TT1,T1,TT2,T2)                    \
  CONCRETE_NULL_TEMPLATE_2_CODE (PTR,TT1,T1,TT2,T2);                          \
  template<TT1 T1,TT2 T2> inline PTR<T1,T2>::PTR (PTR##_rep<T1,T2>* rep2):  \
    rep(rep2) { INC_COUNT (this->rep); }
// end abstract_null

/******************************************************************************
* extensions
******************************************************************************/

#define EXTEND(BASE,PTR) \
  ABSTRACT(PTR);         \
  inline PTR(BASE&);     \
  inline operator BASE ()
#define EXTEND_CODE(BASE,PTR)             \
  ABSTRACT_CODE(PTR);                     \
  inline PTR::PTR(BASE& x):               \
    rep(static_cast<PTR##_rep*>(x.rep)) { \
    INC_COUNT (this->rep); }              \
  inline PTR::operator BASE () { return BASE (this->rep); }
// end extend

// extend_null
#define EXTEND_NULL(BASE,PTR) \
  ABSTRACT_NULL(PTR);         \
  inline PTR(BASE&);          \
  inline operator BASE ()
#define EXTEND_NULL_CODE(BASE,PTR)        \
  ABSTRACT_NULL_CODE(PTR);                \
  inline PTR::PTR(BASE& x):               \
    rep(static_cast<PTR##_rep*>(x.rep)) { \
    INC_COUNT_NULL(this->rep); }          \
  inline PTR::operator BASE () { return BASE (this->rep); }

#define EXTEND_NULL_TEMPLATE(BASE,PTR,T) \
  ABSTRACT_NULL_TEMPLATE(PTR,T);         \
  inline PTR<T>(BASE&);                  \
  inline operator BASE ()
#define EXTEND_NULL_TEMPLATE_CODE(BASE,PTR,TT,T) \
  ABSTRACT_NULL_TEMPLATE_CODE(PTR,TT,T);         \
  template<TT T> inline PTR<T>::PTR(BASE& x):    \
    rep(static_cast<PTR##_rep<T>*>(x.rep)) {     \
    INC_COUNT_NULL(this->rep); }                 \
  template<TT T> inline PTR<T>::operator BASE () { return BASE (this->rep); }
// end extend_null

#endif // defined BASIC_H
