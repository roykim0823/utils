import arg_modifiers

print("=== Argument modifiers ===\n")

# --- 5. defaults ---
print("5. Default arguments:")
print("   add()       =", arg_modifiers.add())
print("   add(1)      =", arg_modifiers.add(1))
print("   add(1, 2)   =", arg_modifiers.add(1, 2))
cfg = arg_modifiers.Config()
print("   Config() ->", cfg.a, cfg.b)
print("   with_config() =", arg_modifiers.with_config())
print("   help(with_config) shows the arg_v signature:")
print("    ", arg_modifiers.with_config.__doc__)
print()

# --- 6. kw_only ---
print("6. kw_only_add — b must be keyword:")
print("   kw_only_add(1, b=2) =", arg_modifiers.kw_only_add(1, b=2))
try:
    arg_modifiers.kw_only_add(1, 2)
except TypeError as e:
    print("   kw_only_add(1, 2)   -> TypeError:", e)
print()

# --- 7. pos_only ---
print("7. pos_only_add — a is positional-only:")
print("   pos_only_add(1, 2)        =", arg_modifiers.pos_only_add(1, 2))
print("   pos_only_add(1, b=2)      =", arg_modifiers.pos_only_add(1, b=2))
try:
    arg_modifiers.pos_only_add(a=1, b=2)
except TypeError as e:
    print("   pos_only_add(a=1, b=2)    -> TypeError:", e)
print()

# --- 8. noconvert ---
print("8. .noconvert() — only_float rejects ints:")
print("   supports_float(2) =", arg_modifiers.supports_float(2))     # int->float OK
print("   only_float(2.0)   =", arg_modifiers.only_float(2.0))       # exact float OK
try:
    arg_modifiers.only_float(2)
except TypeError as e:
    print("   only_float(2)     -> TypeError:", e)
print()

# --- 9. none(true)/.none(false) ---
print("9. .none() — bark accepts None, meow rejects None:")
print("   bark(Dog())  =", arg_modifiers.bark(arg_modifiers.Dog()))
print("   bark(None)   =", arg_modifiers.bark(None))
print("   meow(Cat())  =", arg_modifiers.meow(arg_modifiers.Cat()))
try:
    arg_modifiers.meow(None)
except TypeError as e:
    print("   meow(None)   -> TypeError:", e)