/* -------------------------------------
* Author: Mahmoud Abdullah
* EgFWD Professional Embedded Systems
* Github: https://github.com/Th3-0b3l1sk
* --------------------------------------
*/

#define _CRT_SECURE_NO_WARNINGS

#include "terminal.h"
#include <stdio.h>       // printf, scanf_s
#include <string.h>
#include <stdlib.h>      // atoi
#include <time.h>

#ifdef _EDEBUG
#define D_LOG(x)   printf(x);
#else
#define D_LOG(x)    ;
#endif // DEBUG

extern float g_maxTransaction;          // from app.c
extern void clearStdin(char* buffer);   // from card.c


EN_terminalError_t getTransactionDate(ST_terminalData_t* termData)
{
    if (termData == NULL)
        return WRONG_DATE;

    time_t tmpTime = time(NULL);
    register struct tm* currentDate = localtime(&tmpTime);

    // store the time in format DD/MM/YYYY
    sprintf(termData->transactionDate, "%02d/%02d/%4d", 
             currentDate->tm_mday,                                        
             currentDate->tm_mon + 1,
             currentDate->tm_year + 1900);

    return T_OK;
}

EN_terminalError_t isCardExpired(ST_cardData_t cardData, ST_terminalData_t termData)
{
    // comparing years DD/MM/YYYY MM/YY
    const uint8_t SLASH1 = 2, SLASH2 = 5;

    uint8_t* ptermYear       = &termData.transactionDate[SLASH2 + 1];
    uint8_t* pcardExpiryYear = &cardData.cardExpirationData[SLASH1 + 1];

    uint16_t termYear       = atoi(ptermYear);
    uint16_t cardExpiryYear = atoi(pcardExpiryYear) + 2000;

    // the card expiry year is higher than the transaction year
    if (cardExpiryYear > termYear)
        return T_OK;
    else if (cardExpiryYear < termYear)
        return EXPIRED_CARD;
    
    // case if the card year == trans year then it's necessary to copmare months
    uint8_t* ptermMonth       = &termData.transactionDate[SLASH1 + 1];
    uint8_t* pcardExpiryMonth = cardData.cardExpirationData;
    
    termData.transactionDate[SLASH2]     = '\0';
    cardData.cardExpirationData[SLASH1]  = '\0';

    uint8_t termMonth       = atoi(ptermMonth);
    uint8_t cardExpiryMonth = atoi(pcardExpiryMonth);

    if ((cardExpiryYear == termYear) && (cardExpiryMonth < termMonth)) 
        return EXPIRED_CARD;
    

    return T_OK;
}

EN_terminalError_t isValidCardPAN(ST_cardData_t* cardData)
{
    register uint8_t* pan = cardData->primaryAccountNumber;

    // 1- drop the last digit(validation digit)
    uint8_t panLen = strlen(pan) - 2;                    // the index of the last digit before the v digit
    uint8_t vDigit = (uint8_t)pan[panLen + 1] - 0x30;    // last digit

    // 2- calculate the algorithm
    uint16_t sum = 0;
    uint8_t digit = 0;
    for (uint8_t i = 0; i <= panLen; i++) {
        if ((i % 2) == 0) {
            digit = 2 * ((uint8_t)pan[panLen - i] - 0x30);
            
            sum += digit % 10;
            sum += digit / 10;
        }
        else
            sum += (uint8_t)pan[panLen - i] - 0x30;
    }

    // 3- verigy the digit
    uint8_t c_vDigit = 10 - (sum % 10);

    return c_vDigit == vDigit ? T_OK : INVALID_CARD;
    
}

EN_terminalError_t getTransactionAmount(ST_terminalData_t* termData)
{
    if(termData == NULL)
        return INVALID_AMOUNT;
    
    printf("Please enter the amount: ");
    float amount = 0.f;
    if (scanf("%f", &amount) == NULL) {
        D_LOG("Failed to get the transaction amount.\n");
        return INVALID_AMOUNT;
    }

    if(amount <= 0.0f)
        return INVALID_AMOUNT;

    termData->transAmount = amount;

    return T_OK;
}

EN_terminalError_t isBelowMaxAmount(ST_terminalData_t* termData)
{
    if(termData == NULL)
        return EXCEED_MAX_AMOUNT;

    return termData->transAmount > termData->maxTransAmount? EXCEED_MAX_AMOUNT : T_OK;
}

EN_terminalError_t setMaxAmount(ST_terminalData_t* termData)
{
    termData->maxTransAmount = g_maxTransaction;
    return T_OK;
}
