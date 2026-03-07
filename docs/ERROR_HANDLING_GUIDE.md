# Error Handling Guide - Sapphire v1.3.0

Quick reference for Option<T>, Result<T, E>, and the ? operator.

---

## Option<T> - Safe Nullable Values

Use Option when a value might be absent.

### Creating Options
```sapphire
let some = Some(42)
let none = None()
```

### Pattern Matching
```sapphire
match option:
    Some(x) => print("Value:", x)
    None() => print("No value")
```

### Methods
```sapphire
option.is_some()           # true if Some
option.is_none()           # true if None
option.unwrap()            # Get value or panic
option.unwrap_or(default)  # Get value or default
option.map(fn)             # Transform Some value
option.and_then(fn)        # Chain Option-returning functions
```

### Common Patterns
```sapphire
# Safe unwrap with default
let value = find_user(id).unwrap_or("guest")

# Transform if present
let doubled = Some(5).map(fn(x) { x * 2 })  # Some(10)

# Chain operations
let result = get_user(id)
    .map(fn(u) { u.name })
    .unwrap_or("Unknown")
```

---

## Result<T, E> - Type-Safe Error Handling

Use Result when an operation can fail.

### Creating Results
```sapphire
fn divide(a, b):
    if b == 0:
        return Err("division by zero")
    return Ok(a / b)
```

### Pattern Matching
```sapphire
match result:
    Ok(value) => print("Success:", value)
    Err(error) => print("Error:", error)
```

### Methods
```sapphire
result.is_ok()             # true if Ok
result.is_err()            # true if Err
result.unwrap()            # Get value or panic
result.unwrap_or(default)  # Get value or default
result.unwrap_err()        # Get error or panic
result.map(fn)             # Transform Ok value
result.map_err(fn)         # Transform Err value
result.and_then(fn)        # Chain Result-returning functions
```

### Common Patterns
```sapphire
# Safe unwrap with default
let value = divide(10, 2).unwrap_or(0)

# Transform success value
let doubled = Ok(5).map(fn(x) { x * 2 })  # Ok(10)

# Transform error
let prefixed = Err("fail").map_err(fn(e) { "[ERROR] " + e })

# Chain operations
let result = parse_int(input)
    .and_then(fn(n) { validate(n) })
    .and_then(fn(v) { process(v) })
```

---

## ? Operator - Error Propagation

Use ? to automatically propagate errors up the call stack.

### Basic Usage
```sapphire
fn calculate(a, b, c):
    let x = divide(a, b)?  # If Err, return immediately
    let y = divide(x, c)?  # If Err, return immediately
    return Ok(y)
```

### How It Works
- If Result is `Ok(value)`: unwraps to value and continues
- If Result is `Err(error)`: returns `Err(error)` from function
- Only works with Result types (not Option)

### Multi-Step Validation
```sapphire
fn validate_input(input):
    let checked = validate_not_empty(input)?
    let length_ok = validate_length(checked)?
    let format_ok = validate_format(length_ok)?
    return Ok(format_ok)
```

### Real-World Pipeline
```sapphire
fn process_file(path):
    let content = read_file(path)?
    let parsed = parse_json(content)?
    let validated = validate(parsed)?
    return Ok(validated)
```

---

## Choosing Between Option and Result

### Use Option When:
- Value might be absent (no error condition)
- Looking up in collections
- Optional function parameters
- Nullable fields

### Use Result When:
- Operation can fail with an error
- Need to communicate why something failed
- Want to propagate errors with ?
- Building error handling pipelines

---

## Best Practices

### 1. Use ? for Error Propagation
```sapphire
# Good - Clean and readable
fn process(input):
    let parsed = parse(input)?
    let validated = validate(parsed)?
    return Ok(validated)

# Avoid - Verbose and error-prone
fn process(input):
    let result1 = parse(input)
    if result1.is_err():
        return result1
    let parsed = result1.unwrap()
    # ... more boilerplate
```

### 2. Use Pattern Matching for Complex Logic
```sapphire
match authenticate(user, pass):
    Ok(token) => fetch_data(token)
    Err(error) => handle_auth_error(error)
```

### 3. Use unwrap_or for Defaults
```sapphire
let port = config.get("port").unwrap_or(8080)
let name = user.name.unwrap_or("Anonymous")
```

### 4. Chain Operations with map and and_then
```sapphire
let result = get_user(id)
    .map(fn(u) { u.email })
    .and_then(fn(e) { validate_email(e) })
    .unwrap_or("no-email@example.com")
```

### 5. Return Early with ?
```sapphire
fn complex_operation():
    let a = step1()?
    let b = step2(a)?
    let c = step3(b)?
    return Ok(c)
```

---

## Common Patterns

### Pattern 1: File Operations
```sapphire
fn read_and_parse(path):
    let content = read_file(path)?
    let data = parse_json(content)?
    return Ok(data)
```

### Pattern 2: Validation Chain
```sapphire
fn validate_user(input):
    let username = validate_username(input)?
    let email = validate_email(input)?
    let age = validate_age(input)?
    return Ok([username, email, age])
```

### Pattern 3: Authentication Flow
```sapphire
fn authenticate_and_fetch(creds):
    let user = verify_credentials(creds)?
    let token = generate_token(user)?
    let data = fetch_user_data(token)?
    return Ok(data)
```

### Pattern 4: Optional Chaining
```sapphire
fn get_user_email(id):
    let user = find_user(id)
    match user:
        Some(u) => Ok(u.email)
        None() => Err("user not found")
```

---

## Error Messages

### Option Errors
- `unwrap()` on None: "Called unwrap() on None"

### Result Errors
- `unwrap()` on Err: "Called unwrap() on Err"
- `unwrap_err()` on Ok: "Called unwrap_err() on Ok"

### ? Operator Errors
- Using ? on non-Result: "? operator can only be used with Result types"

---

## Performance

All operations are zero-cost:
- Pattern matching: Compiled to direct checks
- Method calls: Inlined by compiler
- ? operator: Direct value checks
- No allocations for error propagation

---

## Examples

See these files for complete examples:
- `examples/option_showcase.spp` - Option features
- `examples/result_showcase.spp` - Result features
- `examples/try_operator_showcase.spp` - ? operator
- `examples/milestone5_complete.spp` - All features together

---

## Quick Reference Card

```sapphire
# Option<T>
Some(value)                # Create Some
None()                     # Create None
opt.is_some()              # Check if Some
opt.is_none()              # Check if None
opt.unwrap()               # Get or panic
opt.unwrap_or(default)     # Get or default
opt.map(fn)                # Transform
opt.and_then(fn)           # Chain

# Result<T, E>
Ok(value)                  # Create Ok
Err(error)                 # Create Err
res.is_ok()                # Check if Ok
res.is_err()               # Check if Err
res.unwrap()               # Get or panic
res.unwrap_or(default)     # Get or default
res.unwrap_err()           # Get error or panic
res.map(fn)                # Transform value
res.map_err(fn)            # Transform error
res.and_then(fn)           # Chain

# ? Operator
expr?                      # Unwrap Ok or propagate Err
```

---

**Sapphire v1.3.0** - Modern error handling made simple
