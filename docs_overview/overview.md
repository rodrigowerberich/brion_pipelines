@mainpage
# Overview {#Overview}
## Problem description

Multiple log messages from multiple stream have been merged together. The order of the logs is randomized and needs to be organized.

Each log message is of the following format:

```
pipeline_id id encoding [body] next_id 
```

The messages are to be grouped by pipeline_id and display in reverse order of id. The order is determined by the id and next id fields.

In a real scenario I would engage with the stakeholders to clarify some desired behavior, but since this is an exercise and due to time constraints I decided to take some assumptions instead. In the real world it is not a good practice to try to guess what your user wants, you should always ask them.

For these assumption, a couple of definitions are used, see the list of definitions later on

## Assumptions

- Assumption 1: Between the 5 different parts of the message any whitespace can be used in any amount. Even before the first part of the log message.
- Assumption 2: pipeline_id is a "continous string".
- Assumption 3: Encoding is a "continuous string"
- Assumption 4: id and next_id are also "continous strings"
- Assumption 5: If the body is encoded as ascii, all ascii characters are allowed, including [] without the need for any form of escape (These will cause some limitations which will be addressed later)
- Assumption 6: If the message is ill-formed an warning will be saved, the user can chose to run in verbose mode to see those warnings.
- Assumption 7: Any ill-formed message will be disregarded and the valid messages will be organized.


## Definitions
- Definition 1: Continous string is a string where no whitespace characters is allowed.
- Definition 2: Ill-formed "line" here is specified as any sequence of chars on a buffer that do no fall into the following pattern:
```
(newline)(whitespaces)(Continous string)(whitespaces)(Continous string)(whitespaces)(Continous string)(whitespaces)([)(anything)(])(whitespaces)(Continuous string)(whitespaces)(end of line/file)
```

## Structure of this project

For more info on the structure and how to build, check the [readme](@ref Readme)

## Software solution

The solution was divided into 3 parts, parse (log_message_parser), orgazine (log_message_organizer) and the [app component](@ref app.cc). There is also a log_message components which is used to define a log_message and remove a coupling from the organizer to the parser. There is also an external component used to build the CLI interface. [Clipp](https://github.com/muellan/clipp) was used for its lightweight, simple interface.

For more details on each part you can click on the components on the diagram below. I recommend starting with the app component.
@dot
digraph example {
    node [shape=record, fontname=Helvetica, fontsize=10];
    app [ label="app" URL="\ref app"];
    log_message_parser [ label="log_message_parser" URL="\ref Parsing"];
    log_message_organizer [ label="log_message_organizer" URL="\ref Organizing"];
    log_message [ label="log_message" URL="\ref log_message"];
    clip [ label="clipp" ]
    app -> log_message_parser [ arrowhead="open", style="dashed" ];
    app -> log_message_organizer [ arrowhead="open", style="dashed" ];
    app -> clip [ arrowhead="open", style="dashed" ];
    log_message_organizer -> log_message  [ arrowhead="open", style="dashed" ];
    log_message_parser -> log_message [ arrowhead="open", style="dashed" ];
}
@enddot


## What is missing? Or next steps

- Automated integration/application level tests.
- Better documented unit tests.
- Unit tests for the app level.
- More options for parsing, different modes of parsing. Being less tolerant with brackets or, single line only mode.
- Better parsing error detection and messages.
- Option to order the termination ids and invalid ids by some criterium, e.g., alphabetical order, covert to int and sort, etc.
- Chose a different CLI library that wouldn't have problem compiling on clang and mvc. (I've had too strict options)


Go to [app](@ref app)