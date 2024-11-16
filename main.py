from tests.integration.load_safetensors import should_load_bf16_st_correctly
from tools.safetensors import parse_and_display_header

if __name__ == "__main__":
    file_path = "my_tensor.safetensors"
    should_load_bf16_st_correctly()
    parse_and_display_header(file_path)
