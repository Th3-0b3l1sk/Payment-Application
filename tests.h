#ifndef TESTS_HDR_GRD
#define TESTS_HDR_GRD

#include "server.h"
#include "card.h"
#include "terminal.h"
#include "app.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////
//	Server.h
////////////////////////////////////////////////////////////////////

void test_recieveTransactionData()
{
	printf("=====================\n");
	printf("test_recieveTransactionData\n");
	printf("=====================\n");

	EN_transState_t result = APPROVED;
	ST_transaction_t transaction = { 0 };

	// set expiry date
	memcpy(transaction.cardHolderData.cardExpirationData, "05/26", 5);

	// case1: account doesnt exist
	// set username
	memcpy(transaction.cardHolderData.cardHolderName, "ThiefIm Possible", 17);

	// set pan
	memcpy(transaction.cardHolderData.primaryAccountNumber, "5162346746071729", 17);

	// set amount 
	transaction.terminalData.transAmount = 75000.123f;

	// process the transaction
	result = recieveTransactionData(&transaction);
	if (result != DECLINED_STOLEN_CARD) {
		printf("test:invalidUser failed.\n");
		return;
	}

	printf("case1 passed.\n");

	// case2: not enough credit
	// set username
	memcpy(transaction.cardHolderData.cardHolderName, "PoorMan NoMoney", 16);

	// set pan
	memcpy(transaction.cardHolderData.primaryAccountNumber, "5516361478373915", 17);

	// set amount 
	transaction.terminalData.transAmount = 75000.123f;

	// process the transaction
	result = recieveTransactionData(&transaction);
	if (result != DECLINED_INSUFFICIENT_FUND) {
		printf("test:insufficientFund failed.\n");
		return;
	}

	printf("case2 passed.\n");

	// case3: saving transaction[failure]

	// process the transaction
	result = recieveTransactionData(NULL);
	if (result != INTERNAL_SERVER_ERROR) {
		printf("test:approvedTransaction failed.\n");
		return;
	}

	printf("case3 passed.\n");

	// case4: saving transaction[success]
	// set username
	memcpy(transaction.cardHolderData.cardHolderName, "KingMufasa OfJungle", 20);

	// set pan
	memcpy(transaction.cardHolderData.primaryAccountNumber, "79927398713", 12);

	// set amount 
	transaction.terminalData.transAmount = 50000.123f;

	// process the transaction
	result = recieveTransactionData(&transaction);
	if (result != 0) {
		printf("test:approvedTransaction failed.\n");
		return;
	}

	printf("case4 passed.\n");


	printf("=========\n");
	printf("[passed].\n");
	printf("=========\n");
}

void test_isValidAccount()
{
	printf("=====================\n");
	printf("test_isValidAccount\n");
	printf("=====================\n");

	EN_serverEerror_t result = S_OK;
	ST_transaction_t transaction = { 0 };

	// case1: the PAN doesnt exist
	// set pan
	memcpy(transaction.cardHolderData.primaryAccountNumber, "5162346746071729", 17);

	result = isValidAccount(&transaction.cardHolderData);
	if (result != DECLINED_STOLEN_CARD) {
		printf("test_isValidAccount failed.\n");
		return;
	}

	printf("case1 passed.\n");

	// case2: the PAN exists
	memcpy(transaction.cardHolderData.primaryAccountNumber, "79927398713", 12);

	result = isValidAccount(&transaction.cardHolderData);
	if (result != S_OK) {
		printf("test_isValidAccount failed.\n");
		return;
	}

	printf("case2 passed.\n");

	printf("=========\n");
	printf("[passed].\n");
	printf("=========\n");
}

