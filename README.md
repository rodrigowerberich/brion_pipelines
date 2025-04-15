# README {#Readme}
## Requirements for the project

- C++ compiler with c++20 support
- Cmake
- Doxygen (For documentation)
- Graphviz (For documentation)

You can also use the docker file in this repo for a quick build, see the details in the end of this file

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

## How to run?

You can run the application by calling it and passing the file you want to parse as argument.
```
bin/pipeline_parser <file_name>
```

A small help with some further options can be seen by passing -h
```
bin/pipeline_parser -h
```

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


## Running over docker

If you want to build using docker you can run, with this repo as the working directory

```
docker build . -t brion_pipeline:latest
```

If it worked, you should see the help message if you run
```
docker run brion_pipeline:latest
```

To parse a file, e.g example 1, execute:

```
docker run -v ./examples/test_file1.txt:test_file1.txt brion_pipeline:latest ./bin/pipeline_parser test_file1.txt
```


## More info

For more info see the [overview](@ref Overview)