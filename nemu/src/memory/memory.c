#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

paddr_t page_translate(vaddr_t vaddr,bool write){

    PDE pde;
    PTE pte;
    
    Log("vaddr: %#x",vaddr); 
   
    uint32_t DIR = (vaddr >> 22);  // 取虚拟地址中隐含的页目录项
    Log("DIR: %#x",DIR);
    uint32_t PDE_addr=(cpu.cr3.page_directory_base << 12) + (DIR << 2);  // 计算页目录项地址
    Log("CR3: %#x",cpu.cr3.page_directory_base);  
    pde.val=paddr_read(PDE_addr,4);  // 取页表项基地址
    Log("PDE_addr: %#x    pde.val: %#x",PDE_addr,pde.val);
    assert(pde.present);  
    
    uint32_t PAGE=((vaddr >> 12) & 0x3ff); // 取虚拟地址的中间十位，即页表项内偏移地址；
    uint32_t PTE_addr=(pde.val & 0xfffff000)+(PAGE << 2); //取从页目录里获取的页表项基址的高二十位与虚拟地址的中间十位*4结合
    pte.val=paddr_read(PTE_addr,4);
    Log("PTE_addr: %#x    pte.val: %#x",PTE_addr,pte.val);
    assert(pte.present);
    
    uint32_t physical_addr=(pte.val & 0xfffff000)+(vaddr & 0xfff); // 物理地址就是pte的高二十位和虚拟地址的低十二位的结合；
    Log("physical_addr: %#x",physical_addr);
    pde.accessed=1;
    paddr_write(PDE_addr,4,pde.val); //回写页目录项

    if(pte.accessed==0 || (pte.dirty==0 && write)){
      pte.accessed=1;
      pte.dirty=1;
    } 
    paddr_write(PTE_addr,4,pte.val); //回写页表项
   
    return physical_addr;
}

uint32_t paddr_read(paddr_t addr, int len) {
  int mmio_n;
  if ((mmio_n = is_mmio(addr)) != -1)   //物理地址被映射到 I/O 空间
    return mmio_read(addr, len, mmio_n);
  else
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int mmio_n;
  if ((mmio_n = is_mmio(addr)) != -1)  //物理地址被映射到 I/O 空间
    mmio_write(addr, len, data, mmio_n);
  else
    memcpy(guest_to_host(addr), &data, len);
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  if(cpu.cr0.paging) {
        if (((addr<<20)>>20)+len>0x1000) {  // 页内偏移量+len>4KB=2^12=0x1000
            /* this is a special case, you can handle it later. */
            assert(0);
        }
        else {
            paddr_t paddr = page_translate(addr,false);
            return paddr_read(paddr, len);
        }
    }
  else
      return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if(cpu.cr0.paging) {
    if (((addr<<20)>>20)+len>0x1000) {  // 页内偏移量+len>4KB=2^12=0x1000
        /* this is a special case, you can handle it later. */
        assert(0);
    }
    else {
        paddr_t paddr = page_translate(addr,true);
        paddr_write(paddr, len, data);
    }
  }
  else
    paddr_write(addr, len, data);
}
