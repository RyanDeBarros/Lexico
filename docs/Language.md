## General Syntax

A Lexico is made of a whitespace-independent sequence lines called *statements*. A statement has the following form:
```
statement := <instruction> <operand>
operand   := <expression|keyword>
```

### Comments
Single-line unbound comments are written starting with the `#` character. For example:
```
var age = 5  # this is a comment
```

### Variables
Variables can have any name with characters from a-z, A-Z, 0-9, or the underscore character. They must always be prefixed with a `$` character.

### Functions
Functions can have any name with characters from a-z, A-Z, 0-9, or the underscore character. They must be defined with such a name, and invoked by prefixing with a `!` character and suffixing with `(<arglist>)`. If there are one or no arguments, the parentheses can be omitted.

### Arglist

## Instructions

| Instruction | Operand             | Block | Description                                                                                                                                                                                                        |
|-------------|---------------------|-------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `var`       | `<expression>`      | no    | Creates or replaces a variable. The expression must be some kind of assignment operation.<br/>If the variable already exists in an outer scope, this will create a local variable that shadows the outer variable. |
| `fn`        | `name(<arglist>)`   | yes   | Defines a new function.                                                                                                                                                                                            |
| `eval`      | `<expression>`      | no    | Evaluates the expression. If the expression assigns to a variable that does not exist, it is created, but will not shadow an already existing outer variable.                                                      |
| `end`       | `<keyword>`         | no    | Closes a block (scope). Opening and closing blocks must follow LIFO ordering.                                                                                                                                      |
| `push`      | `<expression>`      | no    | Pushes a named (variable) or unnamed (pure expression) page to the page stack.                                                                                                                                     |
| `pop`       |                     | no    | Pops a page from the page stack.                                                                                                                                                                                   |
| `with`      | `<expression>`      | yes   | Pushes a page to the page stack and pops it when the block is closed.                                                                                                                                              |
| `if`        | `<bool-expression>` | yes   | Branching block that executes if the expression evaluates to `true`.                                                                                                                                               |
| `elif`      | `<bool-expression>` | yes   | Branching block that executes if the expression evaluates to `true`, and no prior `if`/`elif` block executed.                                                                                                      |
| `else`      |                     | yes   | Branching block that executes if not prior `if`/`elif` block executed.                                                                                                                                             |
| `while`     | `<bool-expression>` | yes   | Branching block that executes in a loop as long as the expression evaluates to `true`.                                                                                                                             |

### Lexer expansion
If a statement begins with a `!` or `$`, then an implicit `eval` instruction is prepended to the line.

## Built-in functions

### `!Log(args...).void`
Prints to the log output

### `!ChangeCase(case.Case).void`
Changes the case of the page.

### `!ChangeCase(case.Case, in <string-expression>).str`
Changes the case of the passed in string and returns it.

### `!ChangeCase(case.Case, ref string.str).void`
Changes the case of the passed in string in-place.

## Enumerations

### Case

| `Case`  | Description                         |
|---------|-------------------------------------|
| `Upper` | Changes all characters to uppercase |
| `Lower` | Changes all characters to lowercase |


## Typecasting
All variables have a built-in explicit typecast method, that is invoked using:
```
$<varname>.<type>
```

For example:
```
$age.int
$name.str
$characters.list
```

### Tuples
Tuple can be created out of an arglist:
```
([[id]: in|out|ref arg,]...).tup
```
Given a tuple, the elements can be referenced by unique ids or indexes:
```
var $age  = 10
var $name = "Alice"

$t = (ref $age.int, name: in $name.str).tup
$t[0]   = 15
$t.name = "Bob"

!Log $age     # logs 15
!Log $t[0]    # logs 15
!Log $name    # logs "Alice"
!Log $t.name  # logs "Bob"
```
Tuples can be unpacked into an arglist as well:
```
$t.args(out $age2, ref $name2)
```

### Built-in Datatypes

| Datatype | Literal      | Description                           |
|----------|--------------|---------------------------------------|
| `void`   |              | only used for return type enforcement |
| `int`    | any integer  | Integer                               |
| `mark`   | `<integer>m` | Marker (explained further below)      |
| `Case`   | enum         | [Character Case](#Case)               |

### Function signatures
Functions can enforce arguments having certain types using:
```
fn MyFunc(a, b.int, c.str)
```

When passing arguments to this function, one must do:
```
!MyFunc(a, b.int, c.str)
```

They can also enforce return types:
```
fn Calculate(x.float, y.float).int
```

Then one can do:
```
$a = !Calculate(x1.float, y1.float)
$b.int = !Calculate(x2.float, y2.float).int
```