void test_isAmountAvailable()
{
	printf("=====================\n");
	printf("test_isAmountAvailable\n");
	printf("=====================\n");

	EN_serverEerror_t result = S_OK;
	ST_transaction_t transaction = { 0 };

	// set max amount 
	transaction.terminalData.maxTransAmount = 80000.0f;	// [available fund]

	// case1: amount available
	// set username
	strcpy(transaction.cardHolderData.cardHolderName, "TimonTheMagnificent");

	// set expiry date
	strcpy(transaction.cardHolderData.cardExpirationData, "05/26");

	// set pan
	strcpy(transaction.cardHolderData.primaryAccountNumber, "5112667210810487");

	// set amount 
	transaction.terminalData.transAmount = 75000.123f;
	
	result = isAmountAvailable(&transaction.terminalData);
	if (result != S_OK) {
		printf("test_isAmountAvailable failed.\n");
		return;
	}

	printf("case1 passed.\n");

	// case2: amount unavailable
	// set username
	strcpy(transaction.cardHolderData.cardHolderName, "PoorMan NoMoney");

	// set expiry date
	strcpy(transaction.cardHolderData.cardExpirationData, "05/26");

	// set pan
	strcpy(transaction.cardHolderData.primaryAccountNumber, "5516361478373915");

	// set amount 
	transaction.terminalData.transAmount = 190000;

	result = isAmountAvailable(&transaction.terminalData);
	if (result != DECLINED_INSUFFICIENT_FUND) {
		printf("test_isAmountAvailable failed.\n");
		return;
	}

	printf("case2 passed.\n");

	printf("=========\n");
	printf("[passed].\n");
	printf("=========\n");
}

void test_saveTransaction()
{
	printf("=====================\n");
	printf("test_isAmountAvailable\n");
	printf("=====================\n");

	EN_serverEerror_t result = S_OK;
	ST_transaction_t transaction = { 0 };

	// set card name
	memcpy(transaction.cardHolderData.cardHolderName, "TestingSavingTrans", 19);

	// set PAN
	memcpy(transaction.cardHolderData.primaryAccountNumber, "79927398713", 12);

	// set expiry date
	memcpy(transaction.cardHolderData.cardExpirationData, "05/26", 5);

	// set amount 
	transaction.terminalData.transAmount = 50000.123f;
	
	// set date
	memcpy(transaction.terminalData.transactionDate, "05/05/1979", 11);

	// set state
	transaction.transState = DECLINED_STOLEN_CARD;

	result = saveTransaction(&transaction);
	if (result != S_OK) {
		printf("test_isAmountAvailable failed.\n");
		return;
	}

	printf("=========\n");
	printf("[passed].\n");
	printf("=========\n");
}

