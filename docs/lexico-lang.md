# General

`lexico` is an expressive regex-centric scripting language. It is interpreted, strongly typed, and strictly typed.

* A script consists of sequences of statements, mostly one per line.
* Extra whitespace within a line is ignored.
* All names (variables, functions, etc.) may use any alphanumeric/underscore characters, but may not start with a digit.
* Comments are supported, and begin with a `;` character.

## Pattern building

### Introduction

To create a new pattern:

```
pattern alphanumeric
```

`alphanumeric` is now implicitly used by the state machine.

```
pattern tripleDigit
pattern alphanumeric
```

This creates a new pattern, `tripleDigit`, and switches back to `alphanumeric` without resetting it. To delete a pattern before switching instead, use:

```
delete alphanumeric
pattern alphanumeric
```

### Appending

To add new elements to a pattern, use `append`:

```
pattern cat
append "c"
append "a"
append "t"

pattern dog
append "dog"
```

The operand for `append` must be a string or a pattern. You can append multiple patterns at once with:

```
pattern full
append start, end
```

### Disjunction

Use `or` to allow for multiple patterns. When using capture groups, priority is given to sub-patterns in left-to-right order:

```
pattern pet
append "cat" or "dog" or "bird"
```

If a pattern is optional, use `optional`:

```
pattern squirrel
append "s" or "S"
append "quirrel"
append optional "s"
```

Note that `optional <pattern>` is equivalent to `<pattern> or ""`.

### Ranges

Use `to` to simplify letter and number ranges:

```
pattern bigDigit
append "6" to "9"

pattern ABC
append "A" to "C"
```

### Built-in shorthand

There are predefined `$`-prefixed patterns. These cannot be redefined, but are effectively defined in the following way:

```
pattern $digit
append "0" to "9"

pattern $lowercase
append "a" to "z"

pattern $uppercase
append "A" to "Z"

pattern $alphanumeric
append $digit or $lowercase or $uppercase

pattern $varname
append $alphanumeric or "_"

pattern $space
append " " or "\t" or "\n" or "\r" or "\v"

pattern $any
; no script equivalent - represents any character, like regex `.`
```

### Negation

To not allow a pattern, use `not`:

```
pattern singularCat
append "cat"
append not "s"
```

You can also negate subpatterns with `except`:

```
pattern prime
append "2" or "3" or "5" or "7"

pattern onlyComposite
append $digit except prime
```

### Repetition

To repeat a pattern multiple times use `repeat`:

```
pattern phoneNumber
append $digit repeat 3, "-", $digit repeat 3, "-", $digit repeat 4

pattern address
append $digit repeat 1 to 5

pattern atLeastOneA
append "a" repeat min 1

pattern anyB
append "b" repeat min 0

pattern noMoreThanFiveC
append "c" repeat max 5
```

The following syntaxes are defined:

| Syntax | Description |
| - | - |
| `<pattern> repeat X` | `<pattern>` repeated exactly `X` times |
| `<pattern> repeat X to Y` | `<pattern>` repeated any number from `X` to `Y` times |
| `<pattern> repeat min X` | `<pattern>` repeated at least `X` times |
| `<pattern> repeat max X` | `<pattern>` repeated at most `X` times |

In real regex, `+` and `*` are used so often that lexico also has a shorthand:

```
pattern atLeastOneA
append "a"+

pattern anyB
append "b"*
```

Also, note that `repeat 0 to 1` is equivalent to prefixing with `optional`.

### Capture groups

Capture groups are named subpatterns that can be referenced in matches. Define a capture group within a pattern as so:

```
pattern ageField
append "age", "="

pattern age
append $digit repeat 1 to 3

pattern ageField
append capture age
```

### Pattern order of operations

Grouping with parentheses forces order of operations, and each `append` statement is self-contained.
In other words, operations don't carry over to multiple `append` statemenets.
Otherwise a full breakdown is listed below (with equivalent precedence read left to right):

| Precedence | Operation | Position |
| - | - | - |
| 0 | () | - |
| 1 | `to` | middle - only defined between literals |
| 2 | `not` | prefix |
| 3 | `+`<br>`*`<br>`except`<br>`repeat` | postfix |
| 4 | `optional` | prefix |
| 5 | `or` | middle |
| 6 | `,` | middle |
| 7 | `capture` | prefix |
| 8 | `append` | - |

For example:

```
pattern prime
append "2" or "3" or "5" or "7"

pattern example
append capture ("d" to "f" * or optional $digit except prime repeat min 3, not "z" +) repeat 2, "()"
```

This is equivalent to (precedence from right to left):

```
pattern _1
append "d" to "f"

pattern _2
append _1*

pattern _3
append $digit except prime

pattern _4
append _3 repeat min 3

pattern _5
append optional _4

pattern _6
append _2 or _5

pattern _7
append not "z"

pattern _8
append _7+

pattern _9
append _6
append _8

pattern _10
append _9 repeat 2

pattern _11
append _10
append "()"

pattern result
append capture _11
```

## Match querying

### Pages

Lexico uses a page stack for match queries. The default (base) page is the program's input string. Push a new page with:

```
page push <string> 
```

You can then pop it with:

```
page pop
```

To save the current page state, you can use the built-in `$page` string variable:

```
let saveState = $page
```

### Match creation

Lexico also uses a global state for match queries, stored in the built-in `$matches` matches variable. To re-generate it, use:

```
find <pattern>
let matches_ = $matches
```

To get a specific match, use:

```
let firstMatch = matches[0]
```

To restore `$matches` to a previous state, simply reassign it:

```
$matches = matches_
```

### Highlighting

Like other match operations, highlighting works on the global `$matches` object.
To highlight all matches in the GUI, simply use:

```
highlight
```

You can specify a specific matches or match object to highlight:

```
find oldPattern
let matches = $matches

find newPattern

highlight matches_
highlight matches_[0]
highlight 0  ; uses new $matches[0]
```

You can also specify a color with:

```
highlight 3 color Red
```

In version 1.0, the color operand must be a predefined color from these 8 symbols:
* `Yellow` (default)
* `Red`
* `Green`
* `Blue`
* `Grey`/`Gray`
* `Purple`
* `Orange`
* `Mono` (black in light mode, white in dark mode)

To remove a pattern from highlighting, use:

```
highlight delete match_ color Red
```

Or simply clear all highlighting with:

```
highlight delete color Red
```

### Filtering

Filtering removes match objects directly from `$matches`, using a predicate function.

```
fn AtLeastLength(match m) -> bool
  return m.len > 10
end fn

find $alphanumeric+
filter AtLeastLength
highlight

let x = 10
fn LessThan(match m) -> bool
  return m as int < x
end fn

find $digit+
filter LessThan
highlight color Red

x = 5
filter LessThan
highlight color Blue
```

### Replacing

## Keywords

### `let`

Creates a new variable.

```
let age = 42
age = age + 1  ; no need to use `let` again
```

### `if`/`elif`/`else`

Conditional branching blocks.

```
if myCond
  ; do something
elif myOtherCond
  ; do something else
else
  ; do something different
end if
```

### `while`

Conditional looping block.

```
let i = 0
while i < 10
  log i
  i = i + 1
end while
```

## Output

You can log data to the log output with:

```
log "Hello World!"
log "Length of first element = ", len(%[0])
```

# Regex examples

# Extended Backus-Naur Form

## Top-level

```
program   ::= { statement } ;
statement ::= pattern_stmt
            | delete_stmt
            | append_stmt ;

pattern_stmt ::= "pattern" identifier ;
delete_stmt  ::= "delete" identifier ;
```

## Append statement

```
append_stmt ::= "append" expression ;
expression  ::= or_expr { "," or_expr } ;
or_expr     ::= except_expr { "or" except_expr } ;
except_expr ::= repeat_expr { "except" repeat_expr } ;
repeat_expr ::= unary_expr { repetition } ;
```

### Repetition
```
repetition  ::= "+" | "*" | "repeat" repeat_spec ;
repeat_spec ::= number | number "to" number | "min" number | "max" number ;
```

### Unary expression
```
unary_expr ::= [ prefix_op ] primary ;
prefix_op  ::= "not" | "optional" | "capture" ;
```

### Primary
```
primary    ::= range_expr | identifier | builtin | group ;
group      ::= "(" expression ")" ;
range_expr ::= string | string "to" string ;
```

## Terminals
```
identifier ::= letter { letter | digit | "_" } ;
builtin    ::= "$" identifier ;
string     ::= '"' { character } '"' ;
number     ::= digit { digit } ;
```
