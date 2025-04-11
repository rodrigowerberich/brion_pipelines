Style guide used:
https://google.github.io/styleguide/cppguide.html

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