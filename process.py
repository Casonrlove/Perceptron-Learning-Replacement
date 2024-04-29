import os

def extract_ipc_and_cachemiss_from_directory(directory, output_file):
    # Loop through all files in the directory
    filenames = sorted(os.listdir(directory))
    for filename in filenames:
        # Find filename
        index = filename.find(".")

        # Get the substring between the two dots
        name = filename[index + 1:]

        file_path = os.path.join(directory, filename)
        with open(file_path, 'r') as file:
            for line in file:
                if "CPU 0 cumulative IPC:" in line:
                    parts = line.split()
                    ipc_index = parts.index("IPC:")
                    ipc = float(parts[ipc_index + 1])
                elif "LLC TOTAL" in line:
                    parts = line.split()
                    miss_index = parts.index("MISS:")
                    # Extract the values
                    total_miss = int(parts[miss_index + 1]) / 100000
                elif "LLC LOAD" in line:
                    parts = line.split()
                    miss_index = parts.index("MISS:")
                    # Extract the values
                    load_miss = int(parts[miss_index + 1]) / 100000
                    break

        with open(output_file, 'a') as output:
            output.write(str(name) + " ")
            output.write(str(ipc) + " ")
            output.write(str(total_miss) + " ")
            output.write(str(load_miss) + "\n")

# Example usage:
directory = "HW1/champsim"  # Replace with your directory path
output_file = "data_process.txt"
# script_values, ipc_values, cachemiss_values = 
extract_ipc_and_cachemiss_from_directory(directory, output_file)
# print("IPC values:", ipc_values)
