
#include "heap.h"

#define ALIGN 16
#define PAGE_SIZE 4096

typedef uint64_t header_t;
static const header_t ALLOC_BIT = 1ULL;

static inline size_t align_up(size_t x, size_t a) {
  return (x + a - 1) & ~(a - 1);
}
static inline size_t pack_size(size_t size, int used) {
  return (size & ~ALLOC_BIT) | (used ? ALLOC_BIT : 0);
}
static inline size_t unpack_size(header_t h) {
  return (size_t)(h & ~ALLOC_BIT);
}
static inline int is_alloc(header_t h) { return (h & ALLOC_BIT) != 0; }

static inline size_t header_size(void) { return sizeof(header_t); }
static inline size_t footer_size(void) { return sizeof(header_t); }

typedef struct free_block {
  header_t header;
  struct free_block *prev;
  struct free_block *next;
} free_block_t;

static free_block_t free_list_sentinel = {.prev = &free_list_sentinel,
                                          .next = &free_list_sentinel};

static void *heap_low = NULL;
static void *heap_high = NULL;

/* Bump allocator state for internal growth (carves from reserved linker region)
 */
static uintptr_t heap_bump = 0;
static uintptr_t heap_limit = 0;

/* Initialize bump pointers from linker symbols; call once before allocations */
static void init_heap_region(void) {
  if (heap_low)
    return; /* already initialized */
  heap_low = (void *)__heap_start;
  heap_high = (void *)__heap_end;
  heap_bump = (uintptr_t)__heap_start;
  heap_limit = (uintptr_t)__heap_end;

  size_t initial_size = heap_limit - heap_bump;
  if (initial_size >= (header_size() + footer_size() + sizeof(free_block_t))) {
    void *blk = (void *)heap_bump;
    header_t *h = (header_t *)blk;
    *h = pack_size(initial_size, 0);
    header_t *f = (header_t *)((char *)blk + initial_size - footer_size());
    *f = pack_size(initial_size, 0);
    free_block_t *fb = (free_block_t *)blk;
    fb->prev = fb->next = NULL;
    free_list_sentinel.next = free_list_sentinel.prev = fb;
    fb->prev = &free_list_sentinel;
    fb->next = &free_list_sentinel;
  } else {
    free_list_sentinel.next = free_list_sentinel.prev = &free_list_sentinel;
  }
}

/* Free-list helpers */
static void free_list_insert(free_block_t *fb) {
  fb->next = free_list_sentinel.next;
  fb->prev = &free_list_sentinel;
  free_list_sentinel.next->prev = fb;
  free_list_sentinel.next = fb;
}
static void free_list_remove(free_block_t *fb) {
  fb->prev->next = fb->next;
  fb->next->prev = fb->prev;
}

static inline void *block_to_payload(void *block) {
  return (char *)block + header_size();
}
static inline void *payload_to_block(void *payload) {
  return (char *)payload - header_size();
}

static void init_free_block(void *block, size_t size) {
  header_t *h = (header_t *)block;
  *h = pack_size(size, 0);
  header_t *f = (header_t *)((char *)block + size - footer_size());
  *f = pack_size(size, 0);
  free_block_t *fb = (free_block_t *)block;
  fb->prev = fb->next = NULL;
}

/* Coalesce with adjacent free blocks, return pointer to coalesced block start
 */
static void *coalesce(void *block) {
  char *b = (char *)block;
  header_t *h = (header_t *)b;
  size_t sz = unpack_size(*h);

  /* next block */
  char *next = b + sz;
  if (next < (char *)heap_high) {
    header_t next_h = *(header_t *)next;
    if (!is_alloc(next_h)) {
      size_t next_sz = unpack_size(next_h);
      free_block_t *next_fb = (free_block_t *)next;
      free_list_remove(next_fb);
      sz += next_sz;
      *h = pack_size(sz, 0);
      header_t *new_footer = (header_t *)(b + sz - footer_size());
      *new_footer = pack_size(sz, 0);
    }
  }

  /* previous block via footer */
  if (b > (char *)heap_low) {
    header_t *prev_footer = (header_t *)(b - footer_size());
    header_t pf = *prev_footer;
    if (!is_alloc(pf)) {
      size_t prev_sz = unpack_size(pf);
      char *prev = b - prev_sz;
      free_block_t *prev_fb = (free_block_t *)prev;
      free_list_remove(prev_fb);
      sz += prev_sz;
      header_t *new_header = (header_t *)prev;
      *new_header = pack_size(sz, 0);
      header_t *new_footer = (header_t *)(prev + sz - footer_size());
      *new_footer = pack_size(sz, 0);
      return prev;
    }
  }

  return block;
}

/* Grow the allocator by carving pages from the reserved region (bump) */
static void *grow_from_region(size_t min_bytes) {
  size_t want = align_up(min_bytes, PAGE_SIZE);
  uintptr_t addr = align_up(heap_bump, PAGE_SIZE);
  if (addr + want > heap_limit)
    return NULL;
  heap_bump = addr + want;
  void *mem = (void *)addr;
  init_free_block(mem, want);
  free_list_insert((free_block_t *)mem);
  return mem;
}

void heap_init(void) { init_heap_region(); }

void *kmalloc(size_t size) {
  if (!heap_low)
    init_heap_region();
  if (size == 0)
    size = 1;
  size_t total = align_up(size + header_size(), ALIGN);
  size_t min_block = sizeof(free_block_t) + header_size() + footer_size();
  if (total < min_block)
    total = min_block;
  size_t total_with_footer = total + footer_size();

  for (free_block_t *it = free_list_sentinel.next; it != &free_list_sentinel;
       it = it->next) {
    header_t h = it->header;
    size_t bsz = unpack_size(h);
    if (bsz >= total_with_footer) {
      free_list_remove(it);
      size_t remaining = bsz - total_with_footer;
      void *alloc_block = (void *)it;
      if (remaining >= min_block) {
        /* split */
        *(header_t *)alloc_block = pack_size(total_with_footer, 1);
        header_t *alloc_footer =
            (header_t *)((char *)alloc_block + total_with_footer -
                         footer_size());
        *alloc_footer = pack_size(total_with_footer, 1);

        void *rem_block = (char *)alloc_block + total_with_footer;
        init_free_block(rem_block, remaining);
        free_list_insert((free_block_t *)rem_block);
        return block_to_payload(alloc_block);
      } else {
        /* give whole block */
        *(header_t *)alloc_block = pack_size(bsz, 1);
        header_t *alloc_footer =
            (header_t *)((char *)alloc_block + bsz - footer_size());
        *alloc_footer = pack_size(bsz, 1);
        return block_to_payload(alloc_block);
      }
    }
  }

  /* no fit: grow by carving from reserved region and retry */
  if (!heap_low)
    return NULL;
  size_t need = total_with_footer;
  void *mem = grow_from_region(need);
  if (!mem)
    return NULL;
  return kmalloc(size);
}

void kfree(void *ptr) {
  if (!ptr)
    return;
  if (!heap_low)
    init_heap_region();
  void *block = payload_to_block(ptr);
  header_t *h = (header_t *)block;
  size_t sz = unpack_size(*h);
  *h = pack_size(sz, 0);
  header_t *f = (header_t *)((char *)block + sz - footer_size());
  *f = pack_size(sz, 0);

  void *nb = coalesce(block);
  free_block_t *fb = (free_block_t *)nb;
  free_list_insert(fb);
}
