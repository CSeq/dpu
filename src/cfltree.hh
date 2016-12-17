
#ifndef __CFLTREE_HH_
#define __CFLTREE_HH_

namespace dpu{

template <class T, int SS>
class Node
{
public:
   /// depth of this node in the tree
   unsigned depth;
   /// immediate predecessor
   T * pre;
   /// skiptab[i] is the predecessors at distance SS^(i+1), for i < skiptab_size()
   T ** skiptab;

   /// constructor; idx is the position of this node in the MultiNode class; pr
   /// is the immediate predecessor
   inline Node (int idx, T *pr);
   inline ~Node ();

   template <int idx>
   inline const T *find_pred (unsigned d) const;
   template <int idx>
   inline T *find_pred (unsigned d);

   template <int idx>
   void dump () const;

private:
   /// allocates and initializes the skiptab, called from the ctor
   inline T **skiptab_alloc (int idx);

   /// computes the size of the skiptab table
   inline unsigned skiptab_size () const;

   /// return the best predecessor available from this node that allows to
   /// reach in the minimum number of steps a node at the given target depth
   inline T *best_pred (unsigned target) const;
};

//-------template class MultiNode------
template <class T, int SS> // SS: skip step
class MultiNode
{
public:
   Node<T,SS> node[2];

  // inline MultiNode();
   inline MultiNode(T *pred0, T *pred1);
   inline MultiNode(T *pred0);
};

#include "cfltree.hpp"

} // end of namespace

#endif

