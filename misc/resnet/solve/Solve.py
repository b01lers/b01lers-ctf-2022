import torch
import torch.nn as nn
from torchvision.utils import save_image
from torchvision.models.resnet import resnet18
from PIL import Image

from io import BytesIO
import base64
import socket

from resnet_password import key, model


key = torch.tensor(key)

guess = nn.Parameter(torch.rand(1, 3, 224, 224, dtype = torch.float32))
optimizer = torch.optim.AdamW([guess], lr = .01)
loss_func = nn.MSELoss()
batch_size = 1

for i in range(1000):
    embeddings = model(guess)
    loss = loss_func(embeddings[0], key)
    
    # need to have much lower loss to compensate for precision error once encoded to png/base64
    if loss.item() < 1e-6:
        print("DONE!")
        break
    
    loss.backward(retain_graph = True)
    optimizer.step()
    optimizer.zero_grad()
    
    if i % 10 == 0:
        print(f'Iteration: {i}, loss {loss.item()}')
        
# use PNG to avoid lossy compression 
save_image(guess, 'guess.png')

image = Image.open("guess.png")
buffered = BytesIO()
image.save(buffered, format="PNG")
img_str = base64.b64encode(buffered.getvalue())
print(img_str)

# Confirm with sever
HOST, PORT = "localhost", 3001

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

# get first message out of the way
print(str(s.recv(2048).decode()))
s.sendall(img_str)
s.send(b'\n')
print(str(s.recv(2048).decode()))
print(str(s.recv(2048).decode()))
