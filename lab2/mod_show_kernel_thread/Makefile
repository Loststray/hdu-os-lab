obj-m += mod_show_kernel_thread.o

PWD := $(CURDIR)

all: 
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 
clean: 
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean