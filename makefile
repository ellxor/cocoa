bin: main.o padding
	objcopy -O binary -j .text main.o bin
	./padding

main.o: main.c
	clang -c main.c -m16 -Os -Wextra

run: bin
	qemu-system-x86_64 -drive format=raw,file=bin

debug: main.c
	clang -S main.c -m16 -Os -Wextra

clean:
	rm -rf main.o bin
