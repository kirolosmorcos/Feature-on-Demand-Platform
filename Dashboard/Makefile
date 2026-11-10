CC=qcc -Vgcc_ntoaarch64le

LVGL_ROOT=/home/midgeek/Documents/ejad/lvgl-9.4.0
CCFLAGS=-I$(LVGL_ROOT)/env_support/qnx -I$(LVGL_ROOT)
LDFLAGS=-lscreen -lm -llvgl -lsocket -L$(LVGL_ROOT)/env_support/qnx/aarch64/so.le

lvgl_example: lvgl_example.c
	    $(CC) $(CCFLAGS) -Wall -o $@ $< $(LDFLAGS)

clean:
	    rm -f *.o *~ lvgl_example
