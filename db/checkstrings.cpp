// To use, remove the binary, then "make checkstrings"
// Could say "all is well" when not, if some operators are deleted and the
// same number added

#include "operstrings.h"
#include "operator.h"

#include <cstdio>
#include <cstring>

#define checkpoint(x) { x, #x }
static const struct {
	size_t index;
	const char *name;
} checklist[] = {
	checkpoint(opFPlusd),
	checkpoint(opSQRTd),
	checkpoint(opGtrEqUns),
	checkpoint(opTargetInst),
	checkpoint(opList),
	checkpoint(opMachFtr),
	checkpoint(opFpop),
	checkpoint(opExecute),
	checkpoint(opWildStrConst),
	checkpoint(opAnull),
};

int main()
{
	if (sizeof operStrings / sizeof *operStrings == opNumOf) {
		printf("All is correct\n");
		return 0;
	}

	for (size_t i = 0; i < sizeof checklist / sizeof *checklist; ++i) {
		if (strcmp(operStrings[checklist[i].index], checklist[i].name) != 0) {
			printf("Error at or before %s\n", checklist[i].name);
			return 1;
		}
	}
	printf("Error near the end\n");
	return 1;
}
