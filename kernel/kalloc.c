// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

uint64 MAX_PAGES = 0;
uint64 FREE_PAGES = 0;
// My list for the reference count
int refcnt[PHYSTOP / PGSIZE];

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run
{
    struct run *next;
};

struct
{
    struct spinlock lock;
    struct run *freelist;
} kmem;

void kinit()
{
    initlock(&kmem.lock, "kmem");
    freerange(end, (void *)PHYSTOP);
    MAX_PAGES = FREE_PAGES;
}

// Freas the space for the refcnt varible
void freerange(void *pa_start, void *pa_end)
{
    char *p;
    p = (char *)PGROUNDUP((uint64)pa_start);
    for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE)
    {
        refcnt[(uint64)p / PGSIZE] = 1;
        kfree(p);
    }
}

// Increses the shared refcnt on a new process
void increse(uint64 pa)
{
    // acquire the lock
    acquire(&kmem.lock);
    int pn = pa / PGSIZE;
    if (pa > PHYSTOP || refcnt[pn] < 1)
    {
        panic("increase ref cnt");
    }
    refcnt[pn]++;
    release(&kmem.lock);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
// Changes some of the code here to implemet the vfork
void kfree(void *pa)
{
    if (MAX_PAGES != 0)
        assert(FREE_PAGES < MAX_PAGES);

    struct run *r;
    r = (struct run *)pa;
    if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
        panic("kfree");

    acquire(&kmem.lock);
    int pn = (uint64)r / PGSIZE;
    if (refcnt[pn] < 1)
        panic("kfree panic");
    refcnt[pn] -= 1;
    int tmp = refcnt[pn];
    release(&kmem.lock);

    if (tmp > 0)
        return;

    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    FREE_PAGES++;
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
// Altered by me
void *
kalloc(void)
{
    assert(FREE_PAGES > 0);
    struct run *r;

    acquire(&kmem.lock);
    r = kmem.freelist;

    if (r)
    {
        int pn = (uint64)r / PGSIZE;
        if (refcnt[pn] != 0)
        {
            panic("refcnt kalloc");
        }
        refcnt[pn] = 1;
        kmem.freelist = r->next;
    }

    release(&kmem.lock);

    if (r)
        memset((char *)r, 5, PGSIZE); // fill with some junk
    FREE_PAGES--;
    return (void *)r;
}
