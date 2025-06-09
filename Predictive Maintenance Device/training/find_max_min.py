# Open the file and read the contents
file_path = r'dataset/FS2.txt'

with open(file_path, 'r') as file:
    data = file.read()

# Split the data into a list of float values
readings = list(map(float, data.split()))

# Find the maximum and minimum values
max_value = max(readings)
min_value = min(readings)

# Print the results
print(f"Maximum Value: {max_value}")
print(f"Minimum Value: {min_value}")
