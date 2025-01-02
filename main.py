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
[83493, 11, 85, 283, 82, 82, 402, 10333, 11, 6489, 72, 3841, 818, 66, 299, 285, 79, 300, 447, 321, 88, 64, 275, 451, 65, 263, 818, 283, 451, 1764, 84, 6723, 285, 68, 82, 275, 84, 266, 72, 263, 13, 44, 6870, 
11, 6455, 3841, 451, 6723, 285, 81, 165, 82, 372, 261, 1764, 85, 648, 2933, 73, 283, 6634, 71, 2005, 402, 762, 85, 283, 82, 11, 3841, 67, 404, 64, 285, 80, 361, 66, 68, 267, 67, 370, 269, 67, 67, 288, 265, 1
031, 263, 83, 265, 82, 11, 67, 288, 70, 268, 82, 447, 72, 76, 263, 83, 83, 268, 1072, 4355, 1764, 258, 11, 375, 332, 12, 166, 10367, 156, 359, 76, 316, 268, 83, 78, 181, 3841, 818, 79, 283, 6723, 285, 79, 30
0, 11, 78, 181, 73, 165, 2629, 285, 325, 360, 331, 10333, 6489, 72, 13, 32960, 66, 68, 267, 300, 325, 89, 66, 324, 648, 2249, 451, 325, 8747, 265, 80, 361, 273, 82, 71, 300, 277, 5469, 11, 273, 82, 265, 1031
, 263, 83, 265, 82, 69, 269, 70, 268, 83, 359, 68, 451, 267, 258, 165, 68, 47, 277, 346, 80, 361, 447, 276, 67, 263, 64, 273, 3427, 183, 83, 451, 4355, 331, 78, 325, 11, 447, 276, 67, 263, 64, 273, 3427, 183
, 83, 451, 4355, 331, 78, 325, 8385, 268, 3716, 275, 68, 11, 273, 1395, 2933, 713, 267, 68, 11, 79, 277, 831, 285, 263, 818, 376, 283, 588, 79, 300, 75, 258, 83, 261, 385, 66, 332, 68, 324, 268, 3716, 346, 1
1, 3841, 67, 404, 64, 285, 11, 273, 8318, 299, 404, 447, 72, 85, 283, 82, 64, 307, 68, 156, 402, 276, 66, 261, 13, 2149, 269, 82, 346, 77, 68, 267, 79, 300, 76, 263, 66, 300, 11, 66, 316, 2727, 3841, 273, 67
, 285, 64, 285, 75, 156, 11, 5701, 285, 80, 361, 6489, 72, 2933, 66, 263, 376, 2192, 265, 11, 73, 2192, 5701, 26, 295, 3841, 67, 285, 76, 261, 5979, 156, 4355, 85, 648, 11, 3841, 75, 2005, 67, 285, 76, 261, 
5979, 11, 3841, 331, 276, 668, 4355, 85, 648, 11, 3841, 67, 276, 325, 4355, 85, 648, 30854, 818, 28149, 285, 447, 309, 283, 81, 0, 32960, 69, 258, 64, 273, 76, 268, 83, 11, 447, 276, 67, 1395, 64, 1791, 283,
 79, 451, 70, 268, 82, 2933, 73, 283, 6634, 71, 2005, 2727, 67, 285, 268, 83, 25, 104, 30635, 285, 66, 316, 76, 268, 83, 3716, 285, 2442, 84, 79, 283, 81, 402, 78, 404, 66, 295, 668, 71, 372, 276, 275, 165, 
30, 119, 36, 71, 8385, 268, 3841, 273, 324, 81, 165, 79, 263, 5469, 376, 164, 82, 82, 318, 79, 273, 76, 268, 83, 11, 3841, 273, 324, 67, 285, 80, 361, 66, 68, 267, 346, 3427, 183, 83, 451, 75, 309, 283, 81, 
11, 346, 3427, 183, 83, 67, 263, 66, 447, 72, 76, 64, 79, 283, 784, 165, 2933, 73, 283, 6634, 71, 2005, 156, 268, 83, 265, 79, 265, 303, 265, 359, 68, 66, 263, 267, 2739, 302, 72, 263, 76, 165, 66, 276, 2467
2, 361, 11, 1764, 285, 67, 336, 64, 258, 11, 447, 72, 9258, 275, 11, 375, 332, 12, 166, 10367, 82, 318, 79, 273, 76, 268, 83, 156, 2727, 76, 295, 83, 265, 2933, 82, 261, 85, 292, 68, 451, 4355, 66, 316, 76, 
359, 64, 332, 165, 11, 156, 69, 2192, 265, 273, 67, 263, 11, 273, 67, 263, 451, 708, 72, ]

