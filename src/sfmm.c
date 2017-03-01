#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "sfmm.h"
#include "helper.h"

/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */

sf_free_header *freelist_head = NULL;

/* Where Am i supposset to put this */
int npages;
void *heap_start;

static size_t internal;
static size_t external; /* only the free blocks */
static size_t allocations;
static size_t frees;
static size_t coalesces;

void *sf_malloc(size_t size)
{
	size_t act_size = realspace(size), splinter_size = 0;
	uint64_t prev_block_size;

	sf_free_header *temp_flh, *next, *prev, *prev_flh;
	sf_header *new_header;
	sf_footer *new_footer;

	errno = 0;
	if (size <= 0 || act_size > MAXALLOC) {
		errno = EINVAL;
		return NULL;
	}

	temp_flh = freelist_head;
	/*
	 *
	 * New allocated blocks become new freelist head coalesce or not
	 *
	 */

	if (heap_start == NULL)
		heap_start = sf_sbrk(0);

	while (temp_flh != NULL && act_size > FLH_BLOCKSIZE(temp_flh))
		temp_flh = temp_flh->next;

	while (temp_flh == NULL || act_size > FLH_BLOCKSIZE(temp_flh)) {
		sf_footer *footer;

		errno = 0;
		if (npages == 4) {
			errno = ENOMEM;
			external = nfreeblocks();

			return NULL;
		}

		temp_flh = sf_sbrk(0);

		sf_sbrk(1);
		memset(temp_flh, 0, 4096);

		temp_flh = setupsffh(temp_flh, 0, (4096 >> 4), 0);

		if (npages > 0) {
			footer = (void *) temp_flh - SF_FOOTER_SIZE;

			if (footer->alloc == 0) {
				temp_flh = (sf_free_header *)((void *) temp_flh - BLOCKSIZE(footer));
				temp_flh->header.block_size += (4096 >> 4);

				// Add coalesce instance here
				coalesces++;
			}
		}

		footer = setupf(FOOTERADDR(temp_flh, FLH_BLOCKSIZE(temp_flh)), 0, temp_flh->header.block_size);

		if (freelist_head == NULL) {
			freelist_head = temp_flh;

		} else if (freelist_head != temp_flh) {
			prev_flh = freelist_head;

			set_prev_next(temp_flh->next, temp_flh->prev);

			freelist_head = temp_flh;
			freelist_head->prev = NULL;
			freelist_head->next = prev_flh;

			prev_flh->prev = freelist_head;

			// if (prev_flh->next == freelist_head)
			// 	prev_flh->next = NULL;
		}
		/* Increment npages */
		npages++;
	}

	/* Do operations on found free block */
	prev_block_size = FLH_BLOCKSIZE(temp_flh);
	prev = temp_flh->prev;
	next = temp_flh->next;

	/* Check for splinter here */
	splinter_size = prev_block_size - act_size < 32 ? prev_block_size - act_size : 0;

	/* Set header for the allocated block */
	new_header = setupsfh((sf_header *) temp_flh, 1, (act_size + splinter_size) >> 4, act_size - size - HFSIZE);

	/* Set footer for the allocated block */
	new_footer = setupf(FOOTERADDR(new_header, BLOCKSIZE(new_header)), 1, new_header->block_size);

	/* If the free block was fully allocated */
	if (BLOCKSIZE(new_header) == prev_block_size) {
		set_prev_next(next, prev);

		if (temp_flh == freelist_head) 
			freelist_head = freelist_head->next;

	} else {
		/* If there are leftovers, then set new header and footers and change ptr's */
		sf_free_header *leftovers;

		leftovers = setupsffh((sf_free_header *)((void *) new_footer + SF_FOOTER_SIZE), 
			0, (prev_block_size - BLOCKSIZE(new_header)) >> 4, 0);

		setupf(FOOTERADDR(leftovers, FLH_BLOCKSIZE(leftovers)), 0, leftovers->header.block_size);

		/* All leftovers should go to head of the list */
		prev_flh = freelist_head;

		set_prev_next(next, prev);

		freelist_head = leftovers;
		freelist_head->next = freelist_head->prev = NULL;

		if (temp_flh != prev_flh) {
			freelist_head->next = prev_flh;
			prev_flh->prev = freelist_head;

		} else {
			freelist_head->next = prev_flh->next;

			if (prev_flh->next != NULL)
				prev_flh->next->prev = freelist_head;
		}
		
	}

	/* Update internal frag, and external frag, and allocations stuff here */
	external = nfreeblocks();
	internal += new_header->padding_size + HFSIZE;
	allocations++;

	return PAYLOAD(new_header);
}

