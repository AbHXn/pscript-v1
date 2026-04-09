# PScript Programming Language

PScript is a lightweight experimental malayalam based programming language designed for ........... 

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
- Interpreter-based execution

---

---

# Installation

Currently PScript is compiled using **C++**.

### Build

```
git clone https://github.com/AbHXn/pscript-v1

cd pscript-v1

make
```

### Run a Program

```
./pscript program.ps
```

---

# Printing Output

The `para` keyword prints output to the console.

### Syntax

```pscript
para expression;
```

### Example

```pscript
para "hello world!\n";
```

---

# String Concatenation

Strings can be concatenated using the `koode` operator.

```pscript
para "hello" koode "world!\n";
```

Output

```
hello world!
```

⚠ Concatenation works only inside `para`.

---

# Comments

Comments use the `?` symbol.

### Comment

```pscript
? this is a comment ?

??? this is also a valid comment ???
```

Rules:

- Odd number of `?` works
- Even number of `?` does not work

---

# Variables

Variables are declared using `pidi`.

### Syntax

```pscript
pidi variable = value;
```

### Example

```pscript
pidi num1 = 23;
pidi num2 = 33.34;
pidi name = "admin";
```

## Inputing

```pscript
para "Enter name: ";
pidi name = edukku;
```

Output

```
Enter name: admin
```

# Type casting

```pscript
var := INT;
var := STR;
var := THULA; ? floating point ?
var := BOOL;

```

Variables are dynamically typed.

---

# Boolean Values

```
sheri  -> true
thettu -> false
```

Example

```pscript
pidi flag = sheri;
```

---

# ASCII Operations

### Character to ASCII

```pscript
para name[0]:THA_ASCII;
```

Example

```pscript
pidi name = "admin";
para name[0]:THA_ASCII;
```

Output

```
97
```

---

### ASCII to Character

```pscript
para num:PO_ASCII;
```

Example

```pscript
pidi num = 65;
para num:PO_ASCII;
```

Output

```
A
```

---

# Length Operator

The `kanam` operator returns the size of variable.

```pscript
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

```pscript
para num:jaadi;
```

Example

```pscript
pidi num = 23;
para num:jaadi;
```

---

# Arrays

Arrays are created using `kootam`.

Arrays are dynamic and can store multiple data types.

### Syntax

```pscript
pidi arr kootam = {values};
```

### Example

```pscript
pidi arr kootam = {1, 2, 3, "test", {1,2}, 43.34};
```

---

# Size of an array

```pscript
para arr:valupam;
```

Example

```pscript
pidi arr kootam = {1,2,3};
para arr:valupam;
```

Output

```
3
```

---

# Array Indexing

```pscript
para arr[index];
```

Example

```pscript
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

```pscript
nok condition {
    statements
}: umbi {
    statements
};
```

### Example

```pscript
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

```pscript
nok 5 > 2 um 10 > 3 {
    para "valid condition";
};
```

---

# If Else If Ladder

```pscript
pidi num = 23;

nok 1 < num um 10 > num {
    para "between 1 and 10";
}:nok 10 < num um 20 > num {
    para "between 10 and 20";
}: umbi {
    para "greater number";
};
```

---

# Loops

Loops use `ittuthiri`.

### Syntax

```pscript
ittuthiri condition {
    statements
}
```

### Example

```pscript
pidi i = 1;

