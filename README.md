```
        _-_
       |(_)|
        |||
        |||            __        _____ _   _ ____  _        _    ____ ____
        |||            \ \      / /_ _| | | |  _ \| |      / \  / ___|  _ \
        |||             \ \ /\ / / | ||  \| | | | | |     / _ \ \___ \___ \
        |||              \ V  V /  | || |\  | |_| | |___ / ___ \ ___) |__) |
  ^     |^|     ^         \_/\_/  |___|_| \_|____/|_____/_/   \_\____/____/
< ^ >   <+>   < ^ >
 | |    |||    | |
  \ \__/ | \__/ /
    \,__.|.__,/
        (_)
```

# Windlass

Emulating the approach [SNOWLIGHT](https://sysdig.com/blog/unc5174-chinese-threat-actor-vshell/), this dropper supports multiple payload formats:

- A remotely hosted payload (ELF64) that is downloaded and executed in memory (retrieved via libcurl)
- A XOR encoded payload (ELF64) that is decoded and executed in memory

In both scenarios, the newly executed process (the payload) will appear as `[kworker/1:1-events]`. This can be modified with a find & replace in the `.c` files.

Regardless of retrieval method (curl or decoding), the payload will be stored as a file in memory and executed via file descriptor with `fexecve` avoiding any on-disk presence except for the dropper itself. 

# Usage
- Modify the source code to update the URL of the payload to be downloaded (`dropped_curl.c`) or add the XOR encoded payload (`dropper_xor.c`).

## Encoding your payload

Supporting tooling under `/encoders` can be used to encode either a bytecode format (e.g. `xxd -i <binary> > <output.h>`) or an existing ELF64 binary (with no conversion required). 

> [!IMPORTANT]
> If generating bytecode using `xxd` you may need to strip additional non-hex content from the output.

Both of these scripts utilize the following arguments: `python3 xor_encoder_foo.py <input_file> <xor_key>` with the key specified in the format of `0x12`. The XOR key is a single byte, and the encoder will XOR the input file with the key and write the result to stdout (you can redirect as you wish). The payload should then be added into `dropper_xor.c` prior to compilation.

### dropper_curl.c
> [!NOTE]
> `libcurl` needs to be available on your system for this to compile

- To compile dropper_curl.c, run the following command:
```bash
gcc -o dropper_curl dropper_curl.c -lcurl
```

### dropper_xor.c
- To compile dropper_xor.c, run the following command:
```bash
gcc -o dropper_xor dropper_xor.c
```

## Notes
- The compiled binary, using these instructions, will have no sort of obfuscation and is dynamically linked.

