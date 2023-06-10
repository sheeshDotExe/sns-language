#include "interpretType.h"

void fillTypes(struct CommonTypes *cTypes, int *types, unsigned int length, struct ProcessState* processState)
{
	cTypes->length = length;
	cTypes->codes = (int *)malloc(length * sizeof(int));
	for (int i = 0; i < length; i++)
	{
		cTypes->codes[i] = types[i];
	}
}

struct CommonTypes getValidTypes(char **value, unsigned int *lengthP, struct ProcessState* processState)
{
	struct CommonTypes types;

	int validTypes[NUMBER_OF_TYPES];
	unsigned int length = 0;

	if (*lengthP == 1)
	{
		validTypes[length] = Char_c;
		length++;
	}

	validTypes[length] = String_c;
	length++;
	if (isString(*value, *lengthP, processState))
	{
		if (*lengthP == 3)
		{
			validTypes[length] = Char_c;
			length++;
		}
		fillTypes(&types, validTypes, length, processState);
		return types;
	}

	int Bool_v = stringToBool(*value, processState);
	if (Bool_v != -1)
	{
		*lengthP = 1;
		free(*value);
		*value = (char *)malloc(2 * sizeof(char));
		sprintf(*value, "%d", Bool_v);

		validTypes[length] = Int_c;
		length++;
		validTypes[length] = Float_c;
		length++;

		fillTypes(&types, validTypes, length, processState);
		return types;
	}

	if (!isNum(*value, *lengthP, processState))
	{
		validTypes[length] = Int_c;
		length++;
	}

	if (!isFloat(*value, *lengthP, processState))
	{
		validTypes[length] = Float_c;
		length++;
	}

	fillTypes(&types, validTypes, length, processState);
	return types;
}

struct Var *generateVarFromString(char *value, unsigned int length, struct ProcessState* processState)
{

	char **valueP = &value;

	struct CommonTypes types = getValidTypes(valueP, &length, processState);

	struct Var *ret = generateVar(types.codes, types.length, "unnamed", *valueP, (struct Param *)NULL, processState);

	free(types.codes);

	return ret;
}