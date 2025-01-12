# from tests.integration.load_safetensors import should_load_bf16_st_correctly
# from tools.safetensors import parse_and_display_header
from pathlib import Path

import tiktoken
from tiktoken.load import load_tiktoken_bpe

monologue_otis = (
    "Mais, vous savez, moi je ne crois pas qu’il y ait de bonne ou de "
    "mauvaise situation. Moi, si je devais résumer ma "
    "vie aujourd’hui avec vous, je dirais que c’est d’abord des rencontres, "
    "des gens qui m’ont tendu la main, "
    "peut-être à un moment où je ne pouvais pas, où j’étais seul chez moi. "
    "Et c’est assez curieux de se dire que les "
    "hasards, les rencontres forgent une destinée… Parce que quand on a le "
    "goût de la chose, quand on a le goût de la "
    "chose bien faite, le beau geste, parfois on ne trouve pas "
    "l’interlocuteur en face, je dirais, le miroir qui vous "
    "aide à avancer. Alors ce n’est pas mon cas, comme je le disais là, "
    "puisque moi au contraire, j’ai pu ; et je dis "
    "merci à la vie, je lui dis merci, je chante la vie, je danse la vie… "
    "Je ne suis qu’amour ! Et finalement, quand "
    "beaucoup de gens aujourd’hui me disent : « Mais comment fais-tu pour "
    "avoir cette humanité ? » Eh bien je leur "
    "réponds très simplement, je leur dis que c’est ce goût de l’amour, ce "
    "goût donc qui m’a poussé aujourd’hui à "
    "entreprendre une construction mécanique, mais demain, qui sait, "
    "peut-être simplement à me mettre au service de la "
    "communauté, à faire le don, le don de soi…"
)

if __name__ == "__main__":
    # file_path = "my_tensor.safetensors"
    # should_load_bf16_st_correctly()
    # parse_and_display_header(file_path)

    with open("tokenizer.json", "r") as f:
        tokenizer_json = f.read()

    model = tiktoken.get_encoding("tokenizer.json")
    # model = tiktoken.Encoding.from_json(tokenizer_json)

    # model_path = "tokenizer.model"
    # pat_str = r"(?i:'s|'t|'re|'ve|'m|'ll|'d)|[^\r\n\p{L}\p{N}]?\p{L}+|\p{N}{1,3}| ?[^\s\p{L}\p{N}]+[\r\n]*|\s*[\r\n]+|\s+(?!\S)|\s+"  # noqa: E501
    # mergeable_ranks = load_tiktoken_bpe(model_path)
    # model = tiktoken.Encoding(
    #     name=Path(model_path).name,
    #     pat_str=pat_str,
    #     mergeable_ranks=mergeable_ranks,
    #     special_tokens=dict(),
    # )
    tok_ids = model.encode(monologue_otis)
    print(tok_ids)
    for tok_id in tok_ids:
        print(model.decode([tok_id]), end=", ")

    print()

