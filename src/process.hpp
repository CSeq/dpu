
inline Process::Process (Event *creat)
{
   DEBUG ("Process.ctor: this %p pid %d sizeof %d", this, pid(), sizeof (Process));

   // construct the first event box
   EventBox *b = first_box ();
   new (b) EventBox (0);

   // construct a THSTART event in the first box
   Event *e = b->event_above ();
   new (e) Event (creat);
   e->flags.boxfirst = 1;

   // initialize the pointer to the last event created
   last = b->event_above();
}

inline EventIt Process::begin ()
{
   return EventIt (first_event());
}
inline EventIt Process::end ()
{
   return EventIt (last + 1);
}

inline unsigned Process::pid () const
{
   return Unfolding::ptr2pid (this);
}
inline size_t Process::offset (void *ptr) const
{
   return ((size_t) ptr) - (size_t) this;
}

inline Event *Process::first_event () const
{
   return first_box()->event_above();
}

inline EventBox *Process::first_box () const
{
   return ((EventBox *) (this + 1));
}

/// THCREAT(tid), THEXIT(), one predecessor (in the process)
inline Event *Process::add_event_1p (Action ac, Event *p)
{
   Event *e;

   ASSERT (last);
   ASSERT (pid() == last->pid());
   ASSERT (! last->flags.boxlast);

   // check that we have enough memory
   // last points to the last event
   // last+1 is the base address of the new one; last+3 is the upper limit of
   // one event + one box
   static_assert (sizeof (Event) >= sizeof (EventBox), "Event should be larger than EventBox");
   if (offset (last + 3) >= Unfolding::PROC_SIZE)
   {
      throw std::range_error (fmt
            ("Process %d: failure to add new events: out of memory", pid ()));
   }

   if (p == last)
   {
      // add one event at the end of the pool
      e = last + 1;
      new (e) Event (ac);
   }
   else
   {
      // create one box and add the event
      last->flags.boxlast = 1;
      EventBox *b = new (last + 1) EventBox (p);
      e = b->event_above ();
      new (e) Event (ac);
      e->flags.boxfirst = 1;
   }

   last = e;
   return e;
}

/// THJOIN(tid), MTXLOCK(addr), MTXUNLK(addr), two predecessors (process, memory/exit)
inline Event * Process::add_event_2p (Action ac, Event *p, Event *m)
{
   Event *e;

   ASSERT (last);
   ASSERT (pid() == last->pid());
   ASSERT (! last->flags.boxlast);

   // check that we have enough memory
   // last points to the last event
   // last+1 is the base address of the new one; last+3 is the upper limit of
   // one event + one box
   static_assert (sizeof (Event) >= sizeof (EventBox), "Event should be larger than EventBox");
   if (offset (last + 3) >= Unfolding::PROC_SIZE)
   {
      throw std::range_error (fmt
            ("Process %d: failure to add new events: out of memory", pid ()));
   }

   if (p == last)
   {
      // add one event at the end of the pool
      e = last + 1;
      new (e) Event (ac, m);
   }
   else
   {
      // create one box and add the event
      last->flags.boxlast = 1;
      EventBox *b = new (last + 1) EventBox (p);
      e = b->event_above ();
      new (e) Event (ac, m);
      e->flags.boxfirst = 1;
   }

   last = e;
   return e;
}
