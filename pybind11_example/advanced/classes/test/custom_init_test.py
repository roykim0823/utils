import custom_init

print("=== Custom constructors + non-public destructors ===\n")

# --- 5. Factory constructors via py::init(lambda) ---
print("5a. Real C++ ctor py::init<int, std::string>:")
w1 = custom_init.Widget(7, "alpha")
print("    Widget(7, 'alpha') =", w1)
print()

print("5b. Factory lambda returning Widget by value (single int arg):")
w2 = custom_init.Widget(42)
print("    Widget(42)         =", w2)
print()

print("5c. Factory lambda returning std::unique_ptr<Widget> (single str arg):")
w3 = custom_init.Widget("named-only")
print("    Widget('named-only') =", w3)
print()

# --- 6. Non-public destructor via py::nodelete ---
print("6. Singleton — private dtor, py::nodelete holder:")
s1 = custom_init.Singleton.instance()
s2 = custom_init.Singleton.instance()
print("    s1 is s2 (same C++ object)?", s1 is s2 or "different Python wrappers, same C++ ptr")
print("    counter before bumps =", s1.counter())
s1.bump(); s1.bump(); s2.bump()
print("    after 3 bumps        =", s2.counter())

# Cannot construct directly — no py::init bound:
try:
    custom_init.Singleton()
except TypeError as e:
    print("    Singleton() -> TypeError:", e)
