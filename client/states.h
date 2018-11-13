#ifndef STATES_H
#define STATES_H

//======================//
//       CONTEXT        //
//======================//

#define START_WORK 0
//  INIT
#define INIT_SOCKET 000
//  HELO
#define HELO 001
#define RECEIVE_HELO_MESSAGE 011
#define SEND_HELO_MESSAGE 012
#define RECEIVE_HELO_CONNECT 013


#endif // STATES_H