#include "../core/config.h"
#include "../core/logging.h"
#include "../core/safetensors.h"
#include "../tokenizer/tokenizer.h"
#include "matrix.h"
#include "model.h"
#include <fcntl.h>
#include <pcre.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

// static char *monologue_otis = "Mais, vous savez, moi je ne crois pas qu’il y ait de bonne ou de "
//                               "mauvaise situation. Moi, si je devais résumer ma "
//                               "vie aujourd’hui avec vous, je dirais que c’est d’abord des rencontres, "
//                               "des gens qui m’ont tendu la main, "
//                               "peut-être à un moment où je ne pouvais pas, où j’étais seul chez moi. "
//                               "Et c’est assez curieux de se dire que les "
//                               "hasards, les rencontres forgent une destinée… Parce que quand on a le "
//                               "goût de la chose, quand on a le goût de la "
//                               "chose bien faite, le beau geste, parfois on ne trouve pas "
//                               "l’interlocuteur en face, je dirais, le miroir qui vous "
//                               "aide à avancer. Alors ce n’est pas mon cas, comme je le disais là, "
//                               "puisque moi au contraire, j’ai pu ; et je dis "
//                               "merci à la vie, je lui dis merci, je chante la vie, je danse la vie… "
//                               "Je ne suis qu’amour ! Et finalement, quand "
//                               "beaucoup de gens aujourd’hui me disent : « Mais comment fais-tu pour "
//                               "avoir cette humanité ? » Eh bien je leur "
//                               "réponds très simplement, je leur dis que c’est ce goût de l’amour, ce "
//                               "goût donc qui m’a poussé aujourd’hui à "
//                               "entreprendre une construction mécanique, mais demain, qui sait, "
//                               "peut-être simplement à me mettre au service de la "
//                               "communauté, à faire le don, le don de soi…";

static char *monologue_otis = "aujourd’hui qui m’ont";
static const char *st_file_path = "model2.safetensors";
static const char *tok_file_path = "resources/tokenizer.model";
static const char *config_file = "config.json";

int
main()
{
    Tokenizer *tokenizer = Tokenizer_new(tok_file_path);

    return 0;
    Safetensors *st = Safetensors_new(st_file_path);
    Safetensors_print(st);

    Config *config = Config_new(config_file);
    Model *model = Model_new(st, config);

    LOG_INFO("Encoding Otis monologue...");
    int *token_ids;
    int token_count;
    if (Tokenizer_encode(tokenizer, monologue_otis, &token_ids, &token_count) != OK)
    {
        LOG_ERROR("Error encoding tokens");
        return 1;
    }

    printf("[");
    for (int i = 0; i < token_count; i++)
    {
        printf("%d, ", token_ids[i]);
    }
    printf("]\n\n");
    printf("[");
    for (int i = 0; i < token_count; i++)
    {
        printf("%s, ", Tokenizer_decode_single(tokenizer, token_ids[i]));
    }
    printf("]\n");

    LOG_INFO("Forwarding model...");
    Matrix *output = Model_forward(model, token_ids, token_count);
    Matrix_print(output, 10);

    Config_free(config);

    return 0;
}
