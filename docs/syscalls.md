# JSOS System Calls
### yield(`callback`)

> 
> Yields control to another process
> 

> **callback:** A function the operating system should return control to

[Source](../kernel/js/kernel/process.js#L87)

### log(`msg`)

> undocumented

[Source](../kernel/js/kernel/process.js#L97)

### pid()

> undocumented

[Source](../kernel/js/kernel/process.js#L100)

### parentPid()

> undocumented

[Source](../kernel/js/kernel/process.js#L103)

### read(`fd`, `size`, `callback`)

> undocumented

[Source](../kernel/js/kernel/process.js#L110)

### write(`fd`, `data`)

> undocumented

[Source](../kernel/js/kernel/process.js#L122)

### spawnChild(`image`)

> undocumented

[Source](../kernel/js/kernel/process.js#L131)

### loadImage(`image`)

> undocumented

[Source](../kernel/js/kernel/process.js#L150)

### ioctl(`fd`, `method`, `args`)

> undocumented

[Source](../kernel/js/kernel/process.js#L156)

### readDirectory(`path`, `callback`)

> undocumented

[Source](../kernel/js/kernel/process.js#L168)

### open(`path`, `callback`)

> undocumented

[Source](../kernel/js/kernel/process.js#L193)

### close(`fd`)

> undocumented

[Source](../kernel/js/kernel/process.js#L212)

### stat(`path`, `callback`)

> undocumented

[Source](../kernel/js/kernel/process.js#L223)

### env(`name`, `value`)

> undocumented

[Source](../kernel/js/kernel/process.js#L253)

### wait(`pid`, `callback`)

> undocumented

[Source](../kernel/js/kernel/process.js#L266)

### exit()

> undocumented

[Source](../kernel/js/kernel/process.js#L288)

### dup(`src`, `dest`)

> undocumented

[Source](../kernel/js/kernel/process.js#L291)

### pipe()

> undocumented

[Source](../kernel/js/kernel/process.js#L307)

