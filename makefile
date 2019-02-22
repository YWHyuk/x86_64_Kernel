all: BootLoader Kernel32 Kernel64 Disk.img

BootLoader:
	@echo
	@echo ============================ Build Boot Loader ====================================
	@echo 
	
	make -C 00.BootLoader
	
	@echo
	@echo ============================ Build Complete =======================================
	@echo
	
Kernel32:
	@echo
	@echo ============================ Build Kernel32 ====================================
	@echo 
	
	make -C 01.Kernel32
	
	@echo
	@echo ============================ Build Complete =======================================
	@echo
	
Kernel64:
	@echo
	@echo ============================ Build Kernel32 ====================================
	@echo 
	
	make -C 02.Kernel64
	
	@echo
	@echo ============================ Build Complete =======================================
	@echo


Disk.img: 00.BootLoader/BootLoader.bin 01.Kernel32/Kernel32.bin 02.Kernel64/Kernel64.bin
	@echo
	@echo ============================ Disk Image Build Start ===============================
	@echo
	./ImageMaker.exe $^
	
	@echo
	@echo ============================ All Build Complete ===================================
	@echo
	
	@echo ============================== Sending to VM =======================================
	scp -P 2222 Disk.img 192.168.56.1:/home/dewh/disk_dir/disk.img
	@echo ============================== Sending to VM =======================================
	
	
clean:
	make -C 00.BootLoader clean
	make -C 01.Kernel32 clean
	make -C 02.Kernel64 clean
	rm -f Disk.img