# Compiler
CC = arm-none-eabi-gcc

# Toolchain and filenames
OBJCOPY = arm-none-eabi-objcopy
FLASH = st-flash
ELF = rtos.elf
BIN = rtos.bin
OBJECTS = main.o other_module.o

# Linker flags
LDFLAGS = -T STM32F446RETX_FLASH.ld -mcpu=cortex-m4 -mthumb

# Include directories
INC_DIRS = \
    Core/Inc \
    Drivers/STM32F4xx_HAL_Driver/Inc \
    Drivers/CMSIS/Device/ST/STM32F4xx/Include \
    Drivers/CMSIS/Include

# Define compiler flags
CFLAGS = -mcpu=cortex-m4 -mthumb -nostdlib $(addprefix -I, $(INC_DIRS)) -DSTM32F446xx -g

# Source file directories
CORE_SRC_DIR = Core/Src
STARTUP_SRC_DIR = Core/Startup
DRIVERS_SRC_DIR = Drivers/STM32F4xx_HAL_Driver/Src

# Source files from directories
SRCS = $(wildcard $(CORE_SRC_DIR)/*.c) $(wildcard $(STARTUP_SRC_DIR)/*.s) $(wildcard $(DRIVERS_SRC_DIR)/*.c)
OBJS = $(SRCS:.c=.o) 

# Output file
TARGET = rtos.elf

# Build rule
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(filter-out $(STARTUP_SRC_DIR)/%.s, $(OBJS)) $(TARGET)

# Convert ELF to BIN and Flash
flash: $(ELF)
	$(OBJCOPY) -O binary $(ELF) $(BIN)
	$(FLASH) write $(BIN) 0x8000000
