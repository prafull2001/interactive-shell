# interactive-shell
Interactive Unix shell written in C

## Features

### interactive and bash modes

Mode is determined upon shell startup

- interactive mode: if 0 arguments *i.e.* `./mysh` 
- batch mode: if 1 argument *i.e.* `./mysh [batch-file]\n`

###  `alias` command

This shell applications aliaising commands using the `alias` key word:

- `alias ls /bin/ls`
- `ls`

You can also `unalias` commands:

- `unalias ls`

### Redirection


## How it Works
When a command is typed into the prompt, shell creates a child process using the `fork` syscall that executes the entered command using the `exec()` syscall while the parent process `wait()`s for the cihld to finish executing.
