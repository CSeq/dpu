/*
 * pes.cc
 *
 *  Created on: Jan 12, 2016
 *      Author: tnguyen
 */

#include <cstdint>
#include <cassert>
#include <iostream>

#include "pes.hh"
#include "misc.hh"
#include "verbosity.h"

using std::vector;
using namespace ir;

namespace pes{

/*
 * Methods for class Event
 */
Event::Event()
   : pre_proc(nullptr)
   , pre_mem(nullptr)
   , val(0)
   , localvals(0)
   , trans(nullptr)
{
   DEBUG ("%p: Event.ctor:", this);
}

Event::Event(Trans & t)
   : pre_proc(nullptr)
   , pre_mem(nullptr)
   , val(0)
   , localvals(0)
   , trans(&t)

{
   DEBUG ("%p: Event.ctor: t %p '%s'", this, &t, t.str().c_str());
}

bool Event::is_bottom ()
{
   return this->pre_proc == this;
}

//set up 3 attributes: pre_proc, pre_mem and pre_readers and update event's parents
void Event::mk_history(Config & c)
{
   /*
    * For all events:
    * - pre_proc    is the latest event of the same process in c
    * - post_proc, post_mem, and post_rws
    *               remain empty
    *
    * For RD
    * - pre_mem     is the lastest operation on the same process inside of c
    * - pre_readers remains empty
    *
    * For WR
    * - pre_mem     is the lastest WR in c
    * - pre_readers is the lastest OP of ALL processes in c
    *
    * For LOC
    * - pre_mem     is NULL
    * - pre_readers remains empty
    */
   ir::Process & p = this->trans->proc;
   std::vector<Process> & procs = c.unf.m.procs;

   //e's parent is the latest event of the process

   // for all events, initialize pre_proc
   pre_proc = c.latest_proc[p.id];

   switch (trans->type)
   {
      case ir::Trans::RD:
         pre_mem  = c.latest_op[p.id][trans->var];

         // pre_readers stays empty for RD events
         break;

      case ir::Trans::WR:
         pre_mem  = c.latest_wr[trans->var];
        // set pre-readers = set of latest events which use the variable copies of all processes.
                //size of pre-readers is numbers of copies of the variable = number of processes
         for (auto & pr : procs)
            pre_readers.push_back(c.latest_op[pr.id][trans->var]);
         break;

      case ir::Trans::SYN:
         pre_mem  = c.latest_wr[trans->var];
         break;

      case ir::Trans::LOC:
         pre_mem   = nullptr;
         break;
   }
   DEBUG (" History %s", this->str().c_str());
  // update_parents();
}

void Event::update_parents()
{
#if 0
   if (this->is_bottom())
      return;

   //Process & p  = trans->proc;
   Event * prt1;
  // Event * prt2;
   prt1 = this->pre_proc;
  // prt2 = this->pre_mem;
   //prt1->post_proc.push_back(this)  ; // parent 1 = pre_proc

    DEBUG ("%s \n", pre_proc->str().c_str());

   //parent 2 = pre_mem
  // DEBUG("%s \n", (*pre_mem).trans->type_str());
 //  assert(prt2 != nullptr);
   switch (prt2->trans->type)
   {
      case ir::Trans::WR:
       printf("this is a WR");
       prt2->post_mem[p.id].push_back(this); // add a child to vector corresponding to process p
       if (trans->type == ir::Trans::WR)  //if the event itsefl is a WR, add it to parentś post_wr
           prt2->post_wr[p.id].push_back(this);
        break;
      case ir::Trans::RD:
       printf("this is a RD");
       prt2->post_rws.push_back(this);
         break;
      case ir::Trans::SYN:
         prt2->post_rws.push_back(this);
        break;
      case ir::Trans::LOC:
        return;
         break;
   }
#endif

}

bool Event:: operator == (const Event & e) const
{
   return this == &e;
}

Event & Event:: operator  = (const Event & e)
{
   *pre_proc = *(e.pre_proc);

   for (auto& it:post_proc)
   for (auto& i:e.post_proc)
      it = i;

   *pre_mem = *(e.pre_mem);

   for (auto& it:post_mem)
   for (auto& i:e.post_mem)
      it = i;

   for (auto& it:pre_readers)
   for (auto& i :e.pre_readers)
      it = i;

   for (auto& it:post_wr)
   for (auto& i :e.post_wr)
      it = i;

   for (auto& it:post_rws)
   for (auto& i :e.post_rws)
      it = i;

   val = e.val;

   for (auto& it:localvals)
   for (auto& i :e.localvals)
      it = i;

   trans = e.trans;

   return *this;
}

bool Event::check_cfl(Event & e)
{
   printf("start check conflict");
   Event & parent = *(e.pre_mem);
   ir::Trans & trans = *(parent.trans);

   switch (trans.type)
   {
      case ir::Trans::RD:
       for (auto const it: parent.post_rws)
          if (*it == e)
              return true;
       break;

     case ir::Trans::WR: // only access one variable
        for (auto const proc : parent.post_mem)
         for (auto const it: proc)
              if (*it == e)
                 return true;
        break;

     case ir::Trans::SYN:
        for (auto const i: post_rws )
        if (*i == e)
           return true;
        break;

     case ir::Trans::LOC:
        Event & parent_loc = *(e.pre_proc);
        for (auto const i: parent_loc.post_proc)
        if (*i == e)
         return true;
        break;
   }

   return false;
}


std::string Event::str () const
{
   const char * code = trans ? trans->code.str().c_str() : "";
   return fmt ("%p: trans %p code: '%s' pre_proc %p pre_mem %p",
         this, trans, code, pre_proc, pre_mem);
}

/*
 * Methods of class Config
 */

Config::Config (Unfolding & u)
   : gstate (u.m.init_state)
   , latest_proc (u.m.procs.size (), u.bottom)
   , latest_wr (u.m.memsize, u.bottom)
   , latest_op (u.m.procs.size (), std::vector<Event*> (u.m.memsize, u.bottom))
   , unf (u)
{
  // DEBUG ("%p: Config.ctor: u %p", this, unf);

   print_debug ();

   // initialize all attributes for an empty config
   __update_encex (*unf.bottom);
}

void Config::print_debug ()
{
   
   DEBUG ("%p: latest_proc:", this);
   for (auto & e : latest_proc) DEBUG (" %s", e->str().c_str());
   DEBUG ("%p: latest_wr:", this);
   for (auto & e : latest_wr) DEBUG (" %s", e->str().c_str());
   DEBUG ("%p: latest_op:", this);
   for (size_t pid = 0; pid < latest_op.size(); ++pid)
   {
      DEBUG (" Process %zu", pid);
      for (auto & e : latest_op[pid]) DEBUG ("  %s", e->str().c_str());
   }
}

#if 0
Config:: Config (const Config & c)
   : latest_proc (c.latest_proc)
   , latest_wr (c.latest_wr)
   , latest_op (c.latest_op)
   , latest_local_wr (c.latest_local_wr)
   , unf(c.unf)
{
   gstate = c.gstate;
}
#endif

/*
 * Add an event to a configuration
 */

void Config::add_any ()
{
   add (en.size () - 1);
}

void Config::add (Event & e)
{
   for (size_t i = 0; i < en.size (); ++i)
      if (e == *en[i]) add (i);
   throw std::range_error ("Trying to fire event not enabled by a configuration");
}

void Config::add (unsigned idx)
{
   Event & e = *en[idx];
   DEBUG (" Event e: %s \n", e.str().c_str());
   ir::Process & p              = e.trans->proc;
   std::vector<Process> & procs = unf.m.procs;

   // update the configuration
   e.trans->fire (gstate); //update new global states

   latest_proc[p.id] = &e; //update latest event of the process

   printf("latest event of the proc %d is: %p \n", p.id, latest_proc[p.id]);

   //update local variables in trans
   for (auto i: e.trans->localvars)
       latest_wr[i] = &e;

   //update particular attributes according to type of transition.
   switch (e.trans->type)
   {
   case ir::Trans::RD:
      latest_op[p.id][e.trans->var] = &e;
      break;

   case ir::Trans::WR:
      latest_wr[e.trans->var] = &e; // update latest wr event for the variable s.var
      for (unsigned int i = 0; i < procs.size(); i++)
         latest_op[i][e.trans->var] = &e;
      break;

   case ir::Trans::SYN:
      latest_wr[e.trans->var]=&e;
      break;

   case ir::Trans::LOC:
      latest_proc[p.id] = &e;
      break;
   }

   // remove the event en[idx] from the enabled set
   en[idx] = en.back();
   en.pop_back();

   this->print_debug();
   __update_encex(e);
}

void Config::__update_encex (Event &  )
{
   printf("start update_encex\n");
#if 0
   if (en.size() > 0)
      remove_cfl(e);
   else
      printf("Oh la la \n");
#endif

   std::vector<ir::Trans> & trans = unf.m.trans;
   std::vector <ir::Process> & procs = unf.m.procs;
   assert(trans.size() > 0);
   assert(procs.size() > 0);

   std::vector<Trans*> enabled;

   // FIXME !! Modify here !!
   gstate.enabled (enabled);
   for (auto t : enabled)
   {
      DEBUG ("%p is enabled", t);
   }

   // create one event for each transition enabled at unf.gstate
   for (auto & t: trans)
   {
      DEBUG("%s", t.str().c_str());
    //  printf("t.src: %d and t.dest: %d, t.proc.id: %d ", t.src, t.dst, t.proc.id);
     if (t.enabled(gstate) == true)
     {
      printf("is enabled\n");
      unf.evt.emplace_back(t);
        printf("A new event created at: %p\n",&unf.evt.back());
        unf.evt.back().mk_history(*this); // create an history for new event
      en.push_back(&unf.evt.back());
     }
     else
      printf("is not enabled\n");
   }

   printf("en.size: %zu\n", en.size());
}

void Config::remove_cfl(Event & e)
{
   printf("start remove conflict events \n");
   unsigned int i = 0;
   while (i < en.size())
   {
      printf("Lan thu i %d \n", i);
     if (e.check_cfl(*en[i]) == true)
      {
        cex.push_back(en[i]);
         en.erase(en.begin() + i);
      }
      else
       i++;
   }
   printf("\nfinish remove cfl, en.size %zu \n", en.size());
}

/*
 * Methods for class Unfolding
 */
Unfolding::Unfolding (ir::Machine & ma)
   : m (ma)
{
   DEBUG ("%p: Unfolding.ctor: m %p", this, &m);
   __create_botom ();
}

void Unfolding::__create_botom ()
{
   Event * e;

   // create an "bottom" event with all empty
   evt.emplace_back();
   e = & evt[0];

   e->pre_proc = e;
   e->pre_mem = e;

   bottom = e; 
   DEBUG ("%p: Unfolding.__create_bottom: bottom %p", this, e);
}

void Unfolding:: explore(Config & C, std::vector<Event*> D, std::vector<Event*> A)
{
   Event * pe;
   if (C.en.empty() == true) return ;
   if (A.empty() == true)
       pe = *(C.en.begin()); // choose the first element
   else
   { //choose the mutual event in A and C.en to add
      for (auto a = A.begin(); a != A.end(); a++)
       for (auto e = C.en.begin(); e!= C.en.end(); e++)
          if (*e == *a)
             {
                pe = *e;
                A.erase(a);
                C.en.erase(e);
                break;
             }
   }
   C.add(*pe);
   // Alt(C,D.add(e))
   explore (C, D, A);
}

/*
 * Explore a random configuration
 */
void Unfolding::explore_rnd_config ()
{
   Event * e;
   printf ("--------Start Unfolding.explore_rnd_config\n");
   // assert (evt.size () == 0);
   printf("Creat an empty config:\n");
   Config c(*this);

   c.print_debug ();
   return;

   while (c.en.empty() == false)
   {
      e = c.en.back();
      c.add(*e);
   }

#if 0
   if (c.en.empty() == false)
      {
         e = c.en.back();
         c.add(*e);
      }
#endif
   printf("no more enabled");
}

} // end of namespace

