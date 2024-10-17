# Connect to the target
target remote localhost:3333

# Load the ELF file
file rtos.elf

# Reset and halt the target
monitor reset halt

# Load the program into the target
load

break queue.c:115
c