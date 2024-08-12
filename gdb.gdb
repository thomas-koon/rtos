# Connect to the target
target remote localhost:3333

# Load the ELF file
file rtos.elf

#break main.c:85
break *0x8000aa2
condition 1 curr_task->id == 1


# Reset and halt the target
monitor reset halt

# Load the program into the target
load

# Continue execution
continue

layout split
