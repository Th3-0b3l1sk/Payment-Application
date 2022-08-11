/* -------------------------------------
* Author: Mahmoud Abdullah
* EgFWD Professional Embedded Systems
* Github: https://github.com/Th3-0b3l1sk
* --------------------------------------
*/

#define _CRT_SECURE_NO_WARNINGS

#include "server.h"
#include <stdio.h>  // fopen
#include <time.h>   // time api
#include <string.h>
#include <ctype.h>
#include <stdlib.h>


extern uint16_t g_lastSequence;
extern float    g_maxTransaction;

extern void initSettings();

////////////////////////////////////////////////////////////////////
//	Static global variables
////////////////////////////////////////////////////////////////////

static char* strings[] = { "approved",                              // APPROVED
                           "declined:insufficient fund",            // DECLINED_INSUFFICIENT_FUND
                           "declined:stolen card",                  // DECLINED_STOLEN_CARD
                           "declined:internal server error" };      // INTERNAL_SERVER_ERROR


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//	Static helper functions
////////////////////////////////////////////////////////////////////

static EN_serverEerror_t getState(const char* stateStr)
{

    uint8_t match = 0;
    for (; match < sizeof(strings) / sizeof(strings[0]); match++)
        if (strcmp(strings[match], stateStr) == 0)
            break;

    return match;   // converts nicely to EN_serverEerror_t
}

static char*             getStrResource(EN_transState_t transactionState)
{
    return strings[transactionState];
}

static uint8_t           findPAN(const uint8_t* pan)
{ 
    uint8_t success = 0;
    uint8_t buffer[255] = { 0 };    // it's safe to assume 255 is long enough
                                    // since the DB is modified only by the adminstrator
    // loop over, read lines, get pan, not found return
    FILE* db = fopen("Accounts DB.txt", "r");
    while (fgets(buffer, 255, db)) {
        // skip comments and empty lines
        if (buffer[0] == '#' || isspace(buffer[0]))
            continue;

        uint8_t* sep = strchr(buffer, ',');
        if (sep == NULL)    // error in the db
            continue;

        *sep = '\0';
        if (strcmp(buffer, pan) == 0) {
            success = 1;
            break;
        }
            
    }

    fclose(db);
    return success;   // -1 in case of no matching PAN
}

