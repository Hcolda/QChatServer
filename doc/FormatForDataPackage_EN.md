# Data package
## Format
Format: `length + requestID + data type + sequence + hash + binary data + double '\0'`

| Type | Name | Value | Comment |
| :---: | :---: | :---: | :---: |
| int | length | Length of data package |  |
| int | type | Default 0 | Type of data package, `1 for text, 2 for binary, 3 for file stream, 4 for heartbeat package` |
| int | sequneceSize | Default 1 | Valid if data package is splitted |
| int | sequence | Default 0 | Valid if data package is splitted |
| long long | requestID |  |  |
| char | data | Binary data | |
