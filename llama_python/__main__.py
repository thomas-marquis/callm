import json
import sys

sys.path.append(".")

from llama_python.configuration import LlamaConfig
from llama_python.model import LlamaForCausalLM


def main():
    config_content: dict = json.load(open("resources/original_llama/config.json", "r"))
    config = LlamaConfig(**config_content)
    model = LlamaForCausalLM(config)


if __name__ == "__main__":
    main()
