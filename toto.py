from pycallm.lib import pycallm

res = pycallm.tokenize("tokenizer.model", "Je ne pense pas qu'il y ait de bonnes ou de mauvaises situations...")

print("Tokens:")
print(res)
