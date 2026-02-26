#!/bin/bash
gcc enc_server.c -o enc_server
gcc enc_client.c -o enc_client
gcc dec_server.c -o dec_server
gcc dec_client.c -o dec_client
gcc keygen.c -o keygen