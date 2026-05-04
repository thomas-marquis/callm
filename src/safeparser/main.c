#include "metadata_view.h"
#include "output.h"
#include "query_options.h"

#include "../core/safetensors.h"

int
main(int argc, char **argv)
{
    QueryOptions options;
    QueryOptions_init(&options);

    if (QueryOptions_parse(argc, argv, &options) != OK)
    {
        QueryOptions_print_help();
        QueryOptions_free(&options);
        return 1;
    }

    if (options.show_help)
    {
        QueryOptions_print_help();
        QueryOptions_free(&options);
        return 0;
    }

    Safetensors *safetensors = Safetensors_new(options.file_path);

    MetadataView source;
    MetadataView filtered;

    if (MetadataView_build(safetensors, &source) != OK)
    {
        printerr("Failed to parse tensor metadata from '%s'\n", options.file_path);
        Safetensors_free(safetensors);
        QueryOptions_free(&options);
        return 1;
    }

    if (MetadataView_filter(&source, &options, &filtered) != OK)
    {
        printerr("Failed to apply filters\n");
        MetadataView_free(&source);
        Safetensors_free(safetensors);
        QueryOptions_free(&options);
        return 1;
    }

    MetadataView_sort(&filtered, options.sort_by);

    if (Output_print(&filtered) != OK)
    {
        printerr("Failed to print metadata\n");
        MetadataView_free(&filtered);
        MetadataView_free(&source);
        Safetensors_free(safetensors);
        QueryOptions_free(&options);
        return 1;
    }

    MetadataView_free(&filtered);
    MetadataView_free(&source);
    Safetensors_free(safetensors);
    QueryOptions_free(&options);
    return 0;
}
