from Hardcore import digest_to_array, bitstring_to_bytes, predictor

import socket
import hashlib

import numpy as np


def send_recv(guess, sock):
    bitstring = ''.join(guess.astype(int).astype(str))
        
    sock.sendall(bytes(bitstring, "utf-8") + b'\n')
    response = int(sock.recv(256).strip())
    
    return response


def solve_challenge_two(sock):
    I = np.identity(256)

    secret = np.zeros(256).astype(int)
    for ix, e in enumerate(I):
        guesses = []
        for _ in range(15):
            guess = np.random.randint(0, 2, 256)
            
            # XOR with random
            rand_guess = (guess.copy() != e).astype(int)
            
            guess_pred = send_recv(guess, sock)
            rand_pred = send_recv(rand_guess, sock)
            
            # XOR (again) the random guess with normal guess
            guesses.append(int(guess_pred != rand_pred))
            
        # final guess is majority vote of guesses
        secret[ix] = np.median(guesses)
        print(f"cracked bit {ix}", end="\r")
    
    # convert to digest 
    flag = ''.join(secret.astype(int).astype(str))
    flag = bitstring_to_bytes(flag)
    
    return flag


HOST, PORT = "ctf.b01lers.com", 9003

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

# get first message out of the way
print(str(s.recv(2048).decode()))
s.sendall(b'2\n')
print(str(s.recv(2048).decode()))

flag = solve_challenge_two(s)

hasher = hashlib.sha256()
hasher.update(flag)
hash = hasher.hexdigest()

# make sure we got it right
print("Flag:", flag, "Hash:", hash)
