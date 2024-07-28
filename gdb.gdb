# Connect to the target
target remote localhost:3333

# Load the ELF file
file rtos.elf

break main.c:76
break main.c:86
break main.c:96

# Reset and halt the target
monitor reset halt

# Load the program into the target
load

# Continue execution
continue

layout split
