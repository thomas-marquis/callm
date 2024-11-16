import torch
from safetensors.torch import save_file


def should_load_bf16_st_correctly():
    tensor = torch.tensor([[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]], dtype=torch.float32)
    tensor_bfloat16 = tensor.to(torch.bfloat16)
    tensor_float32 = tensor_bfloat16.to(torch.float32)
    tensor_dict = {"my_tensor": tensor_bfloat16, "my_tensor_float32": tensor_float32}
    save_file(tensor_dict, "my_tensor.safetensors")
    print("Tensor saved successfully in safetensors format.")
