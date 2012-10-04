#include "contiki-net.h"

#include "stomp-network.h"

#ifndef STOMPC_H
#define	STOMPC_H

#ifdef	__cplusplus
extern "C" {
#endif

    struct stomp_state {
        struct pt pthread;
        struct psock socket;
        
        struct stomp_network_state network_state;
        
        unsigned char command;
        unsigned char id;
        char *destination;
        char *message;
        char *tx;
    };

    void stomp_init(void);
    
    void stomp_appcall(void *s);
    
    struct stomp_state* stomp_connect(struct stomp_state *s, char *hostname, uip_ipaddr_t *ipaddr, int port);
    
    void stomp_connected(struct stomp_state *s);
    
    void stomp_subscribe(struct stomp_state *s, unsigned char id, char *dest, unsigned char ack);
    
    void stomp_unsubscribe(struct stomp_state *s, unsigned char id);
    
    void stomp_send(struct stomp_state *s, char* dest, char* msg, char *tx);
    
    void stomp_begin(struct stomp_state *s, char *tx);
    
    void stomp_commit(struct stomp_state *s, char *tx);
    
    void stomp_abort(struct stomp_state *s, char *tx);
    
    void stomp_disconnect(struct stomp_state *s);
    
    void stomp_disconnected(struct stomp_state *s);
    
#ifdef	__cplusplus
}
#endif

#endif	/* STOMPC_H */