void sf_free(void *ptr) 
{
	/* Start of implementation of real shitty free ie NO ERROR HANDLING AT ALL for now...*/
	/* For future error handling, check ptr's header and footer and see if they match up for a valid address to free */
	/*
	 *
	 * REMEMBER TO CHECK RANGE OF PTR FOR INVALID READS AND STUFF
	 *
	 *
	 * REFACTOR COALESCING METHODS LATER ON
	 *
	 */

	 /* Shitty error handling for now */
	if (ptr == NULL)
		return;

	errno = 0;
	if (heap_start == NULL || ptr - SF_HEADER_SIZE < heap_start || (ptr - SF_HEADER_SIZE) > sf_sbrk(0) - MINALLOC) {
		errno = EINVAL;
		return;
	}

	sf_header *header;
	sf_free_header *next_header, *new_free_header, *curr_flh, *prev_footer_header;
	sf_footer *footer, *prev_footer;

	uint64_t old_block_size, old_padding_size;
	uint8_t prev, next;

	void *brk = sf_sbrk(0);

	header = (sf_header *)(ptr - SF_HEADER_SIZE);
	footer = FOOTERADDR(header, BLOCKSIZE(header));

	if (header->alloc == 0 && footer->alloc == 0)
		return;

	/* Error handling for invalid addresses */
	errno = 0;
	if (header->alloc == 0 || footer->alloc == 0 || BLOCKSIZE(header) != BLOCKSIZE(footer)) {
		errno = EINVAL;
		return;
	}

	next_header = (void *) footer + SF_FOOTER_SIZE != brk ? (void *) footer + SF_FOOTER_SIZE : NULL;
	prev_footer = (void *) header != heap_start ? (void *) header - SF_HEADER_SIZE : NULL;

	prev = (prev_footer == NULL) ? 0 : prev_footer->alloc == 0;
	next = (next_header == NULL) ? 0 : next_header->header.alloc == 0;

	old_block_size = header->block_size;
	old_padding_size = header->padding_size;

	new_free_header = setupsffh((sf_free_header *) header, 0, old_block_size, 0);

	footer->alloc = 0;

	curr_flh = freelist_head;

	if ((prev && (void *) header != brk) && (next && (void *) next_header != brk)) {
		/* Both blocks left and right are allocated, t-t */

		sf_free_header *next_next, *prev_next;

		next_next = next_header->next;
		// prev_next = ((sf_free_header *)((void *) header - BLOCKSIZE(prev_footer)))->next;

		new_free_header = coalesce_right(next_header, new_free_header);
		footer = FOOTERADDR(new_free_header, FLH_BLOCKSIZE(new_free_header));
		prev_footer_header = coalesce_left(prev_footer, new_free_header, footer);

		prev_next = prev_footer_header->next;

		// Cyclic referencing is still possible if the next and prev reference each other after coalescing becareful
		if (next_next == prev_footer_header)
			next_next = next_next->next;

		freelist_head = prev_footer_header;
		freelist_head->prev = NULL;

		if ((void *) next_header == (void *) curr_flh) {
			freelist_head->next = next_next;

			if (next_next != NULL)
				next_next->prev = freelist_head;

		} else if ((void *) prev_footer_header == (void *) curr_flh) {
			freelist_head->next = prev_next;

			if (prev_next != NULL)
				prev_next->prev = freelist_head;
		} else {
			freelist_head->next = curr_flh;

			if (curr_flh != NULL)
				curr_flh->prev = freelist_head;
		}

		// Add Coalesce instance here
		coalesces++;

	} else if (next && (void *) next_header != brk) {
		new_free_header = coalesce_right(next_header, new_free_header);

		freelist_head = new_free_header;
		freelist_head->prev = NULL;

		if (next_header == curr_flh) {
			freelist_head->next = curr_flh->next;

			if (curr_flh->next != NULL)
				curr_flh->next->prev = freelist_head;

		} else {
			freelist_head->next = curr_flh;
			curr_flh->prev = freelist_head;
		}

		// Add Coalesce instance here
		coalesces++;

	} else if (prev && (void *) header != heap_start) {
		prev_footer_header = coalesce_left(prev_footer, new_free_header, footer);

		freelist_head = prev_footer_header;
		freelist_head->prev = NULL;

		if (freelist_head == curr_flh) {
			freelist_head->next = curr_flh->next;

			if (curr_flh->next != NULL)
				curr_flh->next->prev = freelist_head;

		} else {
			freelist_head->next = curr_flh;
			curr_flh->prev = freelist_head;
		}

		// Add Coalesce instance here
		coalesces++;

	} else {
		freelist_head = (sf_free_header *) coalesce_middle(new_free_header);

		if (curr_flh != NULL && freelist_head != curr_flh) {
			freelist_head->next = curr_flh;
			curr_flh->prev = freelist_head;
		}
	}

	// Add free instance here
	external = nfreeblocks();
	internal -= (old_padding_size + HFSIZE);
	frees++;
}

