#ifndef FILESYS_H
#define FILESYS_H

// flash buffer 0 is for read/write caching

// init after power-on (also after a power failure)
void filesys_init();

// format C: :)
void filesys_reset();

// write event block stored at address addr
// use info in the block to determine its CP and number, as well as its length
// return the address of directory entry for the block
// !!! do nothing if we already have this block! (still return dir entry addr)
uint32_t filesys_write_block(uint8_t *addr);

// find the address of dir entry for this block
// dir entry is uint32_t, high byte is transmission count, low 3 bytes
// is address
uint32_t filesys_get_dir_entry(uint16_t cp, uint16_t num);

// set cur_mem_end: can be used to grow the last added block
// (use only if there were no filesys operations since filesys_write_block
// with a *new* block)
void filesys_set_cur_mem_end(uint32_t new_end);

#endif /* FILESYS_H */
