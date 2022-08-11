/* -------------------------------------
* Author: Mahmoud Abdullah
* EgFWD Professional Embedded Systems
* Github: https://github.com/Th3-0b3l1sk
* --------------------------------------
*/


#ifndef CARD_HDR_GRD
#define CARD_HDR_GRD

#include <stdint.h>	// for uint8_t


////////////////////////////////////////////////////////////////////
//	Type Definitions
////////////////////////////////////////////////////////////////////

typedef enum EN_cardError_t
{
	C_OK,
	WRONG_NAME,
	WRONG_EXP_DATE,
	WRONG_PAN

} EN_cardError_t;

typedef struct ST_cardData_t
{
	uint8_t cardHolderName[25];
	uint8_t primaryAccountNumber[20];
	uint8_t cardExpirationData[6];		// format: MM/YY

} ST_cardData_t;

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//	Function Declarations
////////////////////////////////////////////////////////////////////

EN_cardError_t getCardHolderName(ST_cardData_t* cardData);
EN_cardError_t getCardExpiryDate(ST_cardData_t* cardData);
EN_cardError_t getCardPAN(ST_cardData_t* cardData);

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////


void test_getCardHolderName();
void test_getCardExpiryDate();
void test_getCardPAN();


#endif
