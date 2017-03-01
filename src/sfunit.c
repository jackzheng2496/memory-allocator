#include <criterion/criterion.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sfmm.h"

/**
 *  HERE ARE OUR TEST CASES NOT ALL SHOULD BE GIVEN STUDENTS
 *  REMINDER MAX ALLOCATIONS MAY NOT EXCEED 4 * 4096 or 16384 or 128KB
 */

Test(sf_memsuite, Coalesce_Free_block_before) {
  int *x = sf_malloc(4);
  int *y = sf_malloc(4);
  int *z = sf_malloc(4);
  (void)z;

  sf_free(x);
  sf_free(y);

  sf_header *sfHeader = (sf_header *)(x - 8);
  cr_assert(sfHeader->block_size == 64);

  sf_free(z);
}

Test(sf_memsuite, Coalesce_Free_block_after) {
  int *x = sf_malloc(4);
  int *y = sf_malloc(4);
  int *z = sf_malloc(4);
  (void)z;

  sf_free(y);
  sf_free(x);

  sf_header *sfHeader = (sf_header *)(x-8);
  cr_assert(sfHeader->block_size == 64);

  sf_free(z);

}

Test(sf_memsuite, Coalesce_free_before_and_after) {
  int *x = sf_malloc(4);
  int *y = sf_malloc(4);
  int *z = sf_malloc(4);
  int *a = sf_malloc(4);
  (void)a;

  sf_free(x);
  sf_free(z);
  sf_free(y);

  sf_header *sfHeader = (sf_header *)(x-8);
  cr_assert(sfHeader->block_size == 96);

  sf_free(a);
}
// Test(sf_memsuite, Malloc_an_Integer, .init = sf_mem_init, .fini = sf_mem_fini) {
//     int *x = sf_malloc(sizeof(int));
//     *x = 4;
//     cr_assert(*x == 4, "Failed to properly sf_malloc space for an integer!");
// }

// Test(sf_memsuite, Free_block_check_header_footer_values, .init = sf_mem_init, .fini = sf_mem_fini) {
//     void *pointer = sf_malloc(sizeof(short));
//     sf_free(pointer);
//     pointer = pointer - 8;
//     sf_header *sfHeader = (sf_header *) pointer;
//     cr_assert(sfHeader->alloc == 0, "Alloc bit in header is not 0!\n");
//     sf_footer *sfFooter = (sf_footer *) (pointer - 8 + (sfHeader->block_size << 4));
//     cr_assert(sfFooter->alloc == 0, "Alloc bit in the footer is not 0!\n");
// }

// Test(sf_memsuite, PaddingSize_Check_char, .init = sf_mem_init, .fini = sf_mem_fini) {
//     void *pointer = sf_malloc(sizeof(char));
//     pointer = pointer - 8;
//     sf_header *sfHeader = (sf_header *) pointer;
//     cr_assert(sfHeader->padding_size == 15, "Header padding size is incorrect for malloc of a single char!\n");
// }

// Test(sf_memsuite, Check_next_prev_pointers_of_free_block_at_head_of_list, .init = sf_mem_init, .fini = sf_mem_fini) {
//     int *x = sf_malloc(4);
//     memset(x, 0, 4);
//     cr_assert(freelist_head->next == NULL);
//     cr_assert(freelist_head->prev == NULL);
// }

// Test(sf_memsuite, Coalesce_no_coalescing, .init = sf_mem_init, .fini = sf_mem_fini) {
//     void *x = sf_malloc(4);
//     void *y = sf_malloc(4);
//     memset(y, 0xFF, 4);
//     sf_free(x);
//     cr_assert(freelist_head == x-8);
//     sf_free_header *headofx = (sf_free_header*) (x-8);
//     sf_footer *footofx = (sf_footer*) (x - 8 + (headofx->header.block_size << 4)) - 8;

//     sf_blockprint((sf_free_header*)((void*)x-8));
//     // All of the below should be true if there was no coalescing
//     cr_assert(headofx->header.alloc == 0);
//     cr_assert(headofx->header.block_size << 4 == 32);
//     cr_assert(headofx->header.padding_size == 0, "%d\n", headofx->header.padding_size);

//     cr_assert(footofx->alloc == 0);
//     cr_assert(footofx->block_size << 4 == 32);
// }

