#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <inttypes.h>

void pq_init();
void pq_reset();

// add a directory entry at address dir
void pq_add(uint32_t dir);

// what's at the top? (does not change anything)
uint32_t pq_peek_top();

// visiting the top of the queue;
// you must not call pq_add while visiting!
// 
// pq_vis_start();
// while (...) {
//   t = pq_peek_top();
//   if (is_ok(t)) {
//      (modify block(t) if needed)
//      pq_vis_next();  // top is temporary removed from queue
//   }
// }
// pq_vis_end();   // all visited entries are re-added to queue
void pq_vis_start();
void pq_vis_next();
void pq_vis_end();

uint32_t pq_len();


#endif /* PRIORITY_QUEUE_H */
