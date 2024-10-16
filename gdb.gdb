# Connect to the target
target remote localhost:3333

# Load the ELF file
file rtos.elf

# Reset and halt the target
monitor reset halt

# Load the program into the target
load

# switch twice
break kernel.c:189
c
c

# look at freeing 0x20002800 on switch
break pool.c:136

# Faults here
break list.c:25

c