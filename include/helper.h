#ifndef HELPER_H
#define HELPER_H
#include "sfmm.h"

#define MAXALLOC 16384
#define MINALLOC 32

#define PAYLOAD(add)       	(void *) add + SF_HEADER_SIZE
#define FLH_BLOCKSIZE(flh) 	(flh->header.block_size << 4)
#define BLOCKSIZE(hf)      	(hf->block_size << 4)
#define FOOTERADDR(base, size)	(sf_footer *)((void *) base + size - SF_FOOTER_SIZE)

#define HFSIZE (SF_HEADER_SIZE + SF_FOOTER_SIZE)

extern int npages;
extern void *heap_start;

void set_prev_next(sf_free_header *next, sf_free_header *prev)
{
	if (prev != NULL)
		prev->next = next;
	if (next != NULL)
		next->prev = prev;
}

void *coalesce_right(void *nxt, void *curr)
{
	sf_free_header *next_block, *curr_block;
	sf_footer *next_header_footer;

	next_block = (sf_free_header *) nxt;
	curr_block = (sf_free_header *) curr;

	next_header_footer = (sf_footer *)((void *) next_block + FLH_BLOCKSIZE(next_block) - SF_FOOTER_SIZE);
	next_header_footer->block_size += FLH_BLOCKSIZE(curr_block) >> 4;
	curr_block->header.block_size = BLOCKSIZE(next_header_footer) >> 4;

	set_prev_next(next_block->next, next_block->prev);

	return (void *) curr_block;
}

void *coalesce_left(void *prv, void *curr, void *curr_ftr)
{
	sf_free_header *prev_block, *curr_block;
	sf_footer *curr_footer, *prev_footer;

	curr_block = (sf_free_header *) curr;
	curr_footer = (sf_footer *) curr_ftr;
	prev_footer = (sf_footer *) prv;

	prev_block = (sf_free_header *)((void *) curr_block - BLOCKSIZE(prev_footer));
	prev_block->header.block_size += FLH_BLOCKSIZE(curr_block) >> 4;

	curr_footer->block_size = FLH_BLOCKSIZE(prev_block) >> 4;

	set_prev_next(prev_block->next, prev_block->prev);

	return (void *) prev_block;
}

void *coalesce_middle(void *ptr)
{
	sf_free_header *curr_block;

	curr_block = (sf_free_header *) ptr;

	freelist_head = curr_block;
	freelist_head->prev = NULL;

	return (void *) freelist_head;
}

size_t realspace(size_t size)
{
	return size <= HFSIZE ? 2 * HFSIZE : HFSIZE * ((size + HFSIZE + (HFSIZE - 1)) / HFSIZE);
}

size_t nfreeblocks() 
{
	size_t nbytes = 0;

	sf_free_header *temp_flh = freelist_head;
	while (temp_flh) {
		nbytes += FLH_BLOCKSIZE(temp_flh);
		temp_flh = temp_flh->next;
	}
	return nbytes;
}

void *setupsfh(sf_header *sf, uint64_t alloc, uint64_t block_size, uint64_t padding_size)
{
	sf->alloc = alloc;
	sf->block_size = block_size;
	sf->padding_size = padding_size;

	return sf;
}

void *setupf(sf_footer *sf_f, uint64_t alloc, uint64_t block_size)
{
	sf_f->block_size = block_size;
	sf_f->alloc = alloc;

	return sf_f;
}

void *setupsffh(sf_free_header *sfh, uint64_t alloc, uint64_t block_size, uint64_t padding_size)
{
	sfh->header.padding_size = padding_size;
	sfh->header.block_size = block_size;
	sfh->header.alloc = alloc;

	sfh->next = sfh->prev = NULL;

	return sfh;
}


#endif