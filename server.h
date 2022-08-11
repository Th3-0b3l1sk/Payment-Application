/* -------------------------------------
* Author: Mahmoud Abdullah
* EgFWD Professional Embedded Systems
* Github: https://github.com/Th3-0b3l1sk
* --------------------------------------
*/

#ifndef SERVER_HDR_GRD
#define SERVER_HDR_GRD

#include <stdint.h>
#include "terminal.h"
#include "card.h"

////////////////////////////////////////////////////////////////////
//	Type Definitions
////////////////////////////////////////////////////////////////////

typedef enum   EN_transState_t
{
    APPROVED,
    DECLINED_INSUFFICIENT_FUND,
    DECLINED_STOLEN_CARD,
    INTERNAL_SERVER_ERROR

}      EN_transState_t;

typedef enum   EN_serverError_t
{
    S_OK,
    SAVING_FAILED,
    TRANSACTION_NOT_FOUND,
    ACCOUNT_NOT_FOUND,
    LOW_BALANCE

}     EN_serverEerror_t;

typedef struct ST_accountDB_t
{
    float   balance;
    uint8_t primaryAccountNumber[20];

}       ST_accountDB_t;

typedef struct ST_transaction_t
{
    ST_cardData_t       cardHolderData;
    ST_terminalData_t   terminalData;
    EN_transState_t     transState;
    uint32_t            transactionSequenceNumber;
    
}     ST_transaction_t;

/* for use without the file base db */
typedef struct ST_DB_Accounts_t
{
    uint8_t index;
    ST_accountDB_t account[255];

}     ST_DB_Accounts_t;

typedef struct ST_DB_Transactions_t
{
    uint8_t index;
    ST_transaction_t transaction[255];

} ST_DB_Transactions_t;

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//	Function Declarations
////////////////////////////////////////////////////////////////////

EN_transState_t   recieveTransactionData(ST_transaction_t* transData);
EN_serverEerror_t isValidAccount(ST_cardData_t* cardData);
EN_serverEerror_t isAmountAvailable(ST_terminalData_t* termData);
EN_serverEerror_t saveTransaction(ST_transaction_t* transData);
EN_serverEerror_t getTransaction(uint32_t transactionSequenceNumber, ST_transaction_t* transData);

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

#endif
