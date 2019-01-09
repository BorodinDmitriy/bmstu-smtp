#ifndef STATES_H
#define STATES_H

//======================//
//       CONTEXT        //
//======================//

#define NULL_POINTER - 100

#define SMTP_ERROR -1

#define PREPARE_SOCKET_CONNECTION 0
#define CONNECT 1
#define RECEIVE_SMTP_GREETING 2
#define SEND_EHLO 3
#define RECEIVE_EHLO_RESPONSE 4
#define SEND_MAIL_FROM 5
#define RECEIVE_MAIL_FROM_RESPONSE 6
#define SEND_RCPT_TO 7
#define RECEIVE_RCPT_TO_RESPONSE 8
#define SEND_DATA 9
#define RECEIVE_DATA_RESPONSE 10
#define SEND_LETTER 11
#define RECEIVE_LETTER_RESPONSE 12
#define SEND_QUIT 13
#define RECEIVE_QUIT_RESPONSE 14
#define DISPOSING_SOCKET 15

#define SEND_RSET 20
#define RECEIVE_RSET_RESPONSE 21


#endif // STATES_H