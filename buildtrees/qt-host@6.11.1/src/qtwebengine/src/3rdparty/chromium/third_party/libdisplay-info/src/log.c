#include "log.h"

void
_di_logger_va_add_failure(struct di_logger *logger, const char fmt[], va_list args)
{
	if (!logger->initialized) {
		if (ftell(logger->f) > 0) {
			fprintf(logger->f, "\n");
		}
		fprintf(logger->f, "%s:\n", logger->section);
		logger->initialized = true;
	}

	fprintf(logger->f, "  ");
	vfprintf(logger->f, fmt, args);
	fprintf(logger->f, "\n");
}