static EN_transState_t   queryBalance(float* balance, const uint8_t* pan, uint8_t which)
{
    uint8_t buffer[255] = { 0 };    
    EN_serverEerror_t result = S_OK;
    FILE* db = fopen("Accounts DB.txt", "r+");

    while (fgets(buffer, 255, db)){
        if (buffer[0] == '#' || isspace(buffer[0]))
            continue;

        uint8_t* sep = strchr(buffer, ',');
        if (sep == NULL)    // error in the db
            continue;

        *sep = '\0';
        if (strcmp(buffer, pan) == 0) {
            *sep = ',';
            break;
        }
    }

    uint8_t lineLen = strlen(buffer);

    switch (which)
    {
    case 0: // balance enquiry
    {
        uint8_t* sep = strchr(buffer, ',');
        if (sep == NULL) {
            result = INTERNAL_SERVER_ERROR;
            break;
        }

        uint8_t* ptr = ++sep;
        sep = strchr(sep, ',');
        if (sep == NULL) {
            result = INTERNAL_SERVER_ERROR;
            break;
        }
        *sep = '\0';

        *balance = strtof(ptr, NULL);
        
        break;
    }

    case 1: // balance update
    {
        if (balance == NULL) {
            result = INTERNAL_SERVER_ERROR;
            break;
        }

        // rewind the file pointer to the beginning of the file
        fseek(db, -lineLen -1, SEEK_CUR);
        uint8_t* sep = strchr(buffer, ',');
        if (sep == NULL) {
            result = INTERNAL_SERVER_ERROR;
            break;
        }
        sep++;
       
        sprintf(sep, "%014.3f", *balance);
        // overwrite the null placed by sprintf
        sep += 14;  // 13 for the field width + 1 for the floating point 
        *sep = ',';

        fwrite(buffer, 1, lineLen, db);

        break;
    }

    default:
        fclose(db);
        result = INTERNAL_SERVER_ERROR;
    }

    fclose(db);

    return result;
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

EN_transState_t   recieveTransactionData(ST_transaction_t* transData)
{
    EN_transState_t result = APPROVED;

    if (transData == NULL) 
        return INTERNAL_SERVER_ERROR;


    float balance = 0.f;
    const uint8_t* pan = transData->cardHolderData.primaryAccountNumber;

    if (g_maxTransaction == 0)
        initSettings();

    EN_serverEerror_t error = S_OK;

    // validate PAN
    error = isValidAccount(&transData->cardHolderData);
    if (error != S_OK) {
        result = DECLINED_STOLEN_CARD;
        goto checkout;
    }

    error = queryBalance(&balance, pan, 0); // enquire balance
    if (error != S_OK) {
        result = error;
        goto checkout;
    }

    float tmp = transData->terminalData.maxTransAmount;
    transData->terminalData.maxTransAmount = balance;

    // validate credit
    error = isAmountAvailable(&transData->terminalData);
    if (error != S_OK) {
        result = DECLINED_INSUFFICIENT_FUND;
        goto checkout;
    }

    transData->terminalData.maxTransAmount = tmp;

checkout:
    // save the transaction
    transData->transState = result;
    error = saveTransaction(transData);
    if (error != S_OK)
        return INTERNAL_SERVER_ERROR;

    if (result == APPROVED) {
        // the transaction was a success. update the new balance
        float newBalance = balance - transData->terminalData.transAmount;
        error = queryBalance(&newBalance, pan, 1); // update balance
        if (error != S_OK)
            return error;
    }
    return result;
}

EN_serverEerror_t isValidAccount(ST_cardData_t* cardData)
{
    const uint8_t* pan = cardData->primaryAccountNumber;
    
    if(findPAN(pan) == 0)
        return DECLINED_STOLEN_CARD;

    return S_OK;
}

EN_serverEerror_t isAmountAvailable(ST_terminalData_t* termData)
{
    if (termData->transAmount > termData->maxTransAmount)
        return DECLINED_INSUFFICIENT_FUND;
   
    return S_OK;
}

EN_serverEerror_t saveTransaction(ST_transaction_t* transData)
{
    FILE* transactionLog = fopen("Transactions DB.txt", "a");
    if (transactionLog == NULL) 
        return INTERNAL_SERVER_ERROR;
    
    time_t currentTime = time(NULL);
    uint8_t logMessage[255] = "";
    transData->transactionSequenceNumber = ++g_lastSequence;

    // format: number,name,pan,expiry,amount,date,state
    sprintf(logMessage, "%d,%s,%s,%s,%.3f,%s,%s\n",
                        transData->transactionSequenceNumber,
                        transData->cardHolderData.cardHolderName,
                        transData->cardHolderData.primaryAccountNumber,
                        transData->cardHolderData.cardExpirationData,
                        transData->terminalData.transAmount,
                        transData->terminalData.transactionDate,
                        getStrResource(transData->transState) );   

    if (fwrite(logMessage, 1, strlen(logMessage), transactionLog) == NULL) {
        // failed to write
        fclose(transactionLog);
                
        return SAVING_FAILED;
    }

    fclose(transactionLog);

    ST_transaction_t tra;
    
    getTransaction(transData->transactionSequenceNumber, &tra);

    return S_OK;
}

EN_serverEerror_t getTransaction(uint32_t transactionSequenceNumber, ST_transaction_t* transData)
{
    // sscanf might be a more reasonable option :)

    if (transData == NULL)
        return INTERNAL_SERVER_ERROR;

    uint8_t buffer[255] = { 0 };    // it's safe to assume 255 is long enough
                                    // since the DB is modified only by the adminstrator
    uint8_t* ptr = NULL;
    EN_serverEerror_t result = TRANSACTION_NOT_FOUND;
    FILE* db = fopen("Transactions DB.txt", "r");

    while (fgets(buffer, 255, db)) {
        // skip comments and empty lines
        if (buffer[0] == '#' || isspace(buffer[0]))
            continue;

        uint8_t* sep = strchr(buffer, ',');
        if (sep == NULL)
            continue;   // db error

        // read the number
        ptr = buffer;
        *sep = '\0';
        if (transactionSequenceNumber != strtol(ptr, NULL, 10))
            continue;

        transData->transactionSequenceNumber = transactionSequenceNumber;

        // read the name
        ptr = ++sep;
        sep = strchr(sep, ',');
        if (sep == NULL) {
            transData = NULL;
            result = ACCOUNT_NOT_FOUND; // db error
            break;
        }
        *sep = '\0';
        uint8_t temp = strlen(ptr);
        strcpy_s(transData->cardHolderData.cardHolderName, strlen(ptr) + 1, ptr);
        
        
        // read the pan
        ptr = ++sep;
        sep = strchr(sep, ',');
        if (sep == NULL) {
            transData = NULL;
            result = ACCOUNT_NOT_FOUND; // db error
            break;
        }
        *sep = '\0';

        strcpy_s(transData->cardHolderData.primaryAccountNumber, strlen(ptr) + 1, ptr);

        // read expiry
        ptr = ++sep;
        sep = strchr(sep, ',');
        if (sep == NULL) {
            transData = NULL;
            result = ACCOUNT_NOT_FOUND; // db error
            break;
        }
        *sep = '\0';

        strcpy_s(transData->cardHolderData.cardExpirationData, strlen(ptr) + 1, ptr);

        // read amount
        ptr = ++sep;
        sep = strchr(sep, ',');
        if (sep == NULL) {
            transData = NULL;
            result = ACCOUNT_NOT_FOUND; // db error
            break;
        }
        *sep = '\0';

        transData->terminalData.transAmount = strtof(ptr, NULL);

        transData->terminalData.maxTransAmount = g_maxTransaction;

        // read date
        ptr = ++sep;
        sep = strchr(sep, ',');
        if (sep == NULL) {
            transData = NULL;
            result = ACCOUNT_NOT_FOUND; // db error
            break;
        }
        *sep = '\0';

        strcpy_s(transData->terminalData.transactionDate, strlen(ptr) + 1, ptr);

        // read state
        ptr = ++sep;
        sep = strchr(sep, '\n');
        if (sep == NULL) {
            transData = NULL;
            result = ACCOUNT_NOT_FOUND; // db error
            break;
        }
        *sep = '\0';

        transData->transState = getState(ptr);
        result = S_OK;

        break;
    }

    fclose(db);
    
    return result;
}

