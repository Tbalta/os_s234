GCCPARAM = -m32 -g -std=c99 -fno-stack-protector
ASMPARAM = -f elf32 -F dwarf
LDPARAMS = -m elf_i386
OUTPUT = .

objects = 	obj/kernel.o
LINKER_SCRIPT = ./linker.ld


qemu_path =  "/mnt/c/program files/qemu/qemu-system-x86_64.exe"
qemu_param = -vga std -D ./log.txt -d int,guest_errors -boot d -M q35 -serial mon:stdio -m 1G

all: os_s234.iso
	$(qemu_path) -cdrom $(OUTPUT)/os_s234.iso -no-reboot $(qemu_param)

clean:
	rm -rf kernel/*.o
	rm kernel/kernel.

os_s234.iso: kernel
	rm -rf iso_root
	mkdir -p iso_root
	cp -r userspace/ramdisk iso_root
	cp kernel/kernel.elf \
		limine.cfg limine/limine.sys limine/limine-cd.bin limine/limine-eltorito-efi.bin iso_root/
	xorriso -as mkisofs -b limine-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-eltorito-efi.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o os_s234.iso
	limine/limine-install os_s234.iso
	rm -rf iso_root

.PHONY: kernel
kernel:
	$(MAKE) -C kernel


limine:
	git clone https://github.com/limine-bootloader/limine.git --branch=v2.0-branch-binary --depth=1
	make -C limine