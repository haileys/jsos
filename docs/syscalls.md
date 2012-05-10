# JSOS System Calls
### yield(`callback`)

> 
> Yields control to another process
> 

> **callback:** A function the operating system should return control to

[Source](../kernel/js/kernel/process.js#L87)

### log(`msg`)

> 
> Logs a message to the text console along with the process's ID
> 

> **msg:** The message to log

[Source](../kernel/js/kernel/process.js#L101)

### pid()

> 
> Returns the current process's ID

[Source](../kernel/js/kernel/process.js#L106)

### parentPid()

> 
> Returns the process ID of the current process's parent process

[Source](../kernel/js/kernel/process.js#L111)

### read(`fd`, `size`, `callback`)

> 
> Reads at most `size` bytes from the file identified by `fd`
> 

> **fd:** The file descriptor to be read from

> **size:** The maximum number of bytes to be read

> **callback:** The function to be called upon completion of the read.               The callback will be passed two arguments: `error` and `buff`

[Source](../kernel/js/kernel/process.js#L125)

### write(`fd`, `data`)

> 
> Writes data to an open file
> 

> **fd:** The file descriptor to be written to

> **data:** The data to write

[Source](../kernel/js/kernel/process.js#L142)

### spawnChild(`image`)

> 
> Spawns a child process and returns and object containing the child
> ID (`pid`)
> 

> **image:** The image as a binary buffer to load and execute in the new process

[Source](../kernel/js/kernel/process.js#L156)

### loadImage(`image`)

> 
> Loads and executes an image in the current process
> 

> **image:** The image as a binary buffer

[Source](../kernel/js/kernel/process.js#L176)

### ioctl(`fd`, `method`, `args`)

> 
> Calls a device specific method on an open file
> 

> **fd:** The file descriptor to call the method on

> **method:** The method to call. This is a device-specific string

> **args:** An argument to pass to the method. May be undefined.

[Source](../kernel/js/kernel/process.js#L188)

### readDirectory(`path`, `callback`)

> 
> Reads all entries from a directory. Each entry is represented as an
> object with a `name` property and `type` property.
> 

> **path:** The directory to list

> **callback:** The function to be called upon completion of the operation.               The callback will be passed two arguments: `error` and `entries`

[Source](../kernel/js/kernel/process.js#L207)

### open(`path`, `callback`)

> 
> Opens a file
> 

> **path:** The absolute path to the file to be opened

> **callback:** The function to be called upon completion of the operation.               The callback will be passed two arguments: `error` and `fd`

[Source](../kernel/js/kernel/process.js#L238)

### close(`fd`)

> 
> Closes an open file
> 

> **fd:** The file descriptor to close

[Source](../kernel/js/kernel/process.js#L261)

### stat(`path`, `callback`)

> 
> Returns an object with metadata about a file. The object will have
> the properties `size` (the size of the file in bytes), `name` (the
> name of the file), `type` (one of "file" or "directory"), and `path`
> (the absolute path to the file)
> 

> **path:** The absolute path to the file

> **callback:** The function to be called upon completion of the operation.               The callback will be passed two parameters: `error` and `stat`

[Source](../kernel/js/kernel/process.js#L281)

### env(`name`, `value`)

> 
> Gets or sets an environment variable for the current process
> 

> **name:** The name of the variable to get/set

> **value:** Optional. If omitted, `OS.env` will return the current           value of the environment variable `name`. If set, the           environment variable will be set to this value

[Source](../kernel/js/kernel/process.js#L318)

### wait(`pid`, `callback`)

> 
> Waits for a process to terminate
> 

> **pid:** The ID of the process to wait on

> **callback:** The function to call when the process identified by               `pid` exits. This function will be passed the exit status               of the process as its only parameter

[Source](../kernel/js/kernel/process.js#L338)

### exit()

> 
> Terminates the current process. This system call will return, but
> no more callbacks will be scheduled by the system for this process

[Source](../kernel/js/kernel/process.js#L363)

### dup(`src`, `dest`)

> 
> Duplicates an open file descriptor. This creates an alias for the
> same open file on a different descriptor number.
> 

> **src:** The existing file descriptor to duplicate

> **dest:** Optional. If set, this will be the descriptor number for           alias. This will overwrite any existing file descriptor on           this number. If omitted, a descriptor number will be picked           by the OS.

> **returns:** The descriptor number of the alias

[Source](../kernel/js/kernel/process.js#L376)

### pipe()

> 
> Creates a new bi-directional pipe
> 

> **returns:** The file descriptor the pipe is open on

[Source](../kernel/js/kernel/process.js#L396)

