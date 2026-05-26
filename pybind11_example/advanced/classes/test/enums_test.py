import enums

print("=== Enumerations ===\n")

# --- Unscoped enum: values re-exported into module scope ---
print("Unscoped Color — Color.Red AND enums.Red both work (export_values):")
print("    Color.Red          =", enums.Color.Red)
print("    enums.Red          =", enums.Red, "  (re-exported)")
print("    Color.Red is Red   ?", enums.Color.Red is enums.Red)
print("    describe_color(Blue)=", enums.describe_color(enums.Blue))
print()

# --- Scoped enum class: stays under its type ---
print("Scoped Status — must be qualified, NOT re-exported:")
print("    Status.Ok           =", enums.Status.Ok)
print("    describe_status(Ok) =", enums.describe_status(enums.Status.Ok))
print("    hasattr(enums, 'Ok')?", hasattr(enums, "Ok"))
print()

# --- IntFlag enum: |, & work for bitflag composition ---
print("Bitflag Perm via py::native_enum + IntFlag:")
rw = enums.Perm.Read | enums.Perm.Write
print("    Perm.Read | Perm.Write       =", rw, "raw=", int(rw))
print("    (Read|Write) & Read          =", rw & enums.Perm.Read)
print("    describe_perm(Read|Write|Exec)=",
      enums.describe_perm(enums.Perm.Read | enums.Perm.Write | enums.Perm.Exec))
