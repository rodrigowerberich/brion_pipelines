# App {#app}

## Application logic

The execution parses the command line arguments and provides them to application runner.

The runner is enveloped with a bare bones error handling mechanism, just to differentiate between known application errors(ApplicationRuntimeError), unexpected standard error, or any other abnormal behavior and provide a more graceful termination, returning 1.

The following is the happy flow of the application:
@dot
digraph example {
    node [shape=record, fontname=Helvetica, fontsize=10];
    parse_cli [ label="Parse command line arguments"];
    parse_file [ label="Parse input file"];
    split_messages [ label="Split messages by pipeline id"];
    organize [ label="Sort messages of each pipeline"];
    print [ label="Print message to desired output"];
    parse_cli -> parse_file [ arrowhead="open", style="solid" ];
    parse_file -> split_messages [ arrowhead="open", style="solid" ];
    split_messages -> organize [ arrowhead="open", style="solid" ];
    organize -> print  [ arrowhead="open", style="solid" ];
}
@enddot

## Program options

### Verbosity
By default messages that are ill-formed are silently ignored. If run in the verbose mode (-v or --verbose), the identified errors will be print out.

### Strict mode
With the strict mode (-s or --strict), any ill formed message will cause the program to stop.

### Save output to file
By default the output is writen to the standard output. That can be changed with the -o or --output option, which will instead save the result on the give file. 

## Parsing and organizing
For details on parsing and organizing the messages check
- [Parsing](@ref Parsing)
- [Organizing](@ref Organizing)