# README {#Readme}
## Requirements for the project

- C++ compiler with c++20 support
- Cmake
- Doxygen
- Graphviz

## Style guide used:
https://google.github.io/styleguide/cppguide.html

## How to build?

Using cmake you can build following these instructions:
Assuming you are running from this projects root.
```
cmake -B build
cmake --build build -j 14
```

The compiled binary will be `brion_pipelines/build/components/pipeline_parser`
But it can be installed on your system by calling cmake install
```
cmake --install build --prefix ./
```
This will create a folder called bin in your current directory and the executable pipeline_parser will be inside it.

For the -j option (parallel building), use whatever is more efficient on your machine. 

You will be able to find the more complete documentation in the docs folder. Please use the html version, the latex was not revised.

To open the documentation you can open docs/html/index.html.

## Generating the documentation
To generate the documentation you can run
```
cmake -B build // If you didn't before
cmake --build build -j 14 --target docs
```

The generated documentation will be inside build/docs

## Running unit tests

The unit tests can be build individualy or all at the same time using the `unit_tests` target
```
cmake -B build
cmake --build build -j14 --target unit_tests
ctest -j14 -C Debug -T test --output-on-failure --test-dir build
```

# About this project
Each log message is of the following format:

pipeline_id id encoding [body] next_id 

Assumption 1: Between the 4 different parts of the message any whitespace can be used in any amount. Even before the first part of the log message.
Assumption 2: pipeline_id is a "continous string".
Assumption 3: id and next_id are also "continous strings"
Assumption 4: If the body is encoded as ascii, all ascii characters are allowed, including [] without the need for any form of escape (There are some limitations for the body TODO: Explain)
Assumption 5: If the message is ill-formed an warning will be print, the user can chose to run in silent mode, or print all ill formed lines in the end.

Definition 1: Continous string is a string where no whitespace characters is allowed.
Definition 1: Ill-formed "line" here is specified as any sequence of chars on a buffer that do no fall into the following pattern:
(newline)(whitespaces)(Continous string)(whitespaces)(Continous string)(whitespaces)(0|1)(whitespaces)([)(anything)(])(whitespaces)(Continuous string)(whitespaces)(end of line/file)