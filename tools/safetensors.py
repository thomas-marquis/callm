import torch
from safetensors import safe_open


def parse_and_display_header(file_path):
    """
    Opens a safetensors file, parses the header, and displays it.

    Args:
        file_path (str): The path to the safetensors file.
    """
    with safe_open(file_path, framework="pt") as f:
        header = f.metadata()
        print("Header information:")
        print(header, f.keys())
