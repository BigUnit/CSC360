VDISK = ../disk
IO = ../io
TARGETS = app test01 test02 test03 test04 test05 test06
OBJS = $(VDISK)/vdiskAPI.o $(IO)/File.o
JUNKF = $(OBJS) *~
JUNKD = *.dSYM
CFLAGS  += -g -std=gnu11 -I$(VDISK)
all: $(TARGETS)
$(TARGETS): $(OBJS)
tidy:
	rm -f $(JUNKF); rm -rf $(JUNKD)
clean:
	rm -f $(JUNKF) $(TARGETS); rm -rf $(JUNKD)