# One-Time Pad Encryption System
This is a network-based One-Time Pad (OTP) encryption and decryption system in C using Unix sockets. This system supports encrypting and decrypting capital letters (`A-Z`) and spaces (` `).

There are 5 programs included in this system, as well as a shell script to compile all of them:
- `enc_server.c` - Encryption server (daemon)
- `enc_client.c` - Encryption client
- `dec_server.c` - Decryption server (daemon)
- `dec_client.c` - Decryption client
- `keygen.c` - Encyption/Decryption key generation

## `enc_server.c`
### Usage
`enc_server port_number &`
### Behavior
- Accepts connections from `enc_client`
- Rejects connections from `dec_client`
- Creates a child process using `fork()`
- Receives a file names for the text to be encrypted as well as the encryption key
- Returns encrypted text to `enc_client`
- Supports up to 5 concurrent encryption requests

## `enc_client.c`
### Usage
`enc_client plaintext_file_name key_file_name port_number`
### Behaviour
