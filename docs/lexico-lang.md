# General

`lexico` is an expressive regex-centric scripting language. It is interpreted, strongly typed, and strictly typed.

* A script consists of sequences of statements, mostly one per line.
* Extra whitespace within a line is ignored.
* All names (variables, functions, etc.) may use any alphanumeric/underscore characters, but may not start with a digit.
* Comments are supported, and begin with a `#` character.

TODO aggregation / SQL constructs

TODO recursion built into patterns (see parentheses balancing example)

## Data types

### `capid`

### `cap`

| Attribute | Type | Description |
| - | - |
| `exists` | `bool` | is `true` if the group was captured, `false` otherwise |
| `start` | `int` | starting position |
| `end` | `int` | ending position (one after last character) |
| `len` | `int` | length of group |
| `pos` | `irange` | position range of group |
| `str` | `string` | string of characters captured |
| `sub` | `match` | characters captured as a submatch |

### `match`

| Attribute | Type | Description |
| - | - |
| `caps` | `list` | list of captures |
| `[]` | `cap` | returns a named capture |
| `start` | `int` | starting position |
| `end` | `int` | ending position (one after last character) |
| `len` | `int` | length of group |
| `pos` | `irange` | position range of group |
| `str` | `string` | character subtext as a string |

### `irange`

Can be constructed from the following forms:

```
<int>          # specific index
<int> to <int> # inclusive min/max
min <int>      # inclusive min, unbounded max
max <int>      # 0 to inclusive max, shorthand for 0 to <int>
```

### `srange`

Can be constructed from the following forms:

```
<char>           # specific character
<char> to <char> # inclusive min/max
min <char>       # inclusive lowercase min to "z", or inclusive uppercase min to "Z"
max <char>       # "a" to inclusive lowercase max, or "A" to inclusive uppercase max
```

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

The operand for `append` must be a string or a pattern. You can cast other data types if possible:

```
pattern one
append 1 as string

pattern two
append 2 as pattern
```

You can also append multiple patterns at once with:

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

pattern $letter
append $lowercase or $uppercase

pattern $alphanumeric
append $digit or $letter

pattern $varname
append $alphanumeric or "_"

pattern $space
append " " or "\t" or "\n" or "\r" or "\v"

pattern $newline
append "\r\n" or "\n" or "\r"

pattern $start
# no script equivalent - but equivalent to `^` in regex

pattern $end
# no script equivalent - but equivalent to `$` in regex

pattern $any
# no script equivalent - represents any character, like regex `.`

pattern $cap
# no script equivalent - represents an empty character, used purely for marking capture groups
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

Capture groups are named subpatterns that can be referenced in matches.
If no name is provided, they'll need to be referenced in matches by index.
Define a capture group within a pattern as so:

```
pattern ageField
append "age", "="

pattern value
append $digit repeat 1 to 3

pattern ageField
append capture age value  # named

pattern weightField
append "weight", "="
append capture _ value  # unnamed
```

Note that the data type of the capture group name is `capid`.

### Back-referencing

You can force groups within a pattern to be the same using back-referencing:

```
pattern repeatedWord
append capture word $alphanumeric+
append $space+
append ref word
```

The referred `capid` must be captured explicitly exactly once somewhere in the pattern, and may be re-captured
for referencing in match objects.

### Lookarounds

You can search for non-consuming patterns using lookarounds:

```
pattern cats
append "cat"
append ahead "s"

pattern cat
append "cat"
append not ahead $space

pattern hotdog
append before "hot"
append "dog"

pattern dog
append not before "hot"
append "dog"
```

### Lazy searching
By default, patterns use greedy searches, but subpatterns can be marked explicitly lazy:

```
pattern quote
append "\"", lazy $any*, "\""
```

### Pattern order of operations

Grouping with `()` parentheses forces order of operations, and each `append` statement is self-contained.
In other words, operations don't carry over to multiple `append` statemenets.
Otherwise a full breakdown is listed below (with equivalent precedence executed inside to outside).

| Precedence | Operation | Type |
| - | - | - |
| 1 | `as`<br>`+`<br>`*`<br>`repeat` | Postfix |
| 2 | `not`<br>`optional`<br>`ahead`<br>`not ahead`<br>`behind`<br>`not behind`<br>`ref` | Prefix |
| 3 | `except` | Binary |
| 4 | `or` | Binary |
| 5 | `,` | Binary |
| 6 | `lazy`<br>`capture` | Wrapper |

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

You can pop all with:

```
page delete
```

To save the current page state, you can use the built-in `$page` string variable:

```
let saveState = $page
```

### Match creation

Lexico also uses a global state for match queries, stored in the built-in `%` matches variable. To re-generate it, use:

```
find <pattern>
let matches_ = %
```

To get a specific match, use:

```
let firstMatch = matches[0]
```

To restore `%` to a previous state, simply reassign it:

```
% = matches_
```

### Highlighting

Like other match operations, highlighting works on the global `%` object.
To highlight all matches in the GUI, simply use:

```
highlight
```

You can specify a specific matches or match object to highlight:

```
find oldPattern
let matches_ = %

find newPattern

highlight matches_
highlight matches_[0]
highlight 0  # uses new %[0]
highlight 1 to 5
highlight -1 to -3  # use reverse indexing to highlight last three
```

You can also specify a color with:

```
highlight 3 color red
```

In version 1.0, the color operand must be a predefined color from these 8 symbols:
* `yellow` (default)
* `red`
* `green`
* `blue`
* `grey`/`gray`
* `purple`
* `orange`
* `mono` (black in light mode, white in dark mode)

To remove a pattern from highlighting, use:

```
highlight delete match_ color red
```

Or simply clear all highlighting with:

```
highlight delete color red
```

### Filtering

Filtering removes match objects directly from `%`, using a predicate function.

```
fn AtLeastLength(match m) -> bool
  return m.str.len > 10
end fn

find $alphanumeric+
filter AtLeastLength
highlight

var x = 10
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

### Capture groups

You can query a capture object via:

```
pattern ageField
append "age", "="
append capture age $digit repeat 1 to 3

fn isAdult(match m) -> bool
  return m[age] >= 18
end fn

find ageField
filter isAdult
```

You can also query by index:

```
pattern addition
append capture _ $digit+

pattern adder
append $space*, "+", $space*
append capture _ $digit+

pattern addition
append adder*

find addition

for m in %
  let sum = 0
  for i in 0 to m.caps.len - 1
    sum += m.caps[i].str as int
  end for
  log m.str, " = ", sum
end for
```

### Replacement

To do match-based replacement, use `replace`:

```
replace <match> with <string>
apply <function(match)->string>
```

For example:

```
pattern name
append capture firstName $any+
append capture middle ", "
append capture lastName $any+

fn switch(match m) -> string
  return m[lastName].str + m[middle].str + m[firstName].str
end fn

find name
apply switch
```

### Scoping

You can limit the searching scope for `find` statements using `scope`:

```
scope $line           # match must fit in single line
scope $lines <range>  # match must fit in line range
```

Note that `scope $line` is equivalent to `scope $lines 1`.

## Computation

### Variables

There are 2 classes of variables: global and local. Global variables can be accessed and mutated in any scope,
including in functions without being passed as a parameter, as long as they are defined before-hand.
Local variables only exist in a scope or its inner block scopes (loops, branches, etc.), but not in different
function scopes.

A global variable is defined using `var`, and can only be declared in the program scope:

```
var threshold = 100
```

A local variable is defined using `let`, and can be declared anywhere:

```
let i = 0
```

### Branching

Lexico supports typical conditional branching blocks.

```
if myCond
  # do something
elif myOtherCond
  # do something else
else
  # do something different
end if
```

For example:

```
pattern phoneNumber
append capture par1 "("
append $digit repeat 3
append capture par2 ")"
append "-", $digit repeat 3, "-", $digit repeat 4

fn validate(match m) -> bool
  let p1 = m[par1].exists
  let p2 = m[par2].exists
  return p1 and p2 or not p1 and not p2
end fn

find phoneNumber
filter validate
```

### Looping

Lexico supports two kinds of loops. The first is a while loop:

```
let i = 0
while i < 10
  log i
  i = i + 1
end while
```

The second is a for loop:

```
for <element> in <iterable>
  # do stuff
