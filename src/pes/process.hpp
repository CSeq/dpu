
Process::Process (Event *creat) :
   counters {1}
{
   //DEBUG ("Process.ctor: this %p pid %d sizeof %d", this, pid(), sizeof (Process));

   // construct the first event box
   Eventbox *b = first_box ();
   new (b) Eventbox (0);

   // construct a THSTART event in the first box
   Event *e = b->event_above ();
   new (e) Event (creat);
   e->flags.boxfirst = 1;

   // initialize the pointer to the last event created
   last = b->event_above();
}

EventIt Process::begin ()
{
   return EventIt (first_event());
}
EventIt Process::end ()
{
   return EventIt (last + 1);
}

unsigned Process::pid () const
{
   return Unfolding::ptr2pid (this);
}
size_t Process::offset (void *ptr) const
{
   return ((size_t) ptr) - (size_t) this;
}

Event *Process::first_event () const
{
   return first_box()->event_above();
}

Eventbox *Process::first_box () const
{
   return ((Eventbox *) (this + 1));
}

/// THSTART, 0 process predecessor, 1 predecessor (THCREAT) in another process
Event *Process::add_event_0p (Event *creat)
{
   Event *e;

   ASSERT (last); // we have a last
   ASSERT (pid() == last->pid()); // last point inside us
   ASSERT (! last->flags.boxlast); // box should be open

   // check for available space
   have_room ();

   // close the box, add a new box, insert the THSTART
   last->flags.boxlast = 1;
   Eventbox *b = new (last + 1) Eventbox (0); // null pre-proc
   e = b->event_above ();
   new (e) Event (creat);
   e->flags.boxfirst = 1;

   // update the last pointer and the counters
   last = e;
   counters.events++;
   return e;
}

/// THCREAT(tid), THEXIT(), one predecessor (in the process)
Event *Process::add_event_1p (Action ac, Event *p)
{
   Event *e;

   ASSERT (last); // we have a last
   ASSERT (pid() == last->pid()); // last point inside us
   ASSERT (! last->flags.boxlast); // box should be open

   // check for available space
   have_room ();

   if (p == last)
   {
      // add one event at the end of the pool
      e = last + 1;
      new (e) Event (ac, false);
   }
   else
   {
      // create one box and add the event
      last->flags.boxlast = 1;
      Eventbox *b = new (last + 1) Eventbox (p);
      e = b->event_above ();
      new (e) Event (ac, true);
      e->flags.boxfirst = 1;
   }

   counters.events++;
   last = e;
   return e;
}

/// THJOIN(tid), MTXLOCK(addr), MTXUNLK(addr), two predecessors (process, memory/exit)
Event * Process::add_event_2p (Action ac, Event *p, Event *m)
{
   Event *e;

   ASSERT (last); // we have a last
   ASSERT (pid() == last->pid()); // last point inside us
   ASSERT (! last->flags.boxlast); // box should be open

   // check for available space
   have_room ();

   if (p == last)
   {
      // FIXME: errors when adding the first LOCK
      // add one event at the end of the pool
      e = last + 1;
      new (e) Event (ac, m, false);
   }
   else
   {
      // create one box and add the event
      last->flags.boxlast = 1;
      Eventbox *b = new (last + 1) Eventbox (p);
      e = b->event_above ();
      new (e) Event (ac, m, true);
      e->flags.boxfirst = 1;
   }

   counters.events++;
   last = e;
   return e;
}

void Process::have_room () const
{
   // check that we have enough memory
   // this->last points to the last event
   // last+1 is the base address of the new one; last+3 is the upper limit of
   // one event + one box
   static_assert (sizeof (Event) >= sizeof (Eventbox), "Event should be larger than Eventbox");
   if (offset (last + 3) >= Unfolding::PROC_SIZE)
   {
      throw std::range_error (fmt
            ("Process %d: failure to add new events: out of memory", pid ()));
   }
}
