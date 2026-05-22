#pragma once

#include <string>

struct Pet {
  Pet(const std::string& name) : name(name) {}
  void setName (const std::string& name_) { name = name_; }
  const std::string& getName() const { return name; }

  std::string name;
  virtual ~Pet() = default; // Add a virtual destructor for proper cleanup of derived classes
};

struct Dog : public Pet {
  Dog(const std::string& name) : Pet(name) {}
  std::string bark() const { return "Woof!"; }
};

class Pet2 {
public:
  Pet2(const std::string& name) : name(name) {}
  void setName (const std::string& name_) { name = name_; }
  const std::string& getName() const { return name; }

private:
  std::string name;
};

struct PetO {
  PetO(const std::string& name, int age) : name(name), age(age) {}

  void set(int age_) { age = age_; }
  void set(const std::string& name_) { name = name_; }

  std::string name;
  int age;
};

struct Widget {
    int foo(int x, float y)       { return x + static_cast<int>(y); }
    int foo(int x, float y) const { return x * static_cast<int>(y); }
};