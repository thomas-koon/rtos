# Define a mapping of debug IDs to more human-readable messages
def get_debug_message(debug_id, param):
    if debug_id == 0x0001:
        return f"Pool block initiated at {hex(param)}"
    elif debug_id == 0x0002:
        return f"Pool block allocated at {hex(param)}"
    elif debug_id == 0x0003:
        return f"Pool block freed at {hex(param)}"
    elif debug_id == 0x0004:
        return f"Current free list block at {hex(param)}"
    elif debug_id == 0x0005:
        return f"Mutex lock called by {hex(param)}"
    elif debug_id == 0x0006:
        return f"Mutex taken by {hex(param)}"
    elif debug_id == 0x0007:
        return f"Waiting for mutex held by {hex(param)}"
    elif debug_id == 0x0008:
        return f"Mutex released by {hex(param)}"
    elif debug_id == 0x0009:
        return f"Task switched to {hex(param)}"
    elif debug_id == 0x000A:
        return "Kernel starting"
    elif debug_id == 0x000B:
        return f"Current task ID is {hex(param)}"
    else:
        return f"Unknown debug code {hex(debug_id)} with param {hex(param)}"

# Function to translate UART logs from a file and write to another file
def translate_uart_output(input_file_path, output_file_path):
    translated_output = []
    
    with open(input_file_path, 'r', errors='ignore') as file:  # Ignore invalid chars
        with open(output_file_path, 'w') as outfile:
            for line in file:
                try:
                    # Split the UART output line into debug ID and parameter
                    debug_id, param = line.split()
                    debug_id = int(debug_id, 16)
                    param = int(param, 16) if param != "0" else 0
                    
                    # Get the human-readable message
                    message = get_debug_message(debug_id, param)
                    translated_output.append(message)
                    
                    # Write the message to the output file
                    outfile.write(message + "\n")
                
                except (ValueError, IndexError):
                    # Skip invalid/corrupted lines
                    continue

    return translated_output

# Example usage:
input_file_path = 'log.txt'    # Path to the file where UART output is saved
output_file_path = 'log_translated.txt'  # Path to save the translated output

# Translate the UART output and write it to a file
translated_output = translate_uart_output(input_file_path, output_file_path)

# Print the translated output to the console
for line in translated_output:
    print(line)
