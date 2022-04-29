from Hardcore import digest_to_array, bitstring_to_bytes, predictor

import socket
import hashlib

import numpy as np


def solve_challenge_one(sock):
    I = np.identity(256)

    secret = np.zeros(256).astype(int)
    for ix, e in enumerate(I):
        bitstring = ''.join(e.astype(int).astype(str))
        
        sock.sendall(bytes(bitstring, 'utf-8') + b'\n')
        response = int(sock.recv(256).strip())
        
        secret[ix] = response
    
    # convert to digest 
    flag = ''.join(secret.astype(int).astype(str))
    flag = bitstring_to_bytes(flag)
    
    return flag


HOST, PORT = "localhost", 3000

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

# get first message out of the way
print(str(s.recv(2048).decode()))
s.sendall(b'1\n')
print(str(s.recv(2048).decode()))

flag = solve_challenge_one(s)

hasher = hashlib.sha256()
hasher.update(flag)
hash = hasher.hexdigest()

# make sure we got it right
print("Flag:", flag, "Hash:", hash)