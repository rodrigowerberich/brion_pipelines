# Parsing {#Parsing}

Parsing is divided into two parts:
- Structure
    - structure.h
    - structure.cc
- Semantics
    - semantics.h
    - semantics.cc
    - ascii_body_parser.h
    - ascii_body_parser.cc
    - hex16_body_parser.h
    - hex16_body_parser.cc

## Structure Parsing

The structure parsing goes over the file and tries to identify the Pipeline Log Messages. Its output is a list of log messages with the 5 fields:

- Pipeline ID
- ID
- Encoding
- Body
- Next ID

and a list of the errors found during parsing. 
These fields are just a blob of data at this point and don't represent anything.

The current algorithm used to identify the fields are:
1) Skip all whitespace until a non-whitespace is found
2) Read all data until a whitespace, this is our pipeline ID.
3) Skip all whitespace
4) Read all data until a whitespace, this is our ID
5) Skip all whitespace
6) Read all data until a whitespace, this is our encoding
7) Read all whitespace
8) If we found an open bracket, continue, otherwise throw a parsing error
9) Read all characters until you find a close bracket, followed by whitespace, followed by a continuous string without close bracket, followed by a new line. If the file ends before that, throw a parsing error. Otherwise, the body is the content between the open brackets and the last close bracket. And the next id is the continuous string after that close bracket.

Advantages of this algorithm:
- If there is no ill form related to the body, it allows open brackets "[" and close bracket "]" to be freely used inside the body and can parse correctly

Disadvantages of this algorithm
- If there is an ill form, the next valid message will be added to the body of this message, or if there are other brackets in that body it can lead to even more messages being dropped.
- Closing brackets can not be used as ids, as they will cause problem in the parsing.

Ideally the ascii body should have character scaping for the brackets, this way this could be avoid.

Another options that was considered was to use regexes, but they are a bit slower and this parser does not take much too write. (It also allow me to show a bit more for this coding exercise)

## Semantics parsing

For the semantics parsing there is not much to do, all the fields except the body and the encoding don't really have any semantic meaning and any value is allowed there. 

In the future in case there was some id form we would like to reject it could be added here.

For extensibility the semantics parser allows body parsers to be registered under different encoding codes. These encoding codes are any continous string, but for this application we only use 0 or 1.

The semantics parser then checks if the encoding code is one of the registereds one, and then call the appropriated body parser. If no encoding is found, a parsing error is added to the list of errors and that message is ignored.

The body parsers can also throw parsing errors, which are added to the list of errors. If a body parser throws an error, that message will be ignored and not processed.

The hex parser cleans up any whitespace inside the body, checks the number of characters is even, and that the characters are valid hex numbers. It then transforms then into ascii characters.

The ascii parser currently does nothing. But it could in theory clean up escaped characters.