end for
```

`<iterable>` here can be any of these objects:
* `matches`: iterate over match objects
* `match`: iterate over capture objects
* `list`: iterate over elements
* `irange`: iterate over inclusive integer range
* `srange`: iterate over inclusive character range

Both loops support standard `break` and `continue` statements.

### Logging

You can log data to the log output with:

```
log "Hello World!"
log "Length of first element = ", len(%[0])
```

## Reserved words

Here is a comprehensive list of reserved words, meaning new identifiers cannot use them:

| Keyword | Description |
| - | - |
| `ahead` |
| `and` |
| `append` |
| `apply` |
| `as` |
| `behind` |
| `bool` |
| `break` |
| `cap` |
| `capid` |
| `capture` |
| `color` |
| `continue` |
| `delete` |
| `elif` |
| `else` |
| `end` |
| `except` |
| `filter` |
| `find` |
| `float` |
| `fn` |
| `highlight` |
| `if` |
| `in` |
| `int` |
| `irange` |
| `lazy` |
| `let` |
| `list` |
| `match` |
| `matches` |
| `max` |
| `min` |
| `mod` |
| `not` |
| `optional` |
| `or` |
| `page` |
| `pattern` |
| `pop` |
| `push` |
| `ref` |
| `repeat` |
| `replace` |
| `scope` |
| `srange` |
| `string` |
| `to` |
| `var` |
| `void` |
| `while` |
| `with` |

### Built-in symbols

Here is a list of built-in symbols:

| Symbol | Description |
| - | - |
| `%` |
| `$alphanumeric` |
| `$any` |
| `$cap` |
| `$digit` |
| `$end` |
| `$letter` |
| `$line` |
| `$lines` |
| `$lowercase` |
| `$newline` |
| `$page` |
| `$space` |
| `$start` |
| `$uppercase` |
| `$varname` |

# Regex examples

### Literal

#### regex
```
cat
```

#### lexico
```
pattern cat
append "cat"
find cat
```

### Alternation

#### regex
```
cat|dog|bird
```

#### lexico
```
pattern pet
append "cat" or "dog" or "bird"
find pet
```

### Repeated word

#### regex
```
(\w+)\s+\1
```

#### lexico
```
pattern repeatedWord
append capture word $alphanumeric+
append $space+
append ref word

find repeatedWord
```

### Date capturing

#### regex
```
(?<year>\d{4})-(?<month>\d{2})-(?<day>\d{2})
```

#### lexico
```
pattern date
append capture year $digit repeat 4
append "-"
append capture month $digit repeat 2
append "-"
append capture day $digit repeat 2

find date
```

### Blank line

#### regex
```
^\s*$
```

#### lexico
```
pattern line
append $start, $space*, $end
scope $line
find line
```

### Line length constraint

#### regex
```
^.{0,80}$
```

#### lexico
```
pattern line
append $start, $any* repeat max 80, $end
scope $line
find line
```

### Email

#### regex
```
[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-z]{2,}
```

#### lexico
TODO standard library with built-in patterns/functions
```
pattern userChar
append $alphanumeric or "." or "_" or "%" or "+" or "-"

pattern domainChar
append $alphanumeric or "." or "-"

pattern email
append userChar+
append "@"
append domainChar+
append "."
append $lowercase repeat min 2

find email
```

### URL

#### regex
```
https?:\/\/(www\.)?[\w\-]+\.\w+
```

#### lexico
```
pattern URL
append "http", optional "s", "://", optional "www."
append ($alphanumeric or "-")+
append ".", $alphanumeric+
```

### Name switching

#### regex
```
find:
(\w+), (\w+)

replace:
$2 $1
```

#### lexico

```
pattern name
append capture _ $alphanumeric+
append ", "
append capture _ $alphanumeric+

fn switch(match m) -> string
  return m.caps[1].str + " " + m.caps[0].str
end fn

find name
apply switch
```

### Reformat date

#### regex
```
find:
(\d{4})-(\d{2})-(\d{2})

replace:
$3/$2/$1
```

#### lexico
```
pattern date
append capture _ $digit repeat 4
append "-"
append capture _ $digit repeat 2
append "-"
append capture _ $digit repeat 2

fn rewrite(match m) -> string
  return m.caps[2].str + "/" + m.caps[1].str + "/" + m.caps[0].str
end fn

find date
apply rewrite
```

### Balanced parentheses

#### regex
DNE

#### lexico
```
pattern group
append capture par1 optional "("
append capture sub $any*
append capture par2 optional ")"

fn balanced(match m) -> bool
  let p1 = m[par1].exists
  let p2 = m[par2].exists
  
  if not p1
    if p2
      return false
    else
      return true
    end if
  elif not p2
    return false
  end if
  
  page push m[sub].str
  let pass = true

  let old = %
  find group
  for m in %
    if not balanced(m)
      pass = false
      break
    end if
  end for
  % = old

  page pop
  return pass
end fn

find group
for m in %
  if not balanced(m)
    log "Unbalanced: ", m.str
  end if