// /*
// //############################################
// // STUDENT UNIT TESTS SHOULD BE WRITTEN BELOW
// // DO NOT DELETE THESE COMMENTS
// //############################################
// */
// Test(sf_memsuite, Coalesce_coalesce_right, .init = sf_mem_init, .fini = sf_mem_fini) {
//     void *x = sf_malloc(4);
//     void *y = sf_malloc(4);
//     memset(x, 0xFF, 4);

//     sf_free(y);
//     cr_assert(freelist_head == y-8);

//     sf_free_header *headofy = (sf_free_header *) (y-8);
//     sf_blockprint((sf_free_header*)((void*)y-8));

//     sf_footer *footofy = (sf_footer *) (y - 8 + (headofy->header.block_size << 4) - 8);

//     cr_assert(headofy->header.alloc == 0);
//     cr_assert(headofy->header.block_size << 4 == 4064);
//     cr_assert(headofy->header.padding_size == 0);

//     cr_assert(footofy->alloc == 0);
//     cr_assert(footofy->block_size << 4 == 4064, "%p %d\n", footofy, headofy->header.block_size << 4);
// }

// Test(sf_memsuite, Realloc_realloc_smaller_splinter, .init = sf_mem_init, .fini = sf_mem_fini) {
//     void *x = sf_malloc(32);
//     size_t sizeofx = ((sf_header*)(x-8))->block_size << 4;
//     void *y = sf_malloc(4);
//     memset(y, 0xFF, 4);

//     x = sf_realloc(x, 10);
//     cr_assert(freelist_head == y+24);

//     sf_header *headofx = (sf_header *) (x-8);
//     sf_varprint(x);

//     sf_footer *footofx = (sf_footer *) (x - 8 + (headofx->block_size << 4) - 8);

//     cr_assert(headofx->alloc == 1);
//     cr_assert(headofx->block_size << 4 == sizeofx);
//     cr_assert(headofx->padding_size == 6);

//     cr_assert(footofx->alloc == 1);
//     cr_assert(footofx->block_size << 4 == sizeofx);
// }

// Test(sf_memsuite, Coalesce_coalesce_middle, .init = sf_mem_init, .fini = sf_mem_fini) {
//     void *x = sf_malloc(4);
//     void *y = sf_malloc(4);
//     void *z = sf_malloc(4);

//     sf_free(z);
//     sf_free(x);
//     sf_free(y);

//     cr_assert(freelist_head == x-8);

//     sf_free_header *headofx = (sf_free_header *) (x-8);
//     sf_blockprint((sf_free_header*)(x-8));

//     sf_footer *footofx = (sf_footer *) (x - 8 + (headofx->header.block_size << 4) - 8);

//     cr_assert(headofx->header.alloc == 0);
//     cr_assert(headofx->header.block_size << 4 == 4096);
//     cr_assert(headofx->header.padding_size == 0);

//     cr_assert(footofx->alloc == 0);
//     cr_assert(footofx->block_size << 4 == 4096);
// }

// // EXTRA CREDIT STUFF

// Test(sf_memsuite, Coalesce_coalesce_left, .init = sf_mem_init, .fini = sf_mem_fini) {
//     void *x = sf_malloc(4);
//     void *y = sf_malloc(4);
//     void *z = sf_malloc(4);
//     memset(z, 0xFF, 4);

//     sf_free(x);
//     sf_free(y);

//     cr_assert(freelist_head == x-8);

//     sf_free_header *headofx = (sf_free_header *) (x-8);
//     sf_blockprint((sf_free_header*)(x-8));

//     sf_footer *footofx = (sf_footer *) (x - 8 + (headofx->header.block_size << 4) - 8);

//     cr_assert(headofx->header.alloc == 0);
//     cr_assert(headofx->header.block_size << 4 == 64);
//     cr_assert(headofx->header.padding_size == 0);

//     cr_assert(footofx->alloc == 0);
//     cr_assert(footofx->block_size << 4 == 64);
// }

// Test(sf_memsuite, Free_address_invalid, .init = sf_mem_init, .fini = sf_mem_fini) {
//     void *x = sf_malloc(4);
//     void *y = sf_malloc(4);
//     memset(y, 0xFF, 4);

//     sf_free(x+16);
//     sf_free(y);

//     cr_assert(freelist_head == y-8);

//     sf_header *headofx = (sf_header *) (x-8);
//     sf_blockprint((sf_header*)(x-8));

//     sf_footer *footofx = (sf_footer *) (x - 8 + (headofx->block_size << 4) - 8);

