#include "common.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();
void* new_page(void);

uintptr_t loader(_Protect *as, const char *filename) {
 // TODO();
// ramdisk_read(DEFAULT_ENTRY,0,get_ramdisk_size());
  int fd = fs_open(filename,0,0);
  size_t lens = fs_filesz(fd);
  void *va=DEFAULT_ENTRY;  //起始地址是入口地址
  void *pa;
  int page_num=lens/PGSIZE +1;  // 页数=文件大小/页面大小 
  for(int i=0;i<page_num;i++){
    pa=new_page(); // 申请空闲页面
    Log("Map va to pa: 0x%08x to 0x%08x", va, pa);
    _map(as,va,pa); //
    fs_read(fd,pa,PGSIZE); // 读一页内容，写到这个物理页上
    va +=PGSIZE;

  }
  fs_close(fd);
  
  return (uintptr_t)DEFAULT_ENTRY;
}
