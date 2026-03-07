# Traits Guide

**Status:** Core features implemented  
**Version:** 1.1.0-dev

---

## Overview

Traits define shared behavior across types. They're similar to interfaces in Java or protocols in Swift, but more powerful. Traits enable polymorphism without inheritance and allow you to add functionality to existing types.

---

## Defining Traits

```sapphire
trait Drawable:
    fn draw(self) -> str
    fn area(self) -> float

trait Comparable:
    fn compare(self, other) -> int

trait Printable:
    fn to_string(self) -> str
```

Traits define method signatures that implementing types must provide.

---

## Implementing Traits

### Standalone Implementation

```sapphire
class Circle:
    fn __init__(self, r):
        self.radius = r

impl Drawable for Circle:
    fn draw(self) -> str:
        return "Circle"
    
    fn area(self) -> float:
        return 3.14159 * self.radius * self.radius
```

### Multiple Traits

A type can implement multiple traits:

```sapphire
impl Drawable for Circle:
    fn draw(self) -> str:
        return "Circle"
    fn area(self) -> float:
        return 3.14159 * self.radius * self.radius

impl Printable for Circle:
    fn to_string(self) -> str:
        return "Circle"

impl Comparable for Circle:
    fn compare(self, other) -> int:
        if self.radius < other.radius:
            return -1
        if self.radius > other.radius:
            return 1
        return 0
```

---

## Using Trait Methods

Once a trait is implemented, you can call its methods on instances:

```sapphire
let c = Circle(5.0)
print(c.draw())        # "Circle"
print(c.area())        # 78.54
print(c.to_string())   # "Circle"
```

---

## Generic Functions with Traits

Use traits as constraints on generic functions:

```sapphire
# Any type that implements Drawable can be passed
fn print_drawable(item):
    print("Drawing:", item.draw())
    print("Area:", item.area())

let c = Circle(5.0)
let r = Rectangle(4.0, 6.0)

print_drawable(c)
print_drawable(r)
```

**Note:** Explicit trait bounds syntax (`<T: Drawable>`) is planned for future versions.

---

## Built-in Traits

Sapphire provides several built-in traits:

### Numeric
```sapphire
trait Numeric:
    fn add(self, other: Self) -> Self
    fn sub(self, other: Self) -> Self
```

Implemented by: `int`, `float`

### Comparable
```sapphire
trait Comparable:
    fn compare(self, other: Self) -> int
```

Implemented by: `int`, `float`, `str`

### Eq
```sapphire
trait Eq:
    fn equals(self, other: Self) -> bool
```

Implemented by: `int`, `float`, `str`, `bool`

### Iterable
```sapphire
trait Iterable:
    fn iter(self) -> List<T>
```

Implemented by: `List`, `str`

---

## Examples

### Example 1: Shape Hierarchy

```sapphire
trait Shape:
    fn area(self) -> float
    fn perimeter(self) -> float

class Circle:
    fn __init__(self, r):
        self.radius = r

impl Shape for Circle:
    fn area(self) -> float:
        return 3.14159 * self.radius * self.radius
    fn perimeter(self) -> float:
        return 2.0 * 3.14159 * self.radius

class Rectangle:
    fn __init__(self, w, h):
        self.width = w
        self.height = h

impl Shape for Rectangle:
    fn area(self) -> float:
        return self.width * self.height
    fn perimeter(self) -> float:
        return 2.0 * (self.width + self.height)

# Use polymorphically
fn print_shape_info(shape):
    print("Area:", shape.area())
    print("Perimeter:", shape.perimeter())

let c = Circle(5.0)
let r = Rectangle(4.0, 6.0)

print_shape_info(c)
print_shape_info(r)
```

### Example 2: Serialization

```sapphire
trait Serializable:
    fn to_json(self) -> str
    fn from_json(json: str) -> Self

class User:
    fn __init__(self, name, age):
        self.name = name
        self.age = age

impl Serializable for User:
    fn to_json(self) -> str:
        return "{\"name\":\"" + self.name + "\",\"age\":" + self.age + "}"
    fn from_json(json: str) -> Self:
        # Parse JSON and create User
        return User("parsed", 0)
```

### Example 3: Multiple Traits