ittuthiri i <= 5 {
    para i koode "\n";
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
theku
```

---

# Functions

Functions are declared using `pindi`.

### Syntax

```pscript
pindi function_name(parameters){
    statements
}
```

---

# Function Parameters

Parameters must use `pidi`.

```pscript
pindi square(pidi x){
    poda x * x;
}
```

Call

```pscript
para square(5);
```

Output

```
25
```

---

# Scope Rules

Variables defined inside a function can be returned **only if they are declared in the function’s main body**.

---

## ❌ Invalid Example

```pscript
pindi testfunc(){
    nok condition {
        pidi data = 434;
        poda data;   ? INVALID: data is inside nok block ?
    };
}
```

## Valid Example

```pscript
pindi testfunc(){
    pidi data = 434;
    nok condition {
        poda data;   ? VALID: data is inside pindi block ?
    };
}
```


---

# Array Parameters

```pscript
pindi printArray(pidi arr kootam){
    pidi i = 0;
    ittuthiri i < arr:valupam {
        para arr[i] koode "\n";
        i = i + 1;
    }
}
pidi arr kootam = {1, 2, 3};
printArray(arr);
```

---

# Multiple Parameters

```pscript
pindi add(pidi a, pidi b){
    poda a + b;
}

para add(4,6) koode "\n";
```

Output

```
10
```

---

# Return Statement

Return uses `poda`.

```pscript
pindi test(){
    poda 23;
}
```

---

# Scope Rules

Variables defined inside a function can be returned.

### Valid

```pscript
pindi func(){
    pidi num = 23;
    poda num;
}
```

### Invalid

```pscript
pindi func(){
    nok sheri {
        pidi num = 23;
        poda num;
    };
}
```

---

# Returning Functions

Functions can return other functions.

```pscript
pindi outer(){
    pindi inner(){
        poda 10;
    }
    poda inner;
}
```

---

# Example Algorithms

## Factorial

```pscript
pindi factorial(pidi n){
    pidi result = 1;
    pidi i = 1;

    ittuthiri i <= n {
        result = result * i;
        i = i + 1;
    }
    poda result;
}
para factorial(5) koode "\n";
```

Output

```
120
```

---

## Linear Search

```pscript
pindi search(pidi arr kootam, pidi key){
    pidi i = 0;

    ittuthiri i < arr:valupam {
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

```pscript
pindi fib(pidi n){
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

## Simple password analyzer

```pscript
? user database ?
pidi users kootam = {"alice","bob","charlie","david","eve"};
pidi passwords kootam = {"abc123","pass","complex99","hello777","weak"};

para "User Database Loaded" koode "\n";
para users koode "\n";

? count digits in string ?
pindi countDigits(pidi text){
    pidi i = 0;
    pidi count = 0;

    ittuthiri i < text:kanam {
        nok text[i]:THA_ASCII >= 48 um text[i]:THA_ASCII <= 57 {
            count = count + 1;
        };
        i = i + 1;
    }
    poda count;
}


? count letters ?
pindi countLetters(pidi text){
    pidi i, count = 0, 0;

    ittuthiri i < text:kanam {
        nok text[i]:THA_ASCII >= 97 um text[i]:THA_ASCII <= 122 {
            count = count + 1;
        };
        i = i + 1;
    }
    poda count;
}


? password strength check ?
pindi passwordStrength(pidi pass){
    pidi digits = countDigits(pass);
    pidi letters = countLetters(pass);

    nok digits >= 2 um letters >= 4 {
        poda "STRONG";
    }
    :nok digits >= 1 {
        poda "MEDIUM";
    }
    :umbi{
        poda "WEAK";
    };

}

? analyze users ?

para "\nPassword Analysis" koode "\n";
pidi i = 0;

ittuthiri i < users:valupam {
    para "User:" koode "\n";
    para users[i] koode "\n";

    para "Password:" koode "\n";
    para passwords[i] koode "\n";

    para "Strength:" koode "\n";
    para passwordStrength(passwords[i]) koode "\n";

    para "Digits:" koode "\n";
    para countDigits(passwords[i]) koode "\n";

    para "Letters:" koode "\n";
    para countLetters(passwords[i]) koode "\n";

    para "------" koode "\n";

    i = i + 1;
}


? system statistics ?

para "\nSystem Statistics" koode "\n";

pidi strong 	= 0;
pidi weak 		= 0;
pidi j 			= 0;

ittuthiri j < passwords:valupam {
    nok passwordStrength(passwords[j]) == "STRONG" {
        strong = strong + 1;
    }
    :umbi{
        weak = weak + 1;
    };

    j = j + 1;
}

para "Strong Passwords:" koode "\n";
para strong koode "\n";

para "Weak/Medium Passwords:" koode "\n";
para weak koode "\n";

```

Output

```
User Database Loaded 
[alice, bob, charlie, david, eve] 

Password Analysis 
User: 
alice 
Password: 
abc123 
Strength: 
MEDIUM 
Digits: 
3 
Letters: 
3 
------ 
User: 
bob 
Password: 
pass 
Strength: 
WEAK 
Digits: 
0 
Letters: 
4 
------ 
User: 
charlie 
Password: 
complex99 
Strength: 
STRONG 
Digits: 
2 
Letters: 
7 
------ 
User: 
david 
Password: 
hello777 
Strength: 
STRONG 
Digits: 
3 
Letters: 
5 
------ 
User: 
eve 
Password: 
weak 
Strength: 
WEAK 
Digits: 
0 
Letters: 
4 
------ 

System Statistics 
Strong Passwords: 
2 
Weak/Medium Passwords: 
3 

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

PScript is currently an experimental programming language and under active development.

---

# License

MIT License
