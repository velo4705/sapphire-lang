// ─── Sapphire syntax mode for CodeMirror ────────────────────────────────────
CodeMirror.defineSimpleMode('sapphire', {
    start: [
        { regex: /#.*/, token: 'comment' },
        { regex: /"(?:[^\\]|\\.)*?"/, token: 'string' },
        { regex: /'(?:[^\\]|\\.)*?'/, token: 'string' },
        { regex: /@\w+/, token: 'attribute' },
        { regex: /\b(?:fn|let|mut|return|if|else|elif|for|while|in|match|import|from|class|struct|trait|impl|async|await|try|catch|throw|new|self|true|false|null|and|or|not|is|as)\b/, token: 'keyword' },
        { regex: /\b(?:int|float|str|bool|void|Option|Result|Vec|HashMap)\b/, token: 'type' },
        { regex: /\b(?:print|println|input|len|range|push|pop|append|map|filter|reduce|zip|enumerate|format|parse|to_string|to_int|to_float)\b/, token: 'builtin' },
        { regex: /\b\d+(?:\.\d+)?\b/, token: 'number' },
        { regex: /=>|->|\.\.|\?\.|[+\-*/%=<>!&|^~?:]+/, token: 'operator' },
        { regex: /\b[A-Za-z_]\w*(?=\s*\()/, token: 'def' },
        { regex: /[{}[\]()]/, token: 'bracket' },
    ],
    meta: { lineComment: '#' }
});

// ─── Example programs ────────────────────────────────────────────────────────
const EXAMPLES = {
    hello: {
        file: 'hello.spp',
        code: `# Hello World in Sapphire
print("Hello, Sapphire! 💎")
print("Welcome to the playground.")`,
        output: `Hello, Sapphire! 💎
Welcome to the playground.`
    },

    fibonacci: {
        file: 'fibonacci.spp',
        code: `fn fibonacci(n):
    if n <= 1:
        return n
    return fibonacci(n - 1) + fibonacci(n - 2)

print("Fibonacci sequence:")
for i in range(10):
    print(fibonacci(i))`,
        output: `Fibonacci sequence:
0
1
1
2
3
5
8
13
21
34`
    },

    functions: {
        file: 'functions.spp',
        code: `fn greet(name):
    return "Hello, " + name + "!"

fn add(a, b):
    return a + b

fn factorial(n):
    if n <= 1:
        return 1
    return n * factorial(n - 1)

print(greet("Sapphire"))
print(add(3, 7))
print(factorial(10))`,
        output: `Hello, Sapphire!
10
3628800`
    },

    loops: {
        file: 'loops.spp',
        code: `# Range loop
for i in range(5):
    print(i)

# While loop
n = 1
while n < 32:
    n = n * 2
print(n)`,
        output: `0
1
2
3
4
32`
    },

    pattern: {
        file: 'pattern_matching.spp',
        code: `fn classify(n):
    return match n:
        0 => "zero"
        1 => "one"
        2 => "two"
        _ => "other"

print(classify(0))
print(classify(1))
print(classify(2))
print(classify(99))`,
        output: `zero
one
two
other`
    },

    classes: {
        file: 'classes.spp',
        code: `class Animal:
    fn __init__(self, name):
        self.name = name

    fn speak(self):
        return "..."

class Dog(Animal):
    fn __init__(self, name, breed):
        self.name = name
        self.breed = breed

    fn speak(self):
        return "Woof!"

class Cat(Animal):
    fn speak(self):
        return "Meow!"

dog = Dog("Buddy", "Golden Retriever")
cat = Cat("Whiskers")

print(dog.name, "the", dog.breed, "says:", dog.speak())
print(cat.name, "says:", cat.speak())`,
        output: `Buddy the Golden Retriever says: Woof!
Whiskers says: Meow!`
    },

    errors: {
        file: 'error_handling.spp',
        code: `# Basic try-catch
try:
    throw "Division by zero"
catch e:
    print("Caught:", e)

# Finally block
try:
    print("In try")
finally:
    print("In finally")

# Nested try-catch
try:
    try:
        throw "inner error"
    catch e:
        print("Inner catch")
    throw "outer error"
catch e:
    print("Outer catch")`,
        output: `Caught: RuntimeError: RuntimeError: Division by zero
In try
In finally
Inner catch
Outer catch`
    },

    hashmap: {
        file: 'hashmap.spp',
        code: `import HashMap from "stdlib/collections"

scores = HashMap.new()
scores.insert("Alice", 95)
scores.insert("Bob", 87)
scores.insert("Carol", 92)

print(scores.get("Alice"))
print(scores.get("Bob"))
print(scores.has("Carol"))
print(scores.has("Dave"))
print(scores.size())`,
        output: `95
87
true
false
3`
    },

    decorators: {
        file: 'decorators.spp',
        code: `@cache
fn fib(n):
    if n <= 1:
        return n
    return fib(n - 1) + fib(n - 2)

@timing
fn compute():
    sum = 0
    for i in range(1000):
        sum = sum + i
    print(sum)

print(fib(10))
compute()`,
        output: `55
499500
Function 'compute' took 0.12ms`
    },

    arrays: {
        file: 'arrays.spp',
        code: `arr = [1, 2, 3, 4, 5]
print(arr)
print(arr[0])
print(arr[4])
print(len(arr))

# Index-based iteration
for i in range(3):
    print(arr[i])

# Nested arrays
matrix = [[1, 2], [3, 4], [5, 6]]
print(matrix[0])
print(matrix[1][1])`,
        output: `[1, 2, 3, 4, 5]
1
5
5
1
2
3
[1, 2]
4`
    },

    oop: {
        file: 'oop.spp',
        code: `class Shape:
    fn area(self):
        return 0

class Rectangle(Shape):
    fn __init__(self, w, h):
        self.w = w
        self.h = h

    fn area(self):
        return self.w * self.h

class Circle(Shape):
    fn __init__(self, r):
        self.r = r

    fn area(self):
        return 3.14159 * self.r * self.r

rect = Rectangle(5, 10)
circ = Circle(7)

print(rect.area())
print(circ.area())`,
        output: `50
153.937910`
    },

    sorting: {
        file: 'sorting.spp',
        code: `fn sort(arr):
    i = 0
    while i < len(arr):
        j = i + 1
        while j < len(arr):
            if arr[i] > arr[j]:
                tmp = arr[i]
                arr[i] = arr[j]
                arr[j] = tmp
            j = j + 1
        i = i + 1

nums = [5, 2, 8, 1, 9, 3]
sort(nums)
print(nums)
print(nums[0])
print(nums[5])`,
        output: `[1, 2, 3, 5, 8, 9]
1
9`
    },

    generics: {
        file: 'generics.spp',
        code: `fn identity[T](x):
    return x

fn swap[T](a, b):
    return (b, a)

print(identity[int](42))
print(identity[str]("hello"))
let (x, y) = swap(1, 2)
print(x)
print(y)`,
        output: `42
hello
2
1`
    },

    traits: {
        file: 'traits.spp',
        code: `trait Describable:
    fn describe(self) -> String

class Person:
    fn __init__(self, name, age):
        self.name = name
        self.age = age

impl Describable for Person:
    fn describe(self):
        return self.name + " is " + self.age + " years old"

fn print_description[T: Describable](item):
    print(item.describe())

alice = Person("Alice", 30)
print_description(alice)`,
        output: `Alice is 30 years old`
    },

    channels: {
        file: 'channels.spp',
        code: `ch = Channel[int]()
fn producer():
    ch.send(10)
    ch.send(20)
    ch.send(30)

fn consumer():
    for msg in ch:
        print(msg)

async producer()
async consumer()
print("done")`,
        output: `10
20
30
done`
    },

    fizzbuzz: {
        file: 'fizzbuzz.spp',
        code: `fn fizzbuzz(n):
    if n % 15 == 0:
        return "FizzBuzz"
    if n % 3 == 0:
        return "Fizz"
    if n % 5 == 0:
        return "Buzz"
    return n

i = 1
while i <= 15:
    print(fizzbuzz(i))
    i = i + 1`,
        output: `1
2
Fizz
4
Buzz
Fizz
7
8
Fizz
Buzz
11
Fizz
13
14
FizzBuzz`
    },
};

// ─── CodeMirror setup ────────────────────────────────────────────────────────
let editor;

function updateStatusBar() {
    if (!editor) return;
    const cursor = editor.getCursor();
    const code = editor.getValue();
    document.getElementById('statusCursor').textContent =
        'Ln ' + (cursor.line + 1) + ', Col ' + (cursor.ch + 1);
    document.getElementById('statusChars').textContent =
        code.length + ' chars' + ' | ' + editor.lineCount() + ' lines';
}

function initEditor(code) {
    if (editor) {
        editor.setValue(code);
        updateStatusBar();
        return;
    }
    editor = CodeMirror.fromTextArea(document.getElementById('editor'), {
        mode: 'sapphire',
        theme: 'dracula',
        lineNumbers: true,
        matchBrackets: true,
        autoCloseBrackets: true,
        indentUnit: 4,
        tabSize: 4,
        indentWithTabs: false,
        lineWrapping: false,
        extraKeys: {
            'Ctrl-Enter': runCode,
            'Cmd-Enter': runCode,
            'Tab': (cm) => cm.execCommand('insertSoftTab')
        }
    });
    editor.on('cursorActivity', updateStatusBar);
    editor.on('change', updateStatusBar);
    editor.setValue(code);
    updateStatusBar();
}

// ─── State ───────────────────────────────────────────────────────────────────
let currentExample = null;

function loadExample(key) {
    const ex = EXAMPLES[key];
    if (!ex) return;
    currentExample = key;

    document.getElementById('currentFile').textContent = ex.file;
    initEditor(ex.code);

    document.querySelectorAll('.example-item').forEach(el => {
        el.classList.toggle('active', el.dataset.example === key);
    });

    setOutput('', '');
}

// ─── Run simulation ──────────────────────────────────────────────────────────
function runCode() {
    const btn = document.getElementById('btnRun');
    const t0 = performance.now();
    btn.classList.add('running');
    btn.textContent = '⏳ Running...';
    setOutput('', 'running');

    setTimeout(() => {
        const code = editor.getValue().trim();
        const result = simulate(code);
        const elapsed = ((performance.now() - t0) / 1000).toFixed(3);

        setOutput(result.output, result.error ? 'error' : 'ok', elapsed);
        btn.classList.remove('running');
        btn.textContent = '▶ Run';
    }, 400);
}

function simulate(code) {
    // Match against known examples
    for (const key of Object.keys(EXAMPLES)) {
        if (normalize(code) === normalize(EXAMPLES[key].code)) {
            return { output: EXAMPLES[key].output, error: false };
        }
    }
    return simulateGeneric(code);
}

function normalize(s) {
    return s.replace(/\s+/g, ' ').trim();
}

function simulateGeneric(code) {
    const lines = [];
    const printRegex = /print\s*\(\s*"([^"]*)"\s*\)/g;
    let match;
    while ((match = printRegex.exec(code)) !== null) {
        lines.push(match[1]);
    }

    if (lines.length > 0) {
        return { output: lines.join('\n'), error: false };
    }

    const openBraces = (code.match(/\{/g) || []).length;
    const closeBraces = (code.match(/\}/g) || []).length;
    if (Math.abs(openBraces - closeBraces) > 2) {
        return { output: 'error: mismatched braces', error: true };
    }

    return {
        output: '[Playground] Custom code requires a local Sapphire install.\nThis playground simulates output for the built-in examples.\n\nInstall Sapphire:\n  curl -fsSL https://raw.githubusercontent.com/velo4705/sapphire-lang/main/install.sh | bash',
        error: false
    };
}

function setOutput(text, status, elapsed) {
    const out = document.getElementById('output');
    const statusEl = document.getElementById('outputStatus');

    if (!text) {
        out.innerHTML = '<span class="output-hint">Press <strong>▶ Run</strong> or <strong>Ctrl+Enter</strong> to execute your code.</span>';
        statusEl.textContent = '';
        statusEl.className = 'output-status';
        return;
    }

    if (status === 'error') {
        out.innerHTML = `<span class="output-error">${escapeHtml(text)}</span>`;
        statusEl.textContent = 'error';
        statusEl.className = 'output-status err';
    } else if (status === 'running') {
        out.innerHTML = '<span class="output-hint">Running...</span>';
        statusEl.textContent = 'running';
        statusEl.className = 'output-status running';
    } else {
        out.textContent = text;
        let label = 'success';
        if (elapsed) {
            label += ' (' + elapsed + 's)';
        }
        statusEl.textContent = label;
        statusEl.className = 'output-status ok';
    }
}

function escapeHtml(s) {
    return s.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
}

// ─── Event listeners ─────────────────────────────────────────────────────────
document.addEventListener('DOMContentLoaded', () => {
    initEditor('');
    document.getElementById('currentFile').textContent = 'main.spp';

    // Keyboard navigation for example list
    const exampleList = document.getElementById('exampleList');
    const items = exampleList.querySelectorAll('.example-item');
    items.forEach((item, index) => {
        item.setAttribute('tabindex', '0');
        item.addEventListener('keydown', (e) => {
            if (e.key === 'Enter' || e.key === ' ') {
                e.preventDefault();
                currentExample = item.dataset.example;
                loadExample(item.dataset.example);
            }
        });
    });

    document.getElementById('exampleList').addEventListener('click', (e) => {
        const item = e.target.closest('.example-item');
        if (item) {
            currentExample = item.dataset.example;
            loadExample(item.dataset.example);
        }
    });

    // Keyboard shortcut hint
    const runBtn = document.getElementById('btnRun');
    runBtn.title = 'Run (Ctrl+Enter)';
    const copyBtn = document.getElementById('btnCopy');
    copyBtn.setAttribute('aria-label', 'Copy code to clipboard');
    const clearBtn = document.getElementById('btnClear');
    clearBtn.setAttribute('aria-label', 'Clear output');
    const shareBtn = document.getElementById('btnShare');
    shareBtn.setAttribute('aria-label', 'Share code');

    // Hamburger menu
    const hamburger = document.getElementById('hamburger');
    const navCenter = document.getElementById('navCenter');
    if (hamburger && navCenter) {
        hamburger.addEventListener('click', () => {
            hamburger.classList.toggle('active');
            navCenter.classList.toggle('open');
        });
        navCenter.querySelectorAll('a').forEach(link => {
            link.addEventListener('click', () => {
                hamburger.classList.remove('active');
                navCenter.classList.remove('open');
            });
        });
    }

    document.getElementById('btnRun').addEventListener('click', runCode);

    document.getElementById('btnCopy').addEventListener('click', () => {
        navigator.clipboard.writeText(editor.getValue()).then(() => {
            const btn = document.getElementById('btnCopy');
            btn.textContent = '✓ Copied';
            setTimeout(() => { btn.textContent = '⎘ Copy'; }, 1500);
        });
    });

    document.getElementById('btnClear').addEventListener('click', () => {
        setOutput('', '');
    });

    document.getElementById('btnShare').addEventListener('click', () => {
        const code = editor.getValue();
        const url = window.location.origin + window.location.pathname + '?code=' + encodeURIComponent(code);
        if (navigator.share) {
            navigator.share({ title: 'Sapphire Playground', text: code, url: url });
        } else {
            navigator.clipboard.writeText(url).then(() => {
                const btn = document.getElementById('btnShare');
                btn.textContent = '✓ Copied';
                setTimeout(() => { btn.textContent = '🔗 Share'; }, 1500);
            });
        }
    });
});