"""
EXPECTED
[83493, 11, 9189, 3665, 89, 11, 41091, 4864, 841, 14425, 285, 6502, 934, 44996, 379, 107902, 409, 51651, 6033, 409, 60014, 85, 4105, 6671, 13, 6178, 72, 11, 4502, 4864, 3567, 2852, 9517, 12023, 7643, 17536, 
75804, 91443, 9952, 9189, 11, 4864, 5534, 2852, 1744, 272, 22117, 294, 529, 370, 541, 951, 27918, 11, 951, 47104, 7930, 296, 529, 546, 8541, 84, 1208, 1925, 11, 25692, 85815, 3869, 653, 4545, 39723, 4864, 84
1, 17258, 100063, 6502, 11, 39723, 503, 33777, 2629, 285, 74206, 46206, 41091, 13, 19421, 272, 22117, 79380, 2917, 53819, 409, 513, 13510, 1744, 3625, 706, 2402, 11, 3625, 27918, 11809, 306, 6316, 52087, 804
7, 1981, 4366, 346, 1744, 42676, 389, 264, 514, 733, 60768, 409, 1208, 14896, 11, 42676, 389, 264, 514, 733, 60768, 409, 1208, 14896, 14707, 2267, 635, 11, 514, 72006, 13033, 68, 11, 1370, 86633, 389, 841, 7
1451, 6502, 326, 529, 2295, 1092, 40095, 665, 3663, 11, 4864, 5534, 2852, 11, 514, 296, 8869, 404, 7930, 9189, 41675, 3869, 1860, 11967, 13, 1708, 1105, 3846, 308, 22117, 6502, 1647, 4865, 11, 22299, 4864, 5
14, 834, 2852, 39015, 11, 44829, 593, 41091, 8065, 32150, 556, 11, 503, 50808, 18757, 2652, 1880, 4864, 834, 16888, 72, 3869, 1208, 17536, 11, 4864, 25400, 834, 16888, 72, 11, 4864, 523, 5048, 1208, 17536, 1
1, 4864, 9279, 325, 1208, 17536, 1981, 14465, 841, 36731, 934, 77138, 414, 758, 19421, 1913, 54960, 11, 42676, 48488, 409, 47104, 75804, 91443, 757, 834, 306, 551, 12769, 34447, 4068, 66517, 2442, 84, 5019, 
39929, 20662, 3823, 13109, 949, 8345, 61651, 14707, 4864, 28130, 75871, 82, 25945, 71357, 11, 4864, 28130, 834, 1744, 272, 22117, 3846, 733, 60768, 409, 326, 77138, 414, 11, 3846, 733, 60768, 39587, 7930, 29
6, 40753, 17258, 784, 978, 75804, 91443, 3869, 9465, 59976, 265, 6316, 8246, 32118, 4919, 2428, 11, 10071, 2486, 467, 11, 7930, 829, 275, 11, 25692, 85815, 71357, 3869, 757, 48021, 8065, 2532, 409, 1208, 345
61, 94943, 11, 3869, 20028, 514, 1541, 11, 514, 1541, 409, 779, 72, 1981]

Mais, ,,  vous,  save, z, ,,  moi,  je,  ne,  cro, is,  pas,  qu, ’il,  y,  ait,  de,  bonne,  ou,  de,  mau, v, aise,  situation, .,  Mo, i, ,,  si,  je,  dev, ais,  ré, sumer,  ma,  vie,  aujourd, ’hui,  a
vec,  vous, ,,  je,  dir, ais,  que,  c, ’est,  d, ’, ab, ord,  des,  rencontres, ,,  des,  gens,  qui,  m, ’, ont,  tend, u,  la,  main, ,,  peut, -être,  à,  un,  moment,  où,  je,  ne,  pou, vais,  pas, ,
,  où,  j, ’é, ta, is,  seul,  chez,  moi, .,  Et,  c, ’est,  assez,  cur, ieux,  de,  se,  dire,  que,  les,  has, ards, ,,  les,  rencontres,  forg, ent,  une,  destin, ée, …,  Par, ce,  que,  quand,  on, 
 a,  le,  go, ût,  de,  la,  chose, ,,  quand,  on,  a,  le,  go, ût,  de,  la,  chose,  bien,  fa, ite, ,,  le,  beau,  gest, e, ,,  par, fois,  on,  ne,  trouve,  pas,  l, ’, inter, loc, uteur,  en,  face,
 ,,  je,  dir, ais, ,,  le,  m, iro, ir,  qui,  vous,  aide,  à,  av, ancer, .,  Al, ors,  ce,  n, ’est,  pas,  mon,  cas, ,,  comme,  je,  le,  dis, ais,  là, ,,  puis, que,  moi,  au,  contra, ire, ,,  j, 
’ai,  pu,  ;,  et,  je,  dis,  merc, i,  à,  la,  vie, ,,  je,  lui,  dis,  merc, i, ,,  je,  ch, ante,  la,  vie, ,,  je,  dan, se,  la,  vie, …,  Je,  ne,  suis,  qu, ’am, our,  !,  Et,  fin, alement, ,,  
quand,  beaucoup,  de,  gens,  aujourd, ’hui,  me,  dis, ent,  :,  «,  Mais,  comment,  fais, -t, u,  pour,  avoir,  cette,  human, ité,  ?,  »,  Eh,  bien,  je,  leur,  répond, s,  très,  simplement, ,,  je
,  leur,  dis,  que,  c, ’est,  ce,  go, ût,  de,  l, ’am, our, ,,  ce,  go, ût,  donc,  qui,  m, ’a,  pou, ss, é,  aujourd, ’hui,  à,  entre, prend, re,  une,  construction,  mé, can, ique, ,,  mais,  dem, 
ain, ,,  qui,  sa, it, ,,  peut, -être,  simplement,  à,  me,  mettre,  au,  service,  de,  la,  commun, auté, ,,  à,  faire,  le,  don, ,,  le,  don,  de,  so, i, …, 

ACTUAL
[83493, 11, 9189, 274, 402, 10333, 11, 41091, 4864, 841, 272, 299, 285, 6502, 934, 44996, 379, 107902, 409, 51651, 6033, 409, 296, 2933, 6723, 285, 68, 6671, 13, 386, 6870, 11, 4502, 4864, 294, 5230, 64, 285, 436, 978, 82, 372, 26
1, 7643, 17536, 75804, 91443, 9952, 9189, 11, 4864, 294, 404, 64, 285, 1744, 272, 22117, 294, 378, 247, 370, 269, 67, 951, 27918, 11, 951, 47104, 7930, 296, 378, 247, 263, 83, 259, 268, 1072, 1208, 1925, 11, 25692, 12, 5615, 10367
, 3869, 653, 4545, 297, 15273, 4864, 841, 281, 283, 6723, 285, 6502, 11, 297, 15273, 503, 33777, 2629, 285, 74206, 46206, 41091, 13, 19421, 272, 22117, 79380, 272, 324, 648, 2249, 409, 513, 13510, 1744, 3625, 305, 300, 277, 5469, 
11, 3625, 27918, 282, 269, 70, 268, 83, 6316, 52087, 8047, 1981, 393, 277, 346, 1744, 42676, 389, 264, 514, 733, 60768, 409, 1208, 14896, 11, 42676, 389, 264, 514, 733, 60768, 409, 1208, 14896, 14707, 282, 64, 275, 68, 11, 514, 72
006, 342, 68, 267, 68, 11, 281, 277, 831, 285, 389, 841, 71451, 6502, 326, 378, 247, 258, 83, 261, 385, 66, 332, 68, 324, 665, 3663, 11, 4864, 294, 404, 64, 285, 11, 514, 296, 72, 299, 404, 7930, 9189, 41675, 3869, 264, 85, 276, 6
6, 261, 13, 362, 75, 269, 82, 3846, 308, 22117, 6502, 1647, 4865, 11, 22299, 4864, 514, 294, 285, 64, 285, 326, 6496, 11, 281, 84, 285, 80, 361, 41091, 8065, 272, 263, 376, 2192, 265, 11, 503, 50808, 18757, 2652, 1880, 4864, 834, 
296, 261, 5979, 3869, 1208, 17536, 11, 4864, 25400, 834, 296, 261, 5979, 11, 4864, 272, 71, 276, 668, 1208, 17536, 11, 4864, 294, 276, 325, 1208, 17536, 1981, 14465, 841, 36731, 934, 378, 247, 309, 283, 81, 758, 19421, 282, 258, 6
4, 273, 76, 268, 83, 11, 42676, 48488, 409, 47104, 75804, 91443, 757, 294, 285, 268, 83, 551, 12769, 34447, 4068, 66517, 2442, 84, 5019, 39929, 20662, 305, 372, 276, 275, 978, 949, 8345, 61651, 14707, 4864, 28130, 436, 978, 79, 26
3, 5469, 490, 12416, 71357, 11, 4864, 28130, 834, 1744, 272, 22117, 3846, 733, 60768, 409, 326, 378, 247, 309, 283, 81, 11, 3846, 733, 60768, 39587, 7930, 296, 40753, 281, 283, 784, 978, 75804, 91443, 3869, 220, 268, 83, 265, 79, 
265, 303, 265, 6316, 8246, 296, 978, 66, 276, 24672, 361, 11, 10071, 294, 336, 64, 258, 11, 7930, 274, 64, 275, 11, 25692, 12, 5615, 10367, 71357, 3869, 757, 48021, 8065, 2532, 409, 1208, 272, 316, 76, 359, 64, 332, 978, 11, 3869,
 20028, 514, 1541, 11, 514, 1541, 409, 274, 6870, 1981, ]

[Mais, ,,  vous,  s, av, ez, ,,  moi,  je,  ne,  c, ro, is,  pas,  qu, ’il,  y,  ait,  de,  bonne,  ou,  de,  m, au, va, is, e,  situation, .,  M, oi, ,,  si,  je,  d, ev, a, is,  r, é, s, um, er,  ma,  vie,  aujourd, ’hui,  ave
  vous, ,,  je,  d, ir, a, is,  que,  c, ’est,  d, �, �, ab, or, d,  des,  rencontres, ,,  des,  gens,  qui,  m, �, �, on, t,  t, en, du,  la,  main, ,,  peut, -, ê, tre,  à,  un,  moment,  o, ù,  je,  ne,  p, ou, va, is,  pas, 
  o, ù,  j, ’é, ta, is,  seul,  chez,  moi, .,  Et,  c, ’est,  assez,  c, ur, ie, ux,  de,  se,  dire,  que,  les,  h, as, ar, ds, ,,  les,  rencontres,  f, or, g, en, t,  une,  destin, ée, …,  P, ar, ce,  que,  quand,  on,  a, 
e,  go, ût,  de,  la,  chose, ,,  quand,  on,  a,  le,  go, ût,  de,  la,  chose,  bien,  f, a, it, e, ,,  le,  beau,  g, e, st, e, ,,  p, ar, fo, is,  on,  ne,  trouve,  pas,  l, �, �, in, t, er, lo, c, ut, e, ur,  en,  face, ,
 je,  d, ir, a, is, ,,  le,  m, i, ro, ir,  qui,  vous,  aide,  à,  a, v, an, c, er, .,  A, l, or, s,  ce,  n, ’est,  pas,  mon,  cas, ,,  comme,  je,  le,  d, is, a, is,  l, à, ,,  p, u, is, q, ue,  moi,  au,  c, on, tr, ai, re
,,  j, ’ai,  pu,  ;,  et,  je,  dis,  m, er, ci,  à,  la,  vie, ,,  je,  lui,  dis,  m, er, ci, ,,  je,  c, h, an, te,  la,  vie, ,,  je,  d, an, se,  la,  vie, …,  Je,  ne,  suis,  qu, �, �, am, ou, r,  !,  Et,  f, in, a, le, m
en, t, ,,  quand,  beaucoup,  de,  gens,  aujourd, ’hui,  me,  d, is, en, t,  :,  «,  Mais,  comment,  fais, -t, u,  pour,  avoir,  cette,  h, um, an, it, é,  ?,  »,  Eh,  bien,  je,  leur,  r, é, p, on, ds,  tr, ès,  simplement
,,  je,  leur,  dis,  que,  c, ’est,  ce,  go, ût,  de,  l, �, �, am, ou, r, ,,  ce,  go, ût,  donc,  qui,  m, ’a,  p, ou, ss, é,  aujourd, ’hui,  à,  , en, t, re, p, re, nd, re,  une,  construction,  m, é, c, an, iq, ue, ,,  ma
,  d, em, a, in, ,,  qui,  s, a, it, ,,  peut, -, ê, tre,  simplement,  à,  me,  mettre,  au,  service,  de,  la,  c, om, m, un, a, ut, é, ,,  à,  faire,  le,  don, ,,  le,  don,  de,  s, oi, …, ]
"""
