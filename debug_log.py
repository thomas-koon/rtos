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
    
    elif debug_id == 0x000C:
        return f"Task {hex(param)} posting to MQ"
    elif debug_id == 0x000D:
        return f"Data {hex(param)} posted to MQ"
    elif debug_id == 0x000E:
        return f"Task {hex(param)} blocked trying to post to MQ"
    elif debug_id == 0x000F:
        return f"Task {hex(param)} pending on MQ"
    elif debug_id == 0x0010:
        return f"Data {hex(param)} received from MQ"
    elif debug_id == 0x0011:
        return f"Task {hex(param)} blocked pending on MQ"
    
    elif debug_id == 0x0012:
        return f"Task {hex(param)} posted the semaphore"
    elif debug_id == 0x0013:
        return f"Task {hex(param)} waiting on semaphore"
    elif debug_id == 0x0014:
        return f"Semaphore new count is {hex(param)}"
    elif debug_id == 0x0015:
        return f"Task {hex(param)} blocked waiting on semaphore"
    elif debug_id == 0x0016:
        return f"Task {hex(param)} awaken by semaphore"
    
    elif debug_id == 0x0017:
        return f"Task {hex(param)} blocked waiting on CV"
    elif debug_id == 0x0018:
        return f"Task {hex(param)} woke up from waiting on CV"
    elif debug_id == 0x0019:
        return f"Task {hex(param)} signaled on the CV"
    
    elif debug_id == 0x0020:
        return f"----- Misc debug message A: {hex(param)} ----- "
    
    elif debug_id == 0x0021:
        return f"##### Misc debug message B: {hex(param)} ##### "
    
    elif debug_id == 0x0022:
        return f"~~~~~ Misc debug message C: {hex(param)} ~~~~~ "
    
    elif debug_id == 0x0023:
        return f"===== Misc debug message D: {hex(param)} ===== "
    
    elif debug_id == 0x0024:
        return f"+++++ Misc debug message E: {hex(param)} +++++" 

    
    elif debug_id == 0xFFFF:
        return f"\r\n"
    else:
        return f"Unknown debug code {hex(debug_id)} with param {hex(param)}"

# Function to translate UART logs from a file and write to another file
def translate_uart_output(input_file_path, output_file_path):
    translated_output = []
    
    with open(input_file_path, 'r', errors='ignore') as file:  # Ignore invalid chars
        with open(output_file_path, 'w') as outfile:
            for line in file:
                line = line.strip()  # Remove any extra whitespace
                try:
                    # Check if line contains two space-separated parts
                    if len(line.split()) == 2:
                        debug_id, param = line.split()
                        debug_id = int(debug_id, 16)
                        param = int(param, 16) if param != "0" else 0
                        
                        # Get the human-readable message
                        message = get_debug_message(debug_id, param)
                    else:
                        # If the line doesn't match expected format, treat it as plain text
                        message = line

                    translated_output.append(message)
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
