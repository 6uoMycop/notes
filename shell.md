# Shell Scripting Basics

## Basic Script Building

### Shell Variables

**Setting and exporting variables**

- `srcfile=dataset1`<br>
  Creates (if it didn't exist) a variable named "srcfile" and sets it to the value "dataset1". If the variable already existed, it is overwritten. Note there must be no spaces around the "=".

**Using variables**

- `$srcfile`<br>
  Prefacing the variable name with $ causes the value of the variable to be substituted in place of the name.

- `${srcfile}`<br>
  If the variable is not surrounded by whitespace, the name must be surrounded by "{}" braces so that the shell knows what characters you intend to be part of the name.
  ```Shell
  datafile=census2000
  # Tries to find $datafile_part1, which doesn't exist
  echo $datafile_part1.sas 
  # This is what we intended
  echo ${datafile}_part1.sas
  ```

### Manipulating Variables

**Numeric variables**

- `$(( integer expression ))`<br>
  The $(( ... )) construction interprets the contents as an arithmetic expression. Variables are referenced by name without the "$".<br>
  `i=$((i+1))` often used in `while` loops

### Flow Control

**Conditional execution: if/else**

```Shell
if [[ condition ]]; then
    #statements
fi
```

**Looping: 'while' and 'for' loops**

```Shell
for i in words; do
    #statements
done

for (( i = 0; i < 10; i++ )); do
    #statements
done
```

### Conditional tests

**Character string tests**

- `-z "string"`<br>
  true if string has zero length 
  ```Shell
  String=''

  if [[ -z "$String" ]]; then
      echo "\$String is null."
  fi
  ```

- `$variable = text`<br>
  True if $variable matches text.
  ```Shell
  app="redis"

  if [[ "$app" == "ssdb" ]]; then
      #statements
  fi
  ```

### More conditional tests

**Arithmetic tests**

- `$variable -eq number`<br>
  True if *$variable*, interpreted as a number, is equal to *number*.

- `$variable -ne number`<br>
  True if *$variable*, interpreted as a number, is not equal to *number*.

### Arrays
a convenient way of initializing an entire array is the `array=(element1 element2 ... elementN )` notation. To dereference (retrieve the contents of) an array element, use curly bracket notation, that is, `${array[index]}`. If the index number is @, all members of an array are referenced. To get a count of the number of elements in an array, use either `${#array[@]}`.

```Shell
farm_hosts=("web03" "web04" "web05" "web06" "web07")
echo ${farm_hosts[2]} # web05
farm_hosts[1]=web02
echo ${farm_hosts[*]} # web03 web02 web05 web06 web07

for i in ${farm_hosts[@]}; do
    #statements
done
```
### Advanced I/O

**"Here" documents**

- `<< [-]string`<br>
  redirect input to the temporary file formed by everything up the matching *string* at the start of a line.
  ```Shell
  #!/bin/sh

  cat <<EOF
   This text will be fed to the "cat" program as 
   standard input.  It will also have variable
   and command substitutions performed.
   I am logged in as $USER and today is `date`
  EOF
  ```

### I/O redirection and pipelines

**Output redirection**

- `> filename`<br>
  Standard ouput (file descriptor 1) is redirected to the named file.

- `>> filename`<br>
  Standard ouput is appended to the named file. The file is created if it does not exist.

**Command pipelines**

- `command | command [ | command ...]`<br>
  Pipe multiple commands together. The standard output of the first command becomes the standard input of the second command.

### Preset Variables

**Shell internal settings**

- `$PWD`<br>
  Always set the current working directory

- `$?`<br>
  Set to the exit status of the last command run, so you can test success or failure.

### Command Substitution

**ksh/bash syntax**

- `$(command)`<br>
  A command enclosed is executed and the standard output of that command is substituted.
  ```Shell
  file=/etc/hosts

  echo The file $file has $(wc -l < $file) lines
  ```

### Input and Output

**Script output**

- `echo`<br>
  - `echo "foo"`<br>
    Outputs "foo" with trailing newline
  - `echo -n "foo"`<br>
    Outputs "foo" but doesn't send a newline
  - `echo -e "\tfoo"`<br>
    Outputs "foo" with a tab character in front and a trailing newline

### Signals
Unix signals (software interrupts) can be sent as asynchronous events to shell scripts, just as they can to any other program. The default behaviour is to ignore some signals and immediately exit on others. Scripts may detect signals and divert control to a handler function. This is often used to perform clean-up actions before exiting.

- `trap handler sig ...`<br>
  *handler* is a command to be read and executed on receipt of the specified *sigs*. Signals can be specified by name or number e.g. `HUP`, `INT`, `QUIT`, `TERM`. A Ctrl-C at the terminal generates a `INT`.
  ```Shell
  # Perform any shell command in the single quotes when the "INT" signal is generated

  trap 'echo "Shell command here";' INT
  ```

### Common external commands
**Filesystem**

- `pushd`, `popd`<br>
  This command set is a mechanism for bookmarking working directories, a means of moving back and forth through directories in an orderly manner. A pushdown stack is used to keep track of directory names.

  `pushd dir-name` pushes the path `dir-name` onto the directory stack (to the top of the stack) and simultaneously changes the current working directory to `dir-name`.

  `popd` removes (pops) the top directory path name off the directory stack and simultaneously changes the current working directory to the directory now at the top of the stack.

**Variables**

- `set`<br>
  One use for this is to toggle option flags which help determine the behavior of the script.
  
  Options | Effect
  ------------ | -------------
  `-e` | Exit immediately if a command exits with a non-zero status.
  `-v` | Print each command to stdout before executing it

## Script Control

### Generating Signals

**Pausing a process**

The Ctrl-Z key combination generates a `SIGTSTP` signal, stopping any processes running in the sell.
```
$ ./fred & # run a command in the background with an ampersand (&)
[1] 2276   # 2276 is the PID the Linux system assigns to the process.
$ ./bob
bob is running...
CTRL-Z
[2]+  Stopped                 bob
```
The number in the square brackets is the job number assigned by the shell. The shell refers to each child process running in the shell as a job, and assigns each job a unique job number.

To resume a suspended job so that it continues to run in the foreground, just type `fg job_number`.

### Running Scripts without a Console

When you run a shell session, all the process you run at the command line are child processes of that shell. If you log out or your session crashes or otherwise ends unexpectedly, `SIGHUP` kill signals will be sent to its child processes (including background processes) to end them too.

You can get around this by telling the process(es) that you want kept alive to ignore `SIGHUP` signals. There are two ways to do this: by using the `nohup` command to run the command in an environment where it will ignore termination signals.
