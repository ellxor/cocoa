default: bin

main.s: main.c
	gcc -S main.c -m16 -Os -Wextra

pong.s: main.s
	echo "jmp _main" | cat - main.s > pong.s

pong.o: pong.s
	gcc -c pong.s -m16

bin: pong.o padding
	objcopy -O binary -j .text pong.o bin
	./padding

run: bin
	qemu-system-x86_64 -drive format=raw,file=bin

clean:
	rm -rf main.s pong.s pong.o bin
