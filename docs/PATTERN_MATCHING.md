# Pattern Matching Guide

**Status:** Core features implemented  
**Version:** 1.1.0-dev

---

## Overview

Pattern matching provides a powerful way to destructure and match values against patterns. It's more expressive than traditional if-else chains and enables cleaner, more maintainable code.

---

## Basic Syntax

```sapphire
match <value>:
    <pattern1> => <expression1>
    <pattern2> => <expression2>
    _ => <default_expression>
```

---

## Pattern Types

### 1. Literal Patterns

Match exact values:

```sapphire
fn classify_number(n):
    return match n:
        0 => "zero"
        1 => "one"
        2 => "two"
        _ => "other"

print(classify_number(0))  # "zero"
print(classify_number(5))  # "other"
```

### 2. Variable Patterns

Bind the matched value to a variable:

```sapphire
fn describe(n):
    return match n:
        x => "The number is " + x

# Variable patterns always match and bind the value
```

### 3. Wildcard Pattern

Match anything without binding:

```sapphire
fn is_zero(n):
    return match n:
        0 => true
        _ => false
```

### 4. Multiple Arms

Match against multiple patterns:

```sapphire
fn day_type(day):
    return match day:
        1 => "Monday"
        2 => "Tuesday"
        3 => "Wednesday"
        4 => "Thursday"
        5 => "Friday"
        6 => "Saturday"
        7 => "Sunday"
        _ => "Invalid day"
```

---

## Guard Clauses (Planned)

Add conditions to patterns:

```sapphire
fn classify(n):
    return match n:
        x if x < 0 => "negative"
        x if x > 0 => "positive"
        _ => "zero"
```

**Status:** Parser support added, runtime needs debugging

---

## Array Destructuring (Planned)

Match array structures:

```sapphire
fn describe_list(items):
    return match items:
        [] => "empty"
        [x] => "single item"
        [first, second] => "two items"
        [first, ...rest] => "many items"
        _ => "other"
```

**Status:** AST nodes created, needs testing

---

## Object Destructuring (Planned)

Match object structures:

```sapphire
fn describe_point(point):
    return match point:
        {x: 0, y: 0} => "origin"
        {x, y} => "point at coordinates"
```

**Status:** AST nodes created, needs testing

---

## Best Practices

### 1. Always Include Wildcard

Ensure exhaustive matching:

```sapphire
# Good
match value:
    0 => "zero"
    _ => "other"

# Bad (may throw error if not exhaustive)
match value:
    0 => "zero"
```

### 2. Order Matters

Patterns are checked in order:

```sapphire
match n:
    _ => "always matches first"
    0 => "never reached"
```

### 3. Use Guards for Conditions

```sapphire
# Good
match n:
    x if x < 0 => "negative"
    x if x > 0 => "positive"
    _ => "zero"

# Less clear
match n:
    x => if x < 0 { "negative" } else { "positive" }
```

---

## Examples

### Example 1: HTTP Status Codes

```sapphire
fn status_message(code):
    return match code:
        200 => "OK"
        201 => "Created"
        400 => "Bad Request"
        404 => "Not Found"
        500 => "Internal Server Error"
        _ => "Unknown Status"
```

### Example 2: Option Type (Future)

```sapphire
fn get_value(opt):
    return match opt:
        Some(x) => x
        None => "No value"
```

### Example 3: Result Type (Future)

```sapphire
fn handle_result(result):
    return match result:
        Ok(value) => "Success: " + value
        Err(error) => "Error: " + error
```

---

## Performance

Pattern matching is compiled to efficient branching code:
- Literal patterns: O(1) comparison
- Variable patterns: O(1) binding
- Array patterns: O(n) where n is array length
- Object patterns: O(m) where m is number of fields

---

## Comparison with Other Languages

### Rust
```rust
match value {
    0 => "zero",
    x if x < 0 => "negative",
    _ => "positive"
}
```

### Sapphire
```sapphire
match value:
    0 => "zero"
    x if x < 0 => "negative"
    _ => "positive"
```

Similar syntax, but Sapphire uses indentation instead of braces.

---

## Future Enhancements

- [ ] Or patterns: `1 | 2 | 3 => "small"`
- [ ] Range patterns: `1..10 => "small"`
- [ ] Type patterns: `x: int => "integer"`
- [ ] Exhaustiveness checking at compile time
- [ ] Pattern macros for custom patterns

---

## See Also

- [Control Flow](tutorials/02_CONTROL_FLOW.md)
- [Functions](tutorials/03_FUNCTIONS.md)
- [Traits](TRAITS.md)

