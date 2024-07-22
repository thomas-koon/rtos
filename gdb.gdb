# Connect to the target
target remote localhost:3333

# Load the ELF file
file rtos.elf

# Set a breakpoint at scheduler.c:169
break scheduler.c:163

# Reset and halt the target
monitor reset halt

# Load the program into the target
load

# Continue execution
continue

layout split