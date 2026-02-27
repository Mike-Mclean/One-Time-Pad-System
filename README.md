# One-Time Pad Encryption System
This is a network-based One-Time Pad (OTP) encryption and decryption system in C using Unix sockets. This system supports encrypting and decrypting capital letters (`A-Z`) and spaces (` `).

There are 5 programs included in this system, as well as a shell script to compile all of them:
- `enc_server.c` - Encryption server (daemon)
- `enc_client.c` - Encryption client
- `dec_server.c` - Decryption server (daemon)
- `dec_client.c` - Decryption client
- `keygen.c` - Encyption/Decryption key generation

## `enc_server.c`
Encryption server daemon. Text is sent to the `enc_server` as well as an encyption key. The encrypted text is then sent back to  `enc_client`.
### Usage
`enc_server port_number &`
### Behavior
- Accepts connections from `enc_client`
- Rejects connections from `dec_client`
- Creates a child process using `fork()` for multiple encryptions
- Receives a file names for the text to be encrypted as well as the encryption key
- Returns encrypted text to `enc_client`
- Supports up to 5 concurrent encryption requests

## `enc_client.c`
Encryption client. Sends the name of the text file to be encrypted to `enc_server` as well as the encyption key. The client connects to the encyption server via the `port_number` specified when the command is entered. 
### Usage
`enc_client plaintext_file_name key_file_name port_number`
### Behaviour
- Connects to `enc_server`
- Sends the file name of the text and encyption key to `enc_server` to be encrypted
- Encrypted text from `enc_server` is sent to `stdout`
- Attempts to connect to `dec_serever` will be rejected

## `dec_server.c`
Decryption server daemon. Cipher text is sent to the `dec_server` as well as an encyption key. The decrypted text is then sent back to `dec_client`.
### Usage
`dec_server port_number &`
### Behavior
- Similar networking behavior to `enc_server`
- Accepts connections from `dec_client`
- Rejects connections from `enc_client`
- Creates a child process using `fork()` for multiple decryptions
- Receives a file names for the text to be decrypted as well as the encryption key
- Returns encrypted text to `dec_client`
- Supports up to 5 concurrent decryption requests

## `dec_client.c`
Decryption client. Sends the name of the cipher file to be decrypted to `dec_server` as well as the encyption key. The client connects to the encyption server via the `port_number` specified when the command is entered. 
### Usage
`dec_client ciphertext_file_name key_file_name port_number`
### Behavior
- Connects to `dec_server`
- Sends the file name of the cipher text and encyption key to `dec_server` to be decrypted
- Decrypted text from `dec_server` is sent to `stdout`
- Attempts to connect to `enc_serever` will be rejected

## keygen
Generates a random encryption key of length `keylength` consisting of `A-Z` + spaces
### Usage
`keygen keylength`
### Behavior
- Generates `keylength` random characters
- Uses `rand()` function
- Outputs the encryption key to `stdout`

## Networking
- All network programs connect to a port number on `localhost`
- Network programs use the Unix networking functions listed below:
  - `socket()`
  - `connect()`
  - `bind()`
  - `listen()`
  - `accept()`
  - `send()`
  - `recv()`
- Communication happens exclusively over sockets