//     cr_assert(headofx->alloc == 1);
//     cr_assert(headofx->block_size << 4 == 32);
//     cr_assert(headofx->padding_size == 12);

//     cr_assert(footofx->alloc == 1);
//     cr_assert(footofx->block_size << 4 == 32);
// }

// Test(sf_memsuite, Realloc_address_invalid, .init = sf_mem_init, .fini = sf_mem_fini) {
//     void *x = sf_malloc(4);
//     void *y = sf_realloc(x+16, 64);

//     cr_assert(y == NULL);
//     cr_assert(freelist_head == x+24);

//     sf_header *headofx = (sf_header *) (x-8);
//     sf_blockprint((sf_header*)(x-8));

//     sf_footer *footofx = (sf_footer *) (x - 8 + (headofx->block_size << 4) - 8);

//     cr_assert(headofx->alloc == 1);
//     cr_assert(headofx->block_size << 4 == 32);
//     cr_assert(headofx->padding_size == 12);

//     cr_assert(footofx->alloc == 1);
//     cr_assert(footofx->block_size << 4 == 32);
// }

// Test(sf_memsuite, Realloc_smaller_leftovers, .init = sf_mem_init, .fini = sf_mem_fini) {
//     void *x = sf_malloc(64);
//     x = sf_realloc(x, 16);

//     cr_assert(x != NULL);
//     cr_assert(freelist_head == x+24);

//     sf_header *headofx = (sf_header *) (x-8);
//     sf_blockprint((sf_header*)(x-8));

//     sf_footer *footofx = (sf_footer *) (x - 8 + (headofx->block_size << 4) - 8);

//     cr_assert(headofx->alloc == 1);
//     cr_assert(headofx->block_size << 4 == 32);
//     cr_assert(headofx->padding_size == 0);

//     cr_assert(footofx->alloc == 1);
//     cr_assert(footofx->block_size << 4 == 32);

//     sf_free_header *freelist = (sf_free_header *) (x + 24);
//     sf_footer *freelist_footer = (sf_footer *) ((void *) freelist + (freelist_head->header.block_size << 4) - 8);

//     cr_assert(freelist->header.alloc == 0);
//     cr_assert(freelist->header.block_size << 4 == 4064);
//     cr_assert(freelist->header.padding_size == 0);

//     cr_assert(freelist_footer->alloc == 0);
//     cr_assert(freelist_footer->block_size << 4 == 4064);
// }

// Test(sf_memsuite, Realloc_rightblock_allocated, .init = sf_mem_init, .fini = sf_mem_fini) {
//     void *x = sf_malloc(4);
//     void *y = sf_malloc(4);
//     memset(y, 0xFF, 4);

//     void *x2 = sf_realloc(x, 50);

//     cr_assert(x != NULL);
//     cr_assert(freelist_head == x2+72);
//     cr_assert(freelist_head->header.block_size << 4 == 3952);

//     sf_header *headofx2 = (sf_header *) (x2-8);
//     sf_blockprint((sf_header*)(x2-8));

//     sf_footer *footofx2 = (sf_footer *) (x2 - 8 + (headofx2->block_size << 4) - 8);

//     cr_assert(headofx2->alloc == 1);
//     cr_assert(headofx2->block_size << 4 == 80);
//     cr_assert(headofx2->padding_size == 14);

//     cr_assert(footofx2->alloc == 1);
//     cr_assert(footofx2->block_size << 4 == 80);

// }

// Test(sf_memsuite, Realloc_rightblock_free_nospace, .init = sf_mem_init, .fini = sf_mem_fini) {
//     void *x = sf_malloc(4);
//     void *y = sf_malloc(4);
//     void *z = sf_malloc(4);

//     sf_free(y);

//     cr_assert(freelist_head == y-8);

//     x = sf_realloc(x, 64);
//     cr_assert(x != NULL);
//     cr_assert(x == z+32);

//     sf_header *headofx = (sf_header *) (z+24);
//     sf_blockprint((sf_header*)(x-8));

//     sf_footer *footofx = (sf_footer *) (x - 8 + (headofx->block_size << 4) - 8);

//     cr_assert(headofx->alloc == 1);
//     cr_assert(headofx->block_size << 4 == 80);
//     cr_assert(headofx->padding_size == 0);

//     cr_assert(footofx->alloc == 1);
//     cr_assert(footofx->block_size << 4 == 80);
// }

