-include nemu/Makefile.git

defalut:
	@echo "Please run 'make' under any subprojects to compile."
 
clean:
	-$(MAKE) -C nemu clean
	-$(MAKE) -C nexus-am clean
	-$(MAKE) -C nanos-lite clean
	-$(MAKE) -C navy-apps clean

submit: clean
	git gc
	cd .. && tar cj $(shell basename `pwd`) > $(STU_ID).tar.bz2



count:
	find nemu/ -name "*[.h|.c]" |xargs cat|wc -l


Counts:
	find  nemu/ -name "*[.h|.c]" |xargs cat|grep -v ^/$|wc -l 
	
.PHONY: default clean submit count Counts
