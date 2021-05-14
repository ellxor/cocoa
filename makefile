bin: main.c
	gcc -S main.c -m16 -Os -Wextra			# compile C to 16-bit asm
	echo "jmp _main" | cat - main.s > pong.s	# prepend jmp _main
	gcc -c pong.s -m16				# compile asm to object file
	objcopy -O binary -j .text pong.o bin		# copy .text to raw binary file
	ls -lh | grep bin 				# inspect binary size
	truncate -s %510 bin				# pad binary to 510 bytes
	printf "\x55\xAA" >> bin			# append 0x55AA boot signature

run: bin
	qemu-system-x86_64 -drive format=raw,file=bin

clean:
	rm -rf main.s pong.s pong.o