[Mais, ,, v, ou, s, s, av, ez, ,, mo, i, je, ne, c, ro, is, p, as, qu, il, y, a, it, de, b, on, ne, ou, de, ma, u, va, is, e, s, it, u, at, i, on, ., M, oi, ,, si, je, de, va, is, r, é, s, um, er, ma, v, ie,
 au, j, ou, rd, h, ui, av, ec, v, ou, s, ,, je, d, ir, a, is, q, ue, c, e, st, d, ab, or, d, d, es, re, nc, on, t, re, s, ,, d, es, g, en, s, qu, i, m, on, t, t, en, du, la, ma, in, ,, pe, ut, -, ê, tre, à, 
un, m, om, en, t, o, ù, je, ne, p, ou, va, is, p, as, ,, o, ù, j, é, ta, is, se, ul, ch, ez, mo, i, ., Et, c, e, st, as, se, z, c, ur, ie, ux, de, se, di, re, q, ue, le, s, h, as, ar, ds, ,, le, s, re, nc, o
n, t, re, s, f, or, g, en, t, un, e, de, st, in, é, e, P, ar, ce, q, ue, qu, an, d, on, a, le, go, û, t, de, la, ch, o, se, ,, qu, an, d, on, a, le, go, û, t, de, la, ch, o, se, bi, en, fa, it, e, ,, le, be,
 au, ge, st, e, ,, p, ar, fo, is, on, ne, tr, ou, ve, p, as, l, in, t, er, lo, c, ut, e, ur, en, fa, ce, ,, je, d, ir, a, is, ,, le, mi, ro, ir, qu, i, v, ou, s, a, id, e, à, av, an, c, er, ., Al, or, s, ce,
 n, e, st, p, as, m, on, c, as, ,, c, om, me, je, le, d, is, a, is, l, à, ,, pu, is, q, ue, mo, i, au, c, on, tr, ai, re, ,, j, ai, pu, ;, et, je, d, is, m, er, ci, à, la, v, ie, ,, je, l, ui, d, is, m, er, 
ci, ,, je, ch, an, te, la, v, ie, ,, je, d, an, se, la, v, ie, Je, ne, su, is, qu, am, ou, r, !, Et, f, in, a, le, m, en, t, ,, qu, an, d, be, a, uc, ou, p, de, g, en, s, au, j, ou, rd, h, ui, me, d, is, en,
 t, :, «, Ma, is, c, om, m, en, t, fa, is, -t, u, p, ou, r, av, o, ir, c, et, te, h, um, an, it, é, ?, », E, h, bi, en, je, le, ur, r, é, p, on, ds, tr, è, s, s, im, p, le, m, en, t, ,, je, le, ur, d, is, q,
 ue, c, e, st, ce, go, û, t, de, l, am, ou, r, ,, ce, go, û, t, d, on, c, qu, i, m, a, p, ou, ss, é, au, j, ou, rd, h, ui, à, en, t, re, p, re, nd, re, un, e, c, on, st, ru, ct, i, on, m, é, c, an, iq, ue, ,
, ma, is, d, em, a, in, ,, qu, i, sa, it, ,, pe, ut, -, ê, tre, s, im, p, le, m, en, t, à, me, m, et, t, re, au, s, er, v, ic, e, de, la, c, om, m, un, a, ut, é, ,, à, f, ai, re, le, d, on, ,, le, d, on, de,
 so, i, ]
"""