```sapphire
trait Display:
    fn display(self) -> str

trait Debug:
    fn debug(self) -> str

trait Clone:
    fn clone(self) -> Self

class Point:
    fn __init__(self, x, y):
        self.x = x
        self.y = y

impl Display for Point:
    fn display(self) -> str:
        return "Point"

impl Debug for Point:
    fn debug(self) -> str:
        return "Point { x: " + self.x + ", y: " + self.y + " }"

impl Clone for Point:
    fn clone(self) -> Self:
        return Point(self.x, self.y)

let p = Point(3, 4)
print(p.display())
print(p.debug())
let p2 = p.clone()
```

---

## Advanced Features (Planned)

### Trait Inheritance

```sapphire
trait Shape:
    fn area(self) -> float

trait ColoredShape: Shape:
    fn color(self) -> str
```

### Default Methods

```sapphire
trait Greetable:
    fn name(self) -> str
    
    fn greet(self) -> str:
        return "Hello, " + self.name() + "!"
```

### Associated Types

```sapphire
trait Container:
    type Item
    fn get(self, index: int) -> Item
    fn set(self, index: int, value: Item)
```

### Where Clauses

```sapphire
fn complex<T, U>(a: T, b: U) -> T
    where T: Drawable + Clone,
          U: Comparable:
    let copy = a.clone()
    return copy
```

### Trait Objects

```sapphire
fn draw_all(shapes: List<dyn Drawable>):
    for shape in shapes:
        print(shape.draw())
```

---

## Best Practices

### 1. Small, Focused Traits

```sapphire
# Good - single responsibility
trait Drawable:
    fn draw(self) -> str

trait Resizable:
    fn resize(self, factor) -> float

# Less ideal - too many responsibilities
trait ShapeOps:
    fn draw(self) -> str
    fn resize(self, factor) -> float
    fn rotate(self, angle) -> float
    fn translate(self, x, y)
```

### 2. Descriptive Names

Use clear, descriptive trait names:
- `Drawable` not `D`
- `Comparable` not `Comp`
- `Serializable` not `Serial`

### 3. Consistent Method Names

Follow conventions:
- `to_string()` for string conversion
- `clone()` for copying
- `compare()` for ordering

---

## Comparison with Other Languages

### Rust Traits
```rust
trait Drawable {
    fn draw(&self) -> String;
}

impl Drawable for Circle {
    fn draw(&self) -> String {
        "Circle".to_string()
    }
}
```

### Sapphire Traits
```sapphire
trait Drawable:
    fn draw(self) -> str

impl Drawable for Circle:
    fn draw(self) -> str:
        return "Circle"
```

Similar concepts, but Sapphire uses Python-style syntax.

---

## Performance

Trait method dispatch:
- Static dispatch: O(1) when type is known at compile time
- Dynamic dispatch: O(1) hash table lookup at runtime
- No virtual table overhead
- Inline optimization possible

---

## Common Patterns

### Pattern 1: Polymorphic Collections

```sapphire
trait Animal:
    fn speak(self) -> str

class Dog:
    fn __init__(self, name):
        self.name = name

class Cat:
    fn __init__(self, name):
        self.name = name

impl Animal for Dog:
    fn speak(self) -> str:
        return "Woof!"

impl Animal for Cat:
    fn speak(self) -> str:
        return "Meow!"

fn make_sounds(animals):
    for animal in animals:
        print(animal.speak())
```

### Pattern 2: Builder Pattern

```sapphire
trait Builder:
    fn build(self) -> Self

class Config:
    fn __init__(self):
        self.settings = []
    fn add_setting(self, key, value):
        self.settings.append([key, value])
        return self

impl Builder for Config:
    fn build(self) -> Self:
        return self

let config = Config().add_setting("debug", true).build()
```

### Pattern 3: Strategy Pattern

```sapphire
trait SortStrategy:
    fn sort(self, items) -> list

class QuickSort:
    fn __init__(self):
        pass

impl SortStrategy for QuickSort:
    fn sort(self, items) -> list:
        # Quick sort implementation
        return items

class MergeSort:
    fn __init__(self):
        pass

impl SortStrategy for MergeSort:
    fn sort(self, items) -> list:
        # Merge sort implementation
        return items

fn sort_with_strategy(items, strategy):
    return strategy.sort(items)
```

---

## Limitations

Current limitations (to be addressed in future versions):
- No trait inheritance yet
- No default method implementations
- No associated types
- No where clauses
- No trait objects (dyn Trait)
- Type checking for trait bounds not enforced

---

## See Also

- [Generics](GENERICS.md)
- [Type System](TYPE_INFERENCE.md)
- [OOP Features](LANGUAGE_FEATURES.md)
- [Pattern Matching](PATTERN_MATCHING.md)

