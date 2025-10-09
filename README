# **Loh: A Superset of Python**

Welcome to **Loh**, a superset of the Python syntax focused on making Python more concise, intuitive, and enjoyable to read and write. Loh provides symbol-based alternatives for all Python keywords and introduces extra syntactic sugar to reduce boilerplate and improve clarity.

Since Loh is a superset, **all existing Python code is valid Loh code**.


## **Table of Contents**

1. [Keyword Mapping](https://www.google.com/search?q=%23keyword-mapping)

2. [Syntactic Sugar Enhancements](https://www.google.com/search?q=%23syntactic-sugar-enhancements)

- [Implicit None](https://www.google.com/search?q=%23implicit-none)

- [Streamlined Definitions (def, class, type)](https://www.google.com/search?q=%23streamlined-definitions-def-class-type)

- [Postfix Lambdas](https://www.google.com/search?q=%23postfix-lambdas)

- [The Dot (.) Operator](https://www.google.com/search?q=%23the-dot--operator)

- [Simplified for Loops](https://www.google.com/search?q=%23simplified-for-loops)


## **Keyword Mapping**

Loh provides symbolic aliases for Python's keywords. This allows you to write more compact code without sacrificing readability.

|             |            |                 |
| ----------- | ---------- | --------------- |
| **Keyword** | **Python** | **Loh**         |
| True        | True       | ++              |
| False       | False      | --              |
| None        | None       | \~              |
| and         | and        | &&              |
| or          | or         | \|\|            |
| not         | not        | \~\~ or !!!     |
| is          | is         | ===             |
| in          | in         | <\~             |
| if          | if         | ?               |
| else        | else       | ?!              |
| elif        | elif       | ?!?             |
| for         | for        | $               |
| while       | while      | $?              |
| break       | break      | $>>             |
| continue    | continue   | $<<             |
| try         | try        | \~^             |
| except      | except     | ?^              |
| finally     | finally    | ?\*             |
| raise       | raise      | ^^^             |
| assert      | assert     | ^?!             |
| with        | with       | &               |
| as          | as         | =>              |
| import      | import     | <.              |
| from        | from       | .>              |
| del         | del        | =>              |
| return      | return     | ->              |
| yield       | yield      | \~>             |
| async       | async      | %               |
| await       | await      | %               |
| lambda      | lambda     | (params)->...   |
| pass        | pass       | ...             |
| class       | class      | Name:: or Name: |
| def         | def        | (optional)      |
| type        | type       | :               |
| match       | match      | ? ... ==:       |
| case        | case       | (implicit)      |


### **Logical Operators**

Write logical checks with symbols common in other languages.

- **Python**\
  if (x is True and y is not False) or z is None:\
      print("Logic!")

- **Loh**\
  ? (x === ++ && y is not --) || z === \~:\
      print("Logic!")


### **Control Flow (if/else/elif)**

Structure conditional blocks more visually.

- **Python**\
  if score > 90:\
      grade = 'A'\
  elif score > 80:\
      grade = 'B'\
  else:\
      grade = 'C'

- **Loh**\
  ? score > 90:\
      grade = 'A'\
  ?!? score > 80:\
      grade = 'B'\
  ?!:\
      grade = 'C'


### **Exception Handling**

Handle errors with a more symbolic and structured approach.

- **Python**\
  try:\
      result = 10 / 0\
  except ZeroDivisionError as e:\
      print("Error!")\
  finally:\
      print("Done.")

- **Loh**\
  \~^:\
      result = 10 / 0\
  ?^ ZeroDivisionError => e:\
      print("Error!")\
  ?\*:\
      print("Done.")


### **Imports**

Import modules using directional symbols.

- **Python**\
  import math\
  from math import sqrt

- **Loh**\
  <. math\
  .> math <. sqrt


### **Match/Case**

The match statement gets a ? prefix and case becomes implicit.

- **Python**\
  match status:\
      case 400:\
          return "Bad request"\
      case 404:\
          return "Not found"\
      case \_:\
          return "Something else"

- **Loh**\
  ? status ==:\
      400:\
          -> "Bad request"\
      404:\
          -> "Not found"\
      \_:\
          -> "Something else"


## **Syntactic Sugar Enhancements**

Loh introduces several powerful concepts to reduce boilerplate and write more expressive code.


### **Implicit None**

In Loh, None can be expressed by an empty value in specific contexts, dramatically cleaning up code.


#### **1. Variable Assignment**

You can assign a variable to None by leaving the right-hand side empty.

- **Python**: x = None

- **Loh**: x =

\# Loh Example\
a =\
assert a is None\
\
d: float =\
assert d is None


#### **2. Function Arguments & Parameters**

Specify None as a default parameter value or pass it as an argument implicitly.

- **Python**\
  def process\_data(data, config=None):\
      ...\
  process\_data(my\_data, config=None)

- **Loh**\
  def process\_data(data, config=):\
      ...\
  process\_data(my\_data, config=)


#### **3. Comparisons**

Check for is None or is not None by omitting the value.

- **Python**\
  if my\_var is None: ...

- **Loh**\
  if my\_var is: ...\
  \# Or even more concisely\
  ? my\_var ===: ...


#### **4. Data Structures**

Implicit None can be used inside lists, tuples, dictionaries, and function calls.

- **Python**\
  my\_list = \[None, 1, None]\
  my\_dict = {'a': None}\
  foo(None, None)

- **Loh**\
  my\_list = \[,, 1,,] # Commas determine position\
  my\_dict = {'a':}\
  foo(,,) # Passes two None arguments


### **Streamlined Definitions (def, class, type)**

The def, class, and type keywords can be replaced with symbols or made optional to reduce syntax noise.


#### **Function Definitions (def)**

The def keyword is optional.

- **Python**: def my\_func(a, b):

- **Loh**: my\_func(a, b):

\# Loh Example\
my\_add(a, b) -> int:\
    -> a + b\
\
assert my\_add(2, 3) == 5


#### **Class Definitions (class)**

Use a double-colon :: (for classes with no base) or a single colon : to define a class.

- **Python**: class MyClass(Base):

- **Loh**: MyClass:(Base):

\# Loh Example\
MyClass::\
    \_\_init\_\_(self, value):\
        self.value = value\
    get\_value(self):\
        -> self.value\
\
instance = MyClass(10)\
assert instance.get\_value() == 10


#### **Type Aliases (type)**

The type keyword can be replaced with a single colon :.

- **Python**: type IntOrFloat = int | float

- **Loh**: : IntOrFloat = int | float


### **Postfix Lambdas**

Loh provides a more natural, postfix syntax for lambdas, removing the lambda keyword.

- **Python**: add = lambda x, y: x + y

- **Loh**: add = (x, y) -> x + y

\# Loh Example\
numbers = \[1, 2, 3]\
doubled = map((x) -> x \* 2, numbers)\
assert list(doubled) == \[2, 4, 6]


### **The Dot (.) Operator**

The dot (.) is a powerful new operator in Loh that acts as a placeholder for a context-specific name.


#### **1. Standalone Variable**

Within a scope, . can be used as a variable name. This is useful for single, throwaway variables where a descriptive name is unnecessary.

- **Python**: placeholder = 100

- **Loh**: . = 100


#### **2. Implicit self in Methods**

When defining a method, . can be used as the first parameter to represent self. This is the most common and powerful use case.

- **Python**\
  class Counter:\
      def \_\_init\_\_(self, value):\
          self.value = value\
      def increment(self):\
          self.value += 1

- **Loh**\
  Counter::\
      \_\_init\_\_(., value):\
          .value = value\
      .increment():\
          .value += 1


### **Simplified for Loops**

In addition to the $ alias for for and <\~ for in, Loh provides the := operator as an alternative way to express iteration.

- **Python**\
  total = 0\
  for i in range(10):\
      total += i

- **Loh**\
  \# Using the standard alias\
  total = 0\
  $ i <\~ range(10):\
      total += i\
  \
  \# Using the := alternative\
  total = 0\
  $ i := range(10):\
      total += i

This syntax also works in comprehensions:

- **Python**: evens = \[i for i in range(10) if i % 2 == 0]

- **Loh**: evens = \[i $ i <\~ range(10) ? i % 2 == 0]
