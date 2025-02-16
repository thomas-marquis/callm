import sys

# def should_load_bf16_st_correctly():
#     tensor = torch.tensor([[1.0, 2.0, 3e10], [-4.0, 0.0, 6.123456]], dtype=torch.float32)
#     tensor_bfloat16 = tensor.to(torch.bfloat16)
#     tensor_float32 = tensor_bfloat16.to(torch.float32)
#     tensor_dict = {"my_tensor": tensor_bfloat16, "my_tensor_float32": tensor_float32}
#     save_file(tensor_dict, "my_tensor.safetensors")
#     print("Tensor saved successfully in safetensors format.")
