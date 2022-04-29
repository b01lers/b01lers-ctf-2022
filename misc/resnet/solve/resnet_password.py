import torch
from PIL import Image
from torchvision import transforms
from torchvision.models.resnet import resnet18

from io import BytesIO
import base64

from Embedding import key


model = resnet18(pretrained=True)

# get embedding rather than logits from final layer
model.fc = torch.nn.Identity()

try:
    with open("flag.txt") as f:
        FLAG = f.read()
except FileNotFoundError:
    FLAG = "TEMP_FLAG"
    
key = torch.tensor(key)

def check_image():
    encoded = input("Input base64 image password\n")
    decoded = Image.open(BytesIO(base64.b64decode(encoded)))
    tensored = transforms.ToTensor()(decoded)
    
    with torch.no_grad():
        embedding = model(tensored.reshape(1, 3, 224, 224))[0]
        diff = ((embedding - key)**2).mean()
        diff = diff.item()
    
    print(f"Image embedding differed by {diff}")
    if diff < 1e-4:
        print(f"Nice! Here's the flag: {FLAG}")


if __name__ == '__main__':
    check_image()