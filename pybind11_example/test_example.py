import example

print(example.add(3, 5))       # Outputs: 8
print(example.subtract(10, 4)) # Outputs: 6

# Class
calc = example.Calculator(5)
calc.add(3)
print(calc.get())

import numpy as np

# Create a NumPy array
array = np.array([1.0, 2.0, 3.0])

# Call the C++ function
scaled = example.scale_array(array, 2.0)
print(scaled)  # Outputs: [2. 4. 6.]