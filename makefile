default: bin

main.s: main.c
	gcc -S main.c -m16 -Os -Wextra

pong.s: main.s
	echo "jmp _main" | cat - main.s > pong.s

pong.o: pong.s
	gcc -c pong.s -m16

bin: pong.o
	objcopy -O binary -j .text pong.o bin
	truncate -s %510 bin
	printf "\x55\xAA" >> bin

run: bin
	qemu-system-x86_64 -drive format=raw,file=bin

clean:
	rm -rf main.s pong.s pong.o bin
