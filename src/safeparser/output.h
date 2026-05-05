#ifndef CALLM_SAFEPARSER_OUTPUT_H
#define CALLM_SAFEPARSER_OUTPUT_H

#include "metadata_view.h"
#include "query_options.h"

CallmStatusCode Output_print(const MetadataView *view, QueryOutputMode output_mode);

#endif
