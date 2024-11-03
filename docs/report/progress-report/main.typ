#import "@preview/diatypst:0.1.0": *

#show: slides.with(
  title: "CS323 Project Progress Report", // Required
  subtitle: "The Compiler of a Functional Programming Language",
  date: "01.07.2024",
  authors: ("Zhezhen Cao Fan Club"),

  // Optional Styling
  ratio: 16/9,
  layout: "medium",
  title-color: blue.darken(60%),
  footer: true,
  counter: true,
)

#set list(indent: 1em)
#set enum(indent: 1em)

= Language Design

== Overview of the Design

#align(center)[
#quote[
_Most functional languages *are very similar*, and *vary largely in syntax*._
]
]
#align(right)[
  -- Simon L. Peyton Jones《函数式编程的实现》
]

We represent the following things:

- *Defining functions*
  ```
  defn f x = {x + 1} 
  ```
- *Declaring data types*
  ```
  data List = { Nil, Cons Int List }
  // data type List is either 'Nil' or 'Cons Int List'
  ```
- *Applying fuctions*
  ```
  f 10
  ```
- *Arithmetic*
  - $+, -, times, div, %$
  - bitwise operations
  - boolean operations
- *Algebraic data types*
  - list: `[0, 1, 2,]`
  - tuple: `(0, 1, 2,)`
- *Pattern matching* (to operate on data types)
  ```
  case l of {
    Nil -> { 0 }
    Cons x xs -> { x }
  }
  ```

== Lexical Specification

We have the following token types:

1. 

= Compiler Design

== Structure

#lorem(20)

= Implementation Progress

== Lexical Analyzer

#lorem(20)