// Test(sf_memsuite, Realloc_rightblock_free_splinter, .init = sf_mem_init, .fini = sf_mem_fini) {
//     void *x = sf_malloc(4);
//     void *y = sf_malloc(4);
//     void *z = sf_malloc(4);

//     sf_free(y);

//     cr_assert(freelist_head == y-8);

//     x = sf_realloc(x, 8 * sizeof(int));
//     cr_assert(x != NULL);
//     cr_assert(freelist_head == z+24);

//     sf_header *headofx = (sf_header*) (x-8);
//     sf_blockprint(headofx);

//     sf_footer *footofx = (sf_footer *) (x - 8 + (headofx->block_size << 4) - 8);

//     cr_assert(headofx->alloc == 1);
//     cr_assert(headofx->block_size << 4 == 64);
//     cr_assert(headofx->padding_size == 0);

//     cr_assert(footofx->alloc == 1);
//     cr_assert(footofx->block_size << 4 == 64);
// }

// Test(sf_memsuite, Realloc_rightblock_free_leftovers, .init = sf_mem_init, .fini = sf_mem_fini) {
//     void *x = sf_malloc(4);
//     void *y = sf_malloc(12 * sizeof(int));
//     void *z = sf_malloc(4);
//     memset(z, 0xFF, 4);

//     sf_free(y);
//     sf_snapshot(true);
//     sf_blockprint(y-8);

//     cr_assert(freelist_head == y-8);

//     x = sf_realloc(x, 8 * sizeof(int));
//     cr_assert(x != NULL);

//     sf_header *headofx = (sf_header*) (x-8);
//     sf_blockprint(headofx);

//     sf_footer *footofx = (sf_footer *) (x - 8 + (headofx->block_size << 4) - 8);

//     cr_assert(headofx->alloc == 1);
//     cr_assert(headofx->block_size << 4 == 48);
//     cr_assert(headofx->padding_size == 0);

//     cr_assert(footofx->alloc == 1);
//     cr_assert(footofx->block_size << 4 == 48);
// }

// Test(sf_memsuite, Coalesce_coalesce_middle_prevhead, .init = sf_mem_init, .fini = sf_mem_fini) {
//     void *x = sf_malloc(4);
//     void *y = sf_malloc(4);
//     void *z = sf_malloc(4);

//     sf_free(z);
//     cr_assert(freelist_head == z-8);
//     cr_assert(freelist_head->header.block_size << 4 == 4032);

//     sf_free(x);
//     cr_assert(freelist_head == x-8);
//     cr_assert(freelist_head->header.block_size << 4 == 32);

//     sf_free(y);
//     cr_assert(freelist_head == x-8);

//     sf_free_header *headofx = (sf_free_header *) (x-8);
//     sf_blockprint(headofx);

//     sf_footer *footofx = (sf_footer *) (x - 8 + (headofx->header.block_size << 4) - 8);

//     cr_assert(headofx->header.alloc == 0);
//     cr_assert(headofx->header.block_size << 4 == 4096);
//     cr_assert(headofx->header.padding_size == 0);
//     cr_assert(headofx->next == NULL);
//     cr_assert(headofx->prev == NULL);

//     cr_assert(footofx->alloc == 0);
//     cr_assert(footofx->block_size << 4 == 4096);
// }

// Test(sf_memsuite, Coalesce_coalesce_middle_nexthead, .init = sf_mem_init, .fini = sf_mem_fini) {
//     void *x = sf_malloc(4);
//     void *y = sf_malloc(4);
//     void *z = sf_malloc(4);

//     sf_free(x);
//     cr_assert(freelist_head == x-8);
//     cr_assert(freelist_head->header.block_size << 4 == 32);

//     sf_free(z);
//     cr_assert(freelist_head == z-8);
//     cr_assert(freelist_head->header.block_size << 4 == 4032);

//     sf_free(y);
//     cr_assert(freelist_head == x-8);

//     sf_free_header *headofx = (sf_free_header *) (x-8);
//     sf_blockprint(headofx);

//     sf_footer *footofx = (sf_footer *) (x - 8 + (headofx->header.block_size << 4) - 8);

//     cr_assert(headofx->header.alloc == 0);
//     cr_assert(headofx->header.block_size << 4 == 4096);
//     cr_assert(headofx->header.padding_size == 0);
//     cr_assert(headofx->next == NULL);
//     cr_assert(headofx->prev == NULL);

//     cr_assert(footofx->alloc == 0);
//     cr_assert(footofx->block_size << 4 == 4096);
// }