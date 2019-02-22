################################################################################################
# ºôµå
################################################################################################
NASM = nasm
TEMP =C:/cygwin64/usr/DEWH/cross/bin
GCC32 = $(TEMP)/x86_64-elf-gcc.exe -c -m32 -ffreestanding
LD32 = $(TEMP)/x86_64-elf-ld.exe -melf_i386 -T ../elf_i386.x -nostdlib -e Main -Ttext 0x10200
OBJCOPY32 = $(TEMP)/x86_64-elf-objcopy -j .text -j .data -j .rodata -j .bss -S -O binary

OBJECTDIRECTORY = Temp
SOURCEDIRECTORY = Source

################################################################################################
# ºôµå
################################################################################################

all: prepare Kernel32.bin

prepare:
	mkdir -p $(OBJECTDIRECTORY)
	
Kernel32.bin: $(OBJECTDIRECTORY)/EntryPoint.bin $(OBJECTDIRECTORY)/Kernel32.elf.bin
	cat $^ > $@
	
$(OBJECTDIRECTORY)/EntryPoint.bin: $(SOURCEDIRECTORY)/EntryPoint.s
	$(NASM) -o $@ $<
	
$(OBJECTDIRECTORY)/Kernel32.elf.bin: ExecuteInternalBuild
	$(OBJCOPY32) $(OBJECTDIRECTORY)/Kernel32.elf $@

ExecuteInternalBuild: dep
	make -C $(OBJECTDIRECTORY) -f ../makefile Kernel32.elf
dep:
	@echo === Make Dependacy File ===
	make -C $(OBJECTDIRECTORY) -f ../makefile InternalDependency
	@echo === Dependacy Search Complete ===
clean:
	rm -f *.bin
	rm -f $(OBJECTDIRECTORY)/*.*
################################################################################################
# ºôµå
################################################################################################
CENTRYPOINTOBJECTFILE = Main.o
CSOURCEFILES = $(wildcard ../$(SOURCEDIRECTORY)/*.c)
ASSEMBLYSOURCEFILES = $(wildcard ../$(SOURCEDIRECTORY)/*.asm)
COBJECTFILES= $(subst Main.o, , $(notdir $(patsubst %.c, %.o, $(CSOURCEFILES))))
ASSEMBLYOBJECTFILES= $(notdir $(patsubst %.asm, %.o, $(ASSEMBLYSOURCEFILES)))

%.o: ../$(SOURCEDIRECTORY)/%.c
	$(GCC32) $< 
%.o: ../$(SOURCEDIRECTORY)/%.asm
	$(NASM) -f elf32 -o $@ $<

InternalDependency:
	$(GCC32) -MM $(CSOURCEFILES) > Dependency.dep
	
Kernel32.elf: $(CENTRYPOINTOBJECTFILE) $(COBJECTFILES) $(ASSEMBLYOBJECTFILES)
	@echo ==$(CENTRYPOINTOBJECTFILE) $(COBJECTFILES) $(ASSEMBLYOBJECTFILES)==
	$(LD32) -o $@ $^
ifeq (Dependency.dep, $(wildcard Dependency.dep))
include Dependency.dep
endif