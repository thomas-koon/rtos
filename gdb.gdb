# Connect to the target
target remote localhost:3333

# Load the ELF file
file rtos.elf

break *0x80008da
#break scheduler.c:181

# Reset and halt the target
monitor reset halt

# Load the program into the target
load

# Continue execution
continue

layout split
