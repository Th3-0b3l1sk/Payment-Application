/* -------------------------------------
* Author: Mahmoud Abdullah
* EgFWD Professional Embedded Systems
* Github: https://github.com/Th3-0b3l1sk
* --------------------------------------
*/


#define _CRT_SECURE_NO_WARNINGS

#include "tests.h"
#include "app.h"

#include <stdio.h>
#include <stdlib.h>	// exit
#include <string.h>
#include <ctype.h>


////////////////////////////////////////////////////////////////////
//	Global exported objects
////////////////////////////////////////////////////////////////////
uint16_t g_lastSequence		= 0;
float    g_maxTransaction   = 0.f;

void initSettings()
{
	FILE* db = fopen("Settings.txt", "r");

	fscanf(db, "%f", &g_maxTransaction);

	fclose(db);

	uint8_t buffer[100];
	db = fopen("Transactions DB.txt", "r");
	while (fgets(buffer, 100, db)) {
		// skip comments and empty lines
		if (buffer[0] == '#' || isspace(buffer[0]))
			continue;

		uint8_t* sep = strchr(buffer, ',');
		if (sep == NULL)
			continue;

		*sep = '\0';
		g_lastSequence = (uint16_t)strtol(buffer, NULL, 10);
	}

	fclose(db);
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////


void appStart(void)
{
	// read the max transaction amount and 
	// last transaction index.
	initSettings();

	ST_transaction_t transaction = { 0 };

	// card info
	getCardHolderName(&transaction.cardHolderData);
	getCardExpiryDate(&transaction.cardHolderData);
	getCardPAN(&transaction.cardHolderData);

	// terminal
	EN_terminalError_t tError = T_OK;

	tError = isValidCardPAN(&transaction.cardHolderData);
	if (tError == INVALID_CARD) {
		printf("Invalid card.\n");
		return;
	}

	setMaxAmount(&transaction.terminalData);

	getTransactionDate(&transaction.terminalData);
	tError = isCardExpired(transaction.cardHolderData, transaction.terminalData);
	if (tError == EXPIRED_CARD) {
		printf("Expired card!\n");
		return;
	}

	getTransactionAmount(&transaction.terminalData);
	tError = isBelowMaxAmount(&transaction.terminalData);
	if (tError == EXCEED_MAX_AMOUNT) {
		printf("It is currently not possible to withdraw this amount.\n");	
		return;
	}

	// server
	if (isValidAccount(&transaction.cardHolderData) != S_OK) {
		printf("Invalid account.\n");
		return;
	}

	if (isAmountAvailable(&transaction.terminalData) != S_OK) {
		printf("Not enough money!\n");
		return;
	}

	if (recieveTransactionData(&transaction) != S_OK) {
		printf("Something went wrong, the transaction wasn't completed.");
		return;
	}
	
	printf("Transaction completed successfully!\nThanks for using our bank.\n");


	return;

}

int main()
{
	appStart();

#pragma region CARD_TESTS
	//test_getCardHolderName();
	//test_getCardExpiryDate();
	//test_getCardPAN();
#pragma endregion

#pragma region TERMINAL_TESTS
	//test_getTransactionDate();
	//test_isCardExpired();
	//test_isValidCardPAN();
	//test_isBelowMaxAmount();
	//test_getTransactionAmount();
#pragma endregion
	
#pragma region SERVER_TESTS
	//test_recieveTransactionData();
	//test_isValidAccount();
	//test_isAmountAvailable();
	//test_saveTransaction();
	//test_getTransaction();

#pragma endregion

#pragma region APP_TESTS
	//test_approvedTransaction();
	//test_exceedAmount();
	//test_insufficientFund();
	//test_expiredCard();
	//test_invalidUser();
#pragma endregion

	getchar();
}

