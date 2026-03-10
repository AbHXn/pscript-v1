# Pidi Programming Language

Pidi is a lightweight experimental programming language designed for learning and building interpreters or compilers.

The language focuses on **simplicity, readability, and dynamic behavior**, making it useful for experimenting with programming language design concepts.

---

# Features

- Dynamic typing
- Dynamic arrays
- Functions and closures
- Conditional statements
- Loop constructs
- ASCII operations
- String operations
- Comment system
- Interpreter execution

---

# Hello World

```pidi
para "hello world!";
```

Output

```
hello world!
```

---

# Installation

Currently Pidi is compiled using C++.

### Build

```
g++ main.cpp -o pidi
```

### Run a Program

```
./pidi program.pidi
```

---

# Printing Output

The `para` keyword prints output to the console.

### Syntax

```pidi
para expression;
```

### Example

```pidi
para "hello world!";
```

---

# String Concatenation

Strings can be joined using the `koode` operator.

```pidi
para "hello" koode " world!";
```

Output

```
hello world!
```

⚠ Concatenation works only inside `para`.

---

# Comments

Comments use the `?` symbol.

### Single Comment

```pidi
? this is a comment ?
```

### Multi-line Comment

```pidi
??? multi line comment ???
```

Rules

- Odd number of `?` works
- Even number does not work

---

# Variables

Variables are declared using `pidi`.

### Syntax

```pidi
pidi variable = value;
```

### Example

```pidi
pidi num1 = 23;
pidi num2 = 33.34;
pidi name = "admin";
```

Variables are dynamically typed.

---

# Boolean Values

```
sheri  -> true
thettu -> false
```

Example

```pidi
pidi status = sheri;
```

---

# ASCII Operations

### Character to ASCII

```pidi
para name[0]:THA_ASCII;
```

Example

```pidi
pidi name = "admin";
para name[0]:THA_ASCII;
```

Output

```
97
```

---

### ASCII to Character

```pidi
para num:PO_ASCII;
```

Example

```pidi
pidi num = 65;
para num:PO_ASCII;
```

Output

```
A
```

---

# Length Operator

The `kanam` operator returns the length of a string or array.

```pidi
pidi name = "admin";
para name:kanam;
```

Output

```
5
```

---

# Data Type Information

### Print datatype

```pidi
para num:jaadi;
```

Example

```pidi
pidi num = 10;
para num:jaadi;
```

---

# Arrays

Arrays are declared using `kootam`.

Arrays are dynamic and can store multiple data types.

### Syntax

```pidi
pidi arr kootam = {values};
```

### Example

```pidi
pidi arr kootam = {1, 2, 3, "test", {1,2}, 43.34};
```

---

# Display Array

```pidi
para arr:valupam;
```

Example

```pidi
pidi arr kootam = {1,2,3};
para arr:valupam;
```

Output

```
[1,2,3]
```

---

# Array Indexing

```pidi
para arr[index];
```

Example

```pidi
pidi arr kootam = {10,20,30};
para arr[0];
```

Output

```
10
```

---

# Conditional Statements

Conditionals use the `nok` keyword.

### Syntax

```pidi
nok condition {
    statements
}: umbi {
    statements
};
```

### Example

```pidi
nok 5 > 2 {
    para "true";
}: umbi {
    para "false";
};
```

---

# Logical Operators

```
um -> AND
yo -> OR
```

Example

```pidi
nok 5 > 2 um 10 > 3 {
    para "valid condition";
};
```

---

# If Else If Ladder

```pidi
pidi num = 15;

nok num < 10 {
    para "small";
}:nok num < 20 {
    para "medium";
}: umbi {
    para "large";
};
```

---

# Loops

Loops use `ittuthiri`.

### Syntax

```pidi
ittuthiri condition {
    statements
}
```

### Example

```pidi
pidi i = 1;

ittuthiri i <= 5 {

    para i;

    i = i + 1;
}
```

Output

```
1
2
3
4
5
```

---

# Loop Control

### Continue

```
pinnava
```

### Break

```
thekku
```

---

# Functions

Functions are declared using `pari`.

### Syntax

```pidi
pari function_name(parameters){
    statements
}
```

---

# Function Parameters

Parameters require `pidi`.

```pidi
pari square(pidi x){
    poda x * x;
}
```

Call

```pidi
para square(5);
```

Output

```
25
```

---

# Array Parameters

```pidi
pari printArray(pidi arr kootam){

    pidi i = 0;

    ittuthiri i < arr:kanam {

        para arr[i];

        i = i + 1;
    }
}
```

---

# Multiple Parameters

```pidi
pari add(pidi a, pidi b){
    poda a + b;
}

para add(5,6);
```

Output

```
11
```

---

# Return Statement

Return uses `poda`.

```pidi
pari test(){
    poda 23;
}
```

---

# Scope Rules

Variables defined inside a function can be returned.

### Valid

```pidi
pari func(){

    pidi num = 23;

    poda num;
}
```

### Invalid

```pidi
pari func(){

    nok sheri {

        pidi num = 23;

        poda num;
    };
}
```

---

# Returning Functions

Functions can return other functions.

```pidi
pari outer(){

    pari inner(){
        poda 10;
    }

    poda inner;
}
```

---

# Example Algorithms

## Factorial

```pidi
pari factorial(pidi n){

    pidi result = 1;
    pidi i = 1;

    ittuthiri i <= n {

        result = result * i;
        i = i + 1;
    }

    poda result;
}

para factorial(5);
```

Output

```
120
```

---

## Linear Search

```pidi
pari search(pidi arr kootam, pidi key){

    pidi i = 0;

    ittuthiri i < arr:kanam {

        nok arr[i] == key {
            poda i;
        };

        i = i + 1;
    }

    poda -1;
}
```

---

## Fibonacci

```pidi
pari fib(pidi n){

    nok n <= 1 {
        poda n;
    };

    poda fib(n-1) + fib(n-2);
}

para fib(6);
```

Output

```
8
```

---

# Language Summary

| Feature | Supported |
|------|------|
| Dynamic Typing | ✓ |
| Arrays | ✓ |
| Functions | ✓ |
| Closures | ✓ |
| Loops | ✓ |
| Conditional Statements | ✓ |
| ASCII Operations | ✓ |

---

# Project Status

Pidi is currently an experimental programming language and is under active development.

---

# License

MIT License