void test_getTransaction()
{
	printf("=====================\n");
	printf("test_getTransaction\n");
	printf("=====================\n");

	EN_serverEerror_t result = S_OK;
	ST_transaction_t transaction = { 0 };

	// case1: existing transaction
	uint8_t seqNumber= 1;

	result = getTransaction(seqNumber, &transaction);
	if (result != S_OK) {
		printf("test_getTransaction failed.\n");
		return;
	}

	/* dump the transaction */

	printf("transaction: %d,%s,%s,%s,%.3f,%d\n",
		transaction.transactionSequenceNumber,
		transaction.cardHolderData.cardHolderName,
		transaction.cardHolderData.primaryAccountNumber,
		transaction.cardHolderData.cardExpirationData,
		transaction.terminalData.transAmount,
		transaction.transState);

	printf("case1 passed.\n");

	// case2: non-existing transaction
	seqNumber = -1;
	result = getTransaction(seqNumber, &transaction);
	if (result != TRANSACTION_NOT_FOUND) {
		printf("test_getTransaction failed.\n");
		return;
	}

	printf("case2 passed.\n");

	printf("=========\n");
	printf("[passed].\n");
	printf("=========\n");
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//	Card.h
////////////////////////////////////////////////////////////////////

void test_getCardHolderName()
{
	printf("=====================\n");
	printf("%s", "test_getCardHolderName\n");
	printf("=====================\n");

	EN_cardError_t result = C_OK;
	ST_cardData_t cardData = { 0 };

	// case1: name within 20, 24 bytes, test data KingMufasa OfTheJungle len 22
	result = getCardHolderName(&cardData);
	if (result != C_OK) {
		printf("test_getCardHolderName failed.\n");
		return;
	}

	printf("case1 passed.\n");

	// case2: name less than 20 bytes, test data KingMufasa len 10
	result = getCardHolderName(&cardData);
	if (result != WRONG_NAME) {
		printf("test_getCardHolderName failed.\n");
		return;
	}

	printf("case2 passed.\n");

	// case3: name more than 24 bytes, test data InTheJungle oTheMightyJungle len 28
	result = getCardHolderName(&cardData);
	if (result != WRONG_NAME) {
		printf("test_getCardHolderName failed.\n");
		return;
	}

	printf("case3 passed.\n");

	printf("=========\n");
	printf("[passed].\n");
	printf("=========\n");

	return;
}

void test_getCardExpiryDate()
{
	printf("=====================\n");
	printf("%s", "test_getCardExpiryDate\n");
	printf("=====================\n");

	EN_cardError_t result = C_OK;
	ST_cardData_t cardData = { 0 };

	// case1: valid expiry date		input 05/26
	result = getCardExpiryDate(&cardData);
	if (result != C_OK) {
		printf("test_getCardExpiryDate failed.\n");
		return;
	}

	printf("case1 passed.\n");

	// case2: invalid expiry date
		// case 2-a: invalid format 05-26
	result = getCardExpiryDate(&cardData);
	if (result != WRONG_EXP_DATE) {
		printf("test_getCardExpiryDate failed.\n");
		return;
	}
	// case 2-b: invalid input ab/cd
	result = getCardExpiryDate(&cardData);
	if (result != WRONG_EXP_DATE) {
		printf("test_getCardExpiryDate failed.\n");
		return;
	}
	// case 2-c: invalid month  13/26
	result = getCardExpiryDate(&cardData);
	if (result != WRONG_EXP_DATE) {
		printf("test_getCardExpiryDate failed.\n");
		return;
	}

	printf("case2 passed.\n");

	printf("=========\n");
	printf("[passed].\n");
	printf("=========\n");
}

void test_getCardPAN()
{
	printf("=====================\n");
	printf("%s", "test_getCardPAN\n");
	printf("=====================\n");

	EN_cardError_t result = C_OK;
	ST_cardData_t cardData = { 0 };

	// case1: valid card length 5112667210810487
	result = getCardPAN(&cardData);
	if (result != C_OK) {
		printf("test_getCardPAN failed.\n");
		return;
	}

	printf("case1 passed.\n");

	// case2: a- card length less than minimum 123a
	result = getCardPAN(&cardData);
	if (result != WRONG_PAN) {
		printf("test_getCardPAN failed.\n");
		return;
	}


	// case2: b- card length more than maximum 123a123a123a123a123a
	result = getCardPAN(&cardData);
	if (result != WRONG_PAN) {
		printf("test_getCardPAN failed.\n");
		return;
	}

	// case2: b- invalid format 5112?67210-10487
	result = getCardPAN(&cardData);
	if (result != WRONG_PAN) {
		printf("test_getCardPAN failed.\n");
		return;
	}


	printf("case2 passed.\n");

	printf("=========\n");
	printf("[passed].\n");
	printf("=========\n");;

	return;
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//	Terminal.h
////////////////////////////////////////////////////////////////////

void test_getTransactionDate()
{
	printf("=====================\n");
	printf("%s", "test_getTransactionDate\n");
	printf("=====================\n");

	ST_terminalData_t terminal = { 0 };
	EN_terminalError_t result = T_OK;

	// expected output: current date in the form MM/DD/YY
	result = getTransactionDate(&terminal);
	if (result != T_OK) {
		printf("test_getTransactionDate failed.\n");
		return;
	}

	printf("date: %s\n", terminal.transactionDate);

	printf("==========\n");
	printf("[passed].\n");
	printf("==========\n");

	return;
}

void test_isCardExpired()
{
	printf("=====================\n");
	printf("%s", "test_isCardExpired\n");
	printf("=====================\n");

	ST_terminalData_t terminal = { 0 };
	ST_cardData_t cardData = { 0 };
	EN_terminalError_t result = T_OK;

	getTransactionDate(&terminal);			// set the transaction date to today

	// case1: valid expiry date. expiry year > transaction year. input 05/26
	memcpy(cardData.cardExpirationData, "05/26", 5);
	result = isCardExpired(cardData, terminal);
	if (result != T_OK) {
		printf("test_isCardExpired failed.\n");
		return;
	}

	printf("case1 passed.\n");

	// case2: valid expiry date. expiry year = transaction year. input 07/22
	memcpy(cardData.cardExpirationData, "07/22", 5);
	result = isCardExpired(cardData, terminal);
	if (result != T_OK) {
		printf("test_isCardExpired failed.\n");
		return;
	}

	printf("case2 passed.\n");

	// case3: invalid expiry date. expiry year < transaction year. input 07/20
	memcpy(cardData.cardExpirationData, "07/20", 5);
	result = isCardExpired(cardData, terminal);
	if (result != EXPIRED_CARD) {
		printf("test_isCardExpired failed.\n");
		return;
	}

	printf("case3 passed.\n");

	// case4: invalid expiry date. expiry year = transaction year. input 06/22
	memcpy(cardData.cardExpirationData, "06/22", 5);
	result = isCardExpired(cardData, terminal);
	if (result != EXPIRED_CARD) {
		printf("test_isCardExpired failed.\n");
		return;
	}

	printf("case4 passed.\n");

	printf("==========\n");
	printf("[passed].\n");
	printf("==========\n");

	return;
}

void test_isValidCardPAN()
{
	printf("=====================\n");
	printf("%s", "test_isValidCardPAN\n");
	printf("=====================\n");

	EN_terminalError_t result = T_OK;
	ST_cardData_t cardData = { 0 };

	// case1: valid pan. input 79927398713
	memcpy(cardData.primaryAccountNumber, "79927398713", 12);
	result = isValidCardPAN(&cardData);
	if (result != T_OK) {
		printf("test_isValidCardPAN failed.\n");
		return;
	}

	printf("case1 passed.\n");

	// case2: invalid pan. input 79927391783
	memcpy(cardData.primaryAccountNumber, "79927391234", 12);
	result = isValidCardPAN(&cardData);
	if (result != INVALID_CARD) {
		printf("test_isValidCardPAN failed.\n");
		return;
	}

	printf("case2 passed.\n");

	printf("==========\n");
	printf("[passed].\n");
	printf("==========\n");

	return;
}

void test_getTransactionAmount()
{
	printf("=====================\n");
	printf("test_getTransactionAmount\n");
	printf("=====================\n");

	EN_terminalError_t result = T_OK;
	ST_terminalData_t terminal = { 0 };

	// case1: valid amount. 15123.1
	result = getTransactionAmount(&terminal);
	if (result != T_OK) {
		printf("test_getTransactionAmount failed.\n");
		return;
	}

	printf("case1 passed.\n");

	// case2: invalid amount. ab123cd
	result = getTransactionAmount(&terminal);
	if (result != INVALID_AMOUNT) {
		printf("test_getTransactionAmount failed.\n");
		return;
	}

	printf("case2 passed.\n");

	printf("==========\n");
	printf("[passed].\n");
	printf("==========\n");

	return;
}

void test_isBelowMaxAmount()
{
	printf("=====================\n");
	printf("test_isBelowMaxAmount\n");
	printf("=====================\n");

	EN_terminalError_t result = T_OK;
	ST_terminalData_t terminal = { 0 };

	terminal.maxTransAmount = 10000.123f;

	// case1: greater than max. 10000.500
	terminal.transAmount = 15000.f;
	result = isBelowMaxAmount(&terminal);
	if (result != EXCEED_MAX_AMOUNT) {
		printf("test_isBelowMaxAmount failed.\n");
		return;
	}

	printf("case1 passed.\n");

	// case2: less than max. 2020.8000.12f
	terminal.transAmount = 8000.12f;
	result = isBelowMaxAmount(&terminal);
	if (result != T_OK) {
		printf("test_isBelowMaxAmount failed.\n");
		return;
	}

	printf("case2 passed.\n");

	// case3: equals max.
	terminal.transAmount = 10000.123f;
	result = isBelowMaxAmount(&terminal);
	if (result != T_OK) {
		printf("test_isBelowMaxAmount failed.\n");
		return;
	}

	printf("case3 passed.\n");

	printf("==========\n");
	printf("[passed].\n");
	printf("==========\n");

	return;
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//	App.h
////////////////////////////////////////////////////////////////////

void test_approvedTransaction()
{
	// note: the test modifies the db, further runs will fail
// test data: 
/*	name:	KingMufasa OfJungle
	expiry:	05/26
	pan:	79927398713
	amount: 50000.123

	db entry: 79927398713,0000060553.011,KingMufasa OfJungle
*/
// expected output: the new user balance 10552.88

	ST_transaction_t transaction = { 0 };
	uint8_t result = 0;		// any result other than 0 is a test failure

	// set username
	strcpy(transaction.cardHolderData.cardHolderName, "KingMufasa OfJungle");

	// set expiry date
	strcpy(transaction.cardHolderData.cardExpirationData, "05/26");

	// set pan
	strcpy(transaction.cardHolderData.primaryAccountNumber, "79927398713");

	// set amount 
	transaction.terminalData.transAmount = 50000.123f;

	// process the transaction
	result = recieveTransactionData(&transaction);
	if (result != 0) {
		printf("test:approvedTransaction failed.\n");
		return;
	}

	printf("test:approvedTransaction passed.\n");

	return;
}
	 
void test_exceedAmount()
{
	// test data: 
	/*	name:	TimonTheMagnificent
		expiry:	05/26
		pan:	5112667210810487
		amount: 151000.123
	*/
	// expected output: declined transaction

	ST_transaction_t transaction = { 0 };
	EN_terminalError_t result = 0;

	// set username
	strcpy(transaction.cardHolderData.cardHolderName, "TimonTheMagnificent");

	// set expiry date
	strcpy(transaction.cardHolderData.cardExpirationData, "05/26");

	// set pan
	strcpy(transaction.cardHolderData.primaryAccountNumber, "5112667210810487");

	// set amount 
	transaction.terminalData.transAmount = 157000.123f;

	// process the transaction
	result = isBelowMaxAmount(&transaction.terminalData);
	if (result != EXCEED_MAX_AMOUNT) {
		printf("test:exceedAmount failed.\n");
		return;
	}

	printf("test:exceedAmount passed.\n");

	return;
}
	 
void test_insufficientFund()
{
	// test data: 
	/*	name:	PoorMan NoMoney
			expiry:	05/26
			pan:	5516361478373915
			amount: 75000.123
		*/
		// expected output: declined transaction

	ST_transaction_t transaction = { 0 };
	EN_transState_t  result = 0;

	// set username
	strcpy(transaction.cardHolderData.cardHolderName, "PoorMan NoMoney");

	// set expiry date
	strcpy(transaction.cardHolderData.cardExpirationData, "05/26");

	// set pan
	strcpy(transaction.cardHolderData.primaryAccountNumber, "5516361478373915");

	// set amount 
	transaction.terminalData.transAmount = 75000.123f;

	// process the transaction
	result = recieveTransactionData(&transaction);
	if (result != DECLINED_INSUFFICIENT_FUND) {
		printf("test:insufficientFund failed.\n");
		return;
	}


	printf("test:insufficientFund passed.\n");

	return;
}
	 
void test_expiredCard()
{
	// test data: 
	/*	name:	KingMufasa OfJungle
			expiry:	05/20
			pan:	79927398713
			amount: 50000.123
		*/
		// expected output: declined transaction

	ST_transaction_t transaction = { 0 };
	EN_terminalError_t  result = 0;

	// set username
	strcpy(transaction.cardHolderData.cardHolderName, "KingMufasa OfJungle");

	// set expiry date
	strcpy(transaction.cardHolderData.cardExpirationData, "05/20");

	// set pan
	strcpy(transaction.cardHolderData.primaryAccountNumber, "79927398713");

	// set amount 
	transaction.terminalData.transAmount = 75000.123f;

	// get transaction date
	result = getTransactionDate(&transaction.terminalData);
	if (result != T_OK) {
		printf("test:expiredCard failed.\n");
		return;
	}

	// check the expiry date
	result = isCardExpired(transaction.cardHolderData, transaction.terminalData);
	if (result != EXPIRED_CARD) {
		printf("test:expiredCard failed.\n");
		return;
	}

	printf("test:expiredCard passed.\n");

	return;
}
	 
void test_invalidUser()
{
	// test data: 
	/*	name:	ThiefIm Possible
			expiry:	05/26
			pan:	5162346746071729
			amount: 50000.123
		*/
		// expected output: declined transaction

	ST_transaction_t transaction = { 0 };
	EN_transState_t  result = 0;

	// set username
	strcpy(transaction.cardHolderData.cardHolderName, "ThiefIm Possible");

	// set expiry date
	strcpy(transaction.cardHolderData.cardExpirationData, "05/26");

	// set pan
	strcpy(transaction.cardHolderData.primaryAccountNumber, "5162346746071729");

	result = isValidCardPAN(&transaction.cardHolderData);
	if (result != 0) {
		printf("test:invalidUser failed.\n");
		return;
	}

	// set amount 
	transaction.terminalData.transAmount = 75000.123f;

	// process the transaction
	result = recieveTransactionData(&transaction);
	if (result != DECLINED_STOLEN_CARD) {
		printf("test:invalidUser failed.\n");
		return;
	}


	printf("test:invalidUser passed.\n");

	return;
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

#endif