end for
```

# Extended Backus-Naur Form

## Program Structure

```
program     ::= { statement } ;
statement   ::= pattern_decl
              | delete_stmt
              | append_stmt
              | scope_stmt
              | find_stmt
              | filter_stmt
              | replace_stmt
              | apply_stmt
              | page_stmt
              | fn_decl
              | var_decl
              | assignment
              | control_stmt
              | log_stmt
              | highlight_stmt ;
```

## Pattern System

```
pattern_decl  ::= "pattern" identifier ;
append_stmt   ::= "append" pattern_expr ;

pattern_expr  ::= wrapper_expr ;
wrapper_expr  ::= { wrapper_op } binary_expr ;
wrapper_op    ::= "capture" identifier | "lazy" ;

binary_expr   ::= or_expr { "," or_expr } ;
or_expr       ::= except_expr { "or" except_expr } ;
except_expr   ::= prefix_expr { "except" prefix_expr } ;

prefix_expr   ::= { prefix_op } postfix_expr ;
prefix_op     ::= "not" | "optional" | "ahead" | "not" "ahead" | "behind" | "not" "behind" | "ref" ;

postfix_expr  ::= primary_expr { postfix_op } ;
postfix_op    ::= "+" | "*" | repeat_clause ;
repeat_clause ::= "repeat" irange_expr ;

primary_expr  ::= expression | "(" pattern_expr ")" ;
```

## Match System

```
scope_stmt   ::= "scope" builtin_symbol irange_expr ;
find_stmt    ::= "find" identifier ;
filter_stmt  ::= "filter" identifier ;
replace_stmt ::= "replace" match_expr "with" string_expr ;
apply_stmt   ::= "apply" identifier ;
```

## Imperative System

```
block         ::= { statement } ;
fn_decl       ::= "fn" identifier "(" [ param_list ] ")" "->" type_name block [ return_stmt ] "end" "fn" ;
return_stmt   ::= "return" expression ;

param_list    ::= param { "," param } ;
param         ::= data_type identifier ;

var_decl      ::= var_op assignment ;
var_op        ::= "var" | "let" ;
assignment    ::= identifier "=" expression ;

control_stmt  ::= if_stmt | while_stmt | for_stmt | "break" | "continue" ;
if_stmt       ::= "if" expression block { "elif" expression block } [ "else" block ] "end" "if" ;
while_stmt    ::= "while" expression block "end" "while" ;
for_stmt      ::= "for" identifier "in" expression block "end" "for" ;

function_call ::= identifier "(" [ arg_list ] ")" ;
arg_list      ::= expression { "," expression } ;
```

## Expressions

```
irange_expr         ::= int_expr | int_expr "to" int_expr | "min" int_expr | "max" int_expr ;
int_expr            ::= expression
srange_expr         ::= letter_expr | letter_expr "to" letter_expr | "min" letter_expr | "max" letter_expr ;
letter_expr         ::= expression

expression          ::= logic_expr ;
logic_expr          ::= comparison_expr { ("and" | "or") comparison_expr } ;
comparison_expr     ::= additive_expr [ comparison_op additive_expr ] | as_expr ;
comparison_op       ::= "==" | "!=" | "<" | ">" | "<=" | ">=" ;
as_expr             ::= logic_expr "as" type_name ;
additive_expr       ::= multiplicative_expr { ("+" | "-") multiplicative_expr } ;
multiplicative_expr ::= mod_expr { ("*" | "/") mod_expr } ;
mod_expr            ::= base_expr [ "mod" base_expr ] ;
base_expr           ::= literal | identifier | builtin_symbol | function_call | "(" expression ")" ;
```

## Miscallaneous

```
delete_stmt    ::= "delete" identifier ;

page_stmt      ::= "page" page_op ;
page_op        ::= "push" string_expr | "pop" ;

log_stmt       ::= "log" expression { "," expression } ;
highlight_stmt ::= "highlight" [ "delete" ] [ match_expr | irange_expr ] [ "color" color ]
color          ::= "yellow"
                 | "red"
                 | "green"
                 | "blue"
                 | "grey"
                 | "gray"
                 | "purple"
                 | "orange"
                 | "mono" ;
```

## Terminals

```
type_name     ::= "int"
                | "float"
                | "bool"
                | "string"
                | "void"
                | "match"
                | "matches"
                | "capid"
                | "cap"
                | "irange"
                | "srange"
                | "list" ;

literal       ::= int_leteral | float_literal | bool_literal | string_literal ;
bool_literal  ::= "true" | "false" ;
float_literal ::= "integer" "." "integer" ;
```
