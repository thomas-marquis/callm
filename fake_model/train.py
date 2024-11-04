import torch
import torch.nn as nn
import torch.nn.functional as F
from safetensors.torch import save_file, save_model
# from torchviz import make_dot

# class Net(nn.Module):
#     def __init__(self):
#         super(Net, self).__init__()
#         # Input layer is 10, hidden layer is 5
#         self.hidden = nn.Linear(10, 5)
#         # Output layer is 5 (same as hidden layer) to 1
#         self.output = nn.Linear(5, 1)
#
#     def forward(self, x):
#         # Pass the input through the hidden layer, then apply the ReLU function
#         x = F.relu(self.hidden(x))
#         # Pass through the output layer
#         x = self.output(x)
#         return x

if __name__ == '__main__':
    # net = Net()
    # print(net)
    #
    # x = torch.randn(10)
    # output = net(x)
    # print(output)
    #
    # save_model(net, 'model.safetensors')

    model = torch.load("consolidated.00.pth", map_location="cpu")
    for k, v in model.items():
        print(k, v.shape)
    # for name, module in model.named_modules():
    #     print(f"Couche : {name}")
    #     print(module)
