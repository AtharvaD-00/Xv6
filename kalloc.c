// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld

// Maintains the list of free frames with the help of a queue, implemented using a singly linked list

struct frame {
  struct frame *next;
};

struct list {
  struct frame *head;
  struct frame *tail;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct list freelist;
} kmem;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;

  kmem.freelist.head = ((void *) 0);
  kmem.freelist.tail = ((void *) 0);

  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct frame *f;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);

  f = (struct frame*) v;
  f->next = ((void *) 0);
  if (kmem.freelist.tail)
    kmem.freelist.tail->next = f;
  else
    kmem.freelist.head = f;
  kmem.freelist.tail = f;

  if(kmem.use_lock)
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct frame *f;

  if(kmem.use_lock)
    acquire(&kmem.lock);

  f = kmem.freelist.head;
  if (f) {
    kmem.freelist.head = f->next;
    f->next = ((void *) 0);
    if (! kmem.freelist.head)
      kmem.freelist.tail = ((void *) 0);
  }

  if(kmem.use_lock)
    release(&kmem.lock);

  return (char*)f;
}