void *sf_realloc(void *ptr, size_t size) 
{
	  /*
	   *
	   * REFACTOR THIS GARBAGE LATER
	   */
	if (ptr == NULL)
		return NULL;

	errno = 0;
	if (heap_start == NULL || ptr - SF_HEADER_SIZE < heap_start || 
            ptr - SF_HEADER_SIZE > sf_sbrk(0) - MINALLOC || size <= 0 || realspace(size) > MAXALLOC) {
		errno = EINVAL;
		return NULL;
	}

	sf_free_header *next_block;
	sf_header *curr_block;
	sf_footer *footer;

	size_t act_size, diff;
	uint64_t prev_padding_size;

	curr_block = (sf_header *)(ptr - SF_HEADER_SIZE);
	footer = FOOTERADDR(curr_block, BLOCKSIZE(curr_block));

	errno = 0;
	if (curr_block->alloc == 0 || footer->alloc == 0 || BLOCKSIZE(curr_block) != BLOCKSIZE(footer)) {
		errno = EINVAL;
		return NULL;
	}

	act_size = realspace(size);

	/* Physically adjacent next block */
	next_block = (sf_free_header *)((void *) footer + SF_FOOTER_SIZE);

	prev_padding_size = curr_block->padding_size;
	internal -= (prev_padding_size + HFSIZE);

	if (BLOCKSIZE(curr_block) > act_size) {
		sf_footer *new_footer;
		/** Resize down curr_block, avoid splinters and free leftovers **/
		diff = BLOCKSIZE(curr_block) - act_size;

		curr_block->padding_size = act_size - size - HFSIZE;
		/** Difference is big enough for an actual free block **/
		if (diff >= MINALLOC) {
			sf_header *leftovers;

			curr_block->block_size = (act_size) >> 4;

			new_footer = setupf(FOOTERADDR(curr_block, BLOCKSIZE(curr_block)), 1, BLOCKSIZE(curr_block) >> 4);

			/* Set leftovers as a 'allocated' block then call free on it */
			leftovers = (sf_header *)((void *) new_footer + SF_FOOTER_SIZE);
			leftovers = setupsfh(leftovers, 1, (diff >> 4), 0);

			// leftovers_footer = (sf_footer *)((void *) leftovers + BLOCKSIZE(leftovers) - SF_FOOTER_SIZE);
			setupf(FOOTERADDR(leftovers, BLOCKSIZE(leftovers)), 1, BLOCKSIZE(leftovers) >> 4);
			// leftovers_footer = setupf(leftovers_footer, 1, BLOCKSIZE(leftovers) >> 4);

			sf_free((void *) PAYLOAD(leftovers));
			// we had extra header/footer so add it back in
			internal += HFSIZE;

		} else if ((void *) next_block != sf_sbrk(0) && !next_block->header.alloc) {
			// Check right block if its free, if it is coalesce with the splinter space and add to freelist
			// coalesce++ here too
			sf_free_header *next, *prev, *current;

			curr_block->block_size = (act_size >> 4);
			new_footer = setupf(FOOTERADDR(curr_block, BLOCKSIZE(curr_block)), 1, BLOCKSIZE(curr_block) >> 4);

			current = (sf_free_header *)((void *) new_footer + SF_FOOTER_SIZE);
			setupsffh(current, 0, (diff + FLH_BLOCKSIZE(next_block)) >> 4 ,0);
			setupf(FOOTERADDR(current, FLH_BLOCKSIZE(current)), 0, FLH_BLOCKSIZE(current) >> 4);
			next = next_block->next;
			prev = next_block->prev;

			if (next_block == freelist_head) {
				current->next = next;
				current->prev = prev;

				freelist_head = current;
			} else {
				set_prev_next(next, prev);

				sf_free_header *prev_flh = freelist_head;

				freelist_head = current;
				freelist_head->prev = NULL;
				freelist_head->next = prev_flh;

				prev_flh->prev = freelist_head;
			}

			frees++;
			coalesces++;
		}

		internal += curr_block->padding_size + HFSIZE;

	} else if (BLOCKSIZE(curr_block) < act_size) {
		sf_header *space;

		footer = (sf_footer *)((void *) curr_block + BLOCKSIZE(curr_block) - SF_FOOTER_SIZE);

		/* Check to see if this block is last block before end of heap */
		if ((void *) next_block != sf_sbrk(0)   && 
			     !next_block->header.alloc  &&
			     FLH_BLOCKSIZE(next_block) + BLOCKSIZE(curr_block) >= act_size) {

			sf_free_header *next, *prev;

			next = next_block->next;
			prev = next_block->prev;

			curr_block->padding_size = act_size - size - HFSIZE;

			/* do stuff for fitting here */

			/* Check if next block is allocated */
			size_t total_size = FLH_BLOCKSIZE(next_block) + BLOCKSIZE(curr_block);
			diff = total_size - act_size;

			/* there are leftovers */
			if (diff >= MINALLOC) {
				sf_free_header *prev_flh, *leftovers;

				/* Free the right block first */

				curr_block->block_size = (act_size >> 4);

				// footer = (sf_footer *)((void *) curr_block + BLOCKSIZE(curr_block) - SF_FOOTER_SIZE);
				footer = setupf(FOOTERADDR(curr_block, BLOCKSIZE(curr_block)), 1, BLOCKSIZE(curr_block) >> 4);

				leftovers = (sf_free_header *)((void *) footer + SF_FOOTER_SIZE);
				leftovers = setupsffh(leftovers, 0, (diff >> 4), 0);

				setupf(FOOTERADDR(leftovers, FLH_BLOCKSIZE(leftovers)), 0, FLH_BLOCKSIZE(leftovers) >> 4);

				prev_flh = freelist_head;

				set_prev_next(next, prev);

				freelist_head = leftovers;
				freelist_head->next = freelist_head->prev = NULL;

				if ((void *) next_block != prev_flh) {
					freelist_head->next = prev_flh;
					prev_flh->prev = freelist_head;
				} else {
					freelist_head->next = next;

					if (next != NULL)
						next->prev = freelist_head;
				}

			} else {
				curr_block->block_size = (total_size >> 4);

				footer = (sf_footer *)((void *) curr_block + BLOCKSIZE(curr_block) - SF_FOOTER_SIZE);
				footer = setupf(footer, 1, BLOCKSIZE(curr_block) >> 4);

				set_prev_next(next, prev);

				if ((void *) next_block == (void *) freelist_head) {
					freelist_head = freelist_head->next;

					if (freelist_head)
						freelist_head->prev = NULL;
				}
			}

			// Add Coalesce instance here
			internal += curr_block->padding_size + HFSIZE;
			coalesces++;
			allocations++;
			
		} else {
			/* may or may not be free block to the right, we dont care, just find a new block in the freelist head */
			/* check freelist for freeblock */
			uint64_t copysize = BLOCKSIZE(curr_block) - curr_block->padding_size - HFSIZE;
			unsigned char temp[copysize];
			memmove((void *) temp, (void *) PAYLOAD(curr_block), copysize);

			sf_free(PAYLOAD(curr_block));

			space = sf_malloc(size) - SF_HEADER_SIZE;
			sf_blockprint(space);

			errno = 0;
			if (space == NULL) {
				errno = ENOMEM;
				return NULL;
			}

			memmove((void *) PAYLOAD(space), (void *) temp, copysize);

			// put back padding shit from sf_Free
			internal += prev_padding_size + HFSIZE;
			curr_block = space;
		}
 
	} else {
		// internal -= curr_block->padding_size;
		curr_block->padding_size = BLOCKSIZE(curr_block) - HFSIZE - size;
		internal += curr_block->padding_size + HFSIZE;
	}

	// Add allocation instance here
	external = nfreeblocks();
	return PAYLOAD(curr_block);
}

int sf_info(info* meminfo) 
{
	if (!meminfo)
		return -1;

	meminfo->internal = internal;
	meminfo->external = external;
	meminfo->allocations = allocations;
	meminfo->frees = frees;
	meminfo->coalesce = coalesces;

  	return 0;
}
