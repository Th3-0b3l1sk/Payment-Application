/* -------------------------------------
* Author: Mahmoud Abdullah
* EgFWD Professional Embedded Systems
* Github: https://github.com/Th3-0b3l1sk
* --------------------------------------
*/

#define _CRT_SECURE_NO_WARNINGS

#include <string.h>	// strlen, strcpy
#include <stdio.h>	// printf, scanf_s
#include <ctype.h>
#include <stdlib.h>
#include "card.h"

#ifdef _DEBUG
#define D_LOG(x)   printf(x);
#else
#define D_LOG(x)    ;
#endif // _DEBUG

void clearStdin(char* buffer)
{
	char* lf = strchr(buffer, '\n');
	if (lf == NULL) {
		while (fgetc(stdin) != '\n')
			;	// null statement; ignore the rest of the stream
	}
	else
		*lf = '\0';
}

EN_cardError_t getCardHolderName(ST_cardData_t* cardData)
{
#define MAX_NAME_LEN 24
#define MIN_NAME_LEN 20
	
	// check if cardData is a null pointer
	if (cardData == NULL)
		return WRONG_NAME;	

	register char* cardName = cardData->cardHolderName;

	printf("Please enter your name: ");
	
	// get the card name, maximum length 24
	uint8_t tmpBuffer[30] = { 0 };
	if (fgets(tmpBuffer, 30, stdin) == NULL) {		// reads up to n - 1 (24 max) then appends '\0'
		D_LOG("Failed to read card holder name.\n");
		return WRONG_NAME;
	}

	// empty the stdin buffer
	clearStdin(tmpBuffer);
	
	// check the name length
	if ((strlen(tmpBuffer ) > MAX_NAME_LEN) || (strlen(tmpBuffer) < MIN_NAME_LEN))
		return WRONG_NAME;
	else {
		memcpy_s(cardName, MAX_NAME_LEN, tmpBuffer, MAX_NAME_LEN);
		cardName[MAX_NAME_LEN] = '\0';
		return C_OK;
	}
	
	return C_OK;

#undef MAX_NAME_LEN
#undef MIN_NAME_LEN
}

EN_cardError_t getCardExpiryDate(ST_cardData_t* cardData)
{
#define MAX_EXP_LEN 5

	if (cardData == NULL)
		return WRONG_EXP_DATE;

	printf("Please enter the card expiry date in the form MM/YY: ");

	register uint8_t* cardExpiry = cardData->cardExpirationData;

	fgets(cardExpiry, MAX_EXP_LEN + 1, stdin);
	
	clearStdin(cardExpiry);

	if (strlen(cardExpiry) != MAX_EXP_LEN)
		return WRONG_EXP_DATE;

	// the expected format is MM/YY. Index 2 should be a /
	if (cardExpiry[2] != '/')
		return WRONG_EXP_DATE;

	uint8_t isValid = 1;
	// check if MM and YY are valid numbers
	for (int i = 0; i < MAX_EXP_LEN; i++) {
		if (cardExpiry[i] == '/') {
			cardExpiry[i] = '\0';	// to limit the string in testing month(strtol)
			continue;
		}
			
		if (cardExpiry[i] < 0x30 || cardExpiry[i] > 0x39) {		// hex zero and hex 9
			isValid = 0;
			break;
		}
	}

	if (!isValid)
		return WRONG_EXP_DATE;

	// check if MM > 12 or MM < 1
	int8_t month = strtol(cardExpiry, NULL, 10);
	if (month > 12 || month < 1)
		return WRONG_EXP_DATE;	

	cardExpiry[2] = '/';	

	return C_OK;

#undef MAX_EXP_LEN
}

EN_cardError_t getCardPAN(ST_cardData_t* cardData)
{
#define MAX_PAN_LEN 19
#define MIN_PAN_LEN 16

	if (cardData == NULL)
		return WRONG_PAN;

	printf("Please enter your PAN: ");

	register uint8_t* cardPan = cardData->primaryAccountNumber;
	uint8_t tmpBuffer[MAX_PAN_LEN + 10] = { 0 };

	fgets(tmpBuffer, sizeof(tmpBuffer), stdin);
	
	clearStdin(tmpBuffer);

	if ((strlen(tmpBuffer) > MAX_PAN_LEN ) || (strlen(tmpBuffer) < MIN_PAN_LEN))
		return WRONG_PAN;

	memcpy_s(cardPan, MAX_PAN_LEN, tmpBuffer, MAX_PAN_LEN);

	for (int i = 0; i < strlen(cardPan); i++)
		if (!isalnum(cardPan[i])) {
			return WRONG_PAN;
		}
	return C_OK;

#undef MAX_PAN_LEN
#undef MIN_PAN_LEN 
}
