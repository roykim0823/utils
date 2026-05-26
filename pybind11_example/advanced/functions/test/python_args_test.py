import python_args

print("=== Python objects as arguments, *args / **kwargs ===\n")

print("1. format_dict (native py::dict, no std::map conversion):")
print("  ", python_args.format_dict({"name": "Molly", "age": 3, "kind": "dog"}))
print()

print("2. list_length (native py::list):")
print("  ", python_args.list_length([1, 2, 3, 4, 5]))
print()

print("3. describe_args (*args + **kwargs):")
print("  ", python_args.describe_args(1, 2, 3, foo="x", bar="y"))
print("  ", python_args.describe_args())
print()

print("4. greet_many (fixed + *args + **kwargs):")
print("  ", python_args.greet_many("Hello", "Molly", "Charly", punct="!"))
print("  ", python_args.greet_many("Hi"))
