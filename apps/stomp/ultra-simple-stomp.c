#include "ultra-simple-stomp.h"

#include "stomp-tools.h"
#include "stomp-strings.h"

#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"

#include "uip-debug.h"

#include <string.h>

const char stomp_version_default[4] = {0x31, 0x2e, 0x31,};

const char stomp_content_type_default[11] = {0x74, 0x65, 0x78, 0x74, 0x2f, 0x70, 0x6c, 0x61, 0x69, 0x6e,};

struct ultra_simple_stomp state;

static void
__sent() {
    free(state.buf);
    state.buf = NULL;
    stomp_net_sent(state);
}

static void
__send() {
    if (state.buf == NULL) {
        return;
    }
#ifndef WITH_UDP
    if (state.len > uip_mss()) {
        state.sentlen = uip_mss();
    } else {
        state.sentlen = state.len;
    }
#endif
#ifdef WITH_UDP
    uip_udp_packet_sendto(state.conn, state.buf + off, state.len, state.addr, UIP_HTONS(state.port));
    __sent();
#else
    uip_send(state.buf + state.off, state.sentlen);
#endif
}

#ifndef WITH_UDP

static void
__acked() {
    state.len -= state.sentlen;
    if (state.len == 0) {
        __sent();
    } else {
        state.off += state.sentlen;
    }
    state.sentlen = 0;
}
#endif

void
stomp_app() {
#ifndef WITH_UDP
    if (uip_connected()) {
        state.flags = 0;
        stomp_net_connected();
        __send();
        return;
    }
    if (uip_closed()) {
        stomp_net_closed();
    }
    if (uip_aborted()) {
        stomp_net_aborted();
    }
    if (uip_timedout()) {
        stomp_net_timedout();
    }
    if (state.flags & ULTRA_SIMPLE_STOMP_FLAG_DISCONNECT) {
        uip_close();
        return;
    }
    if (state.flags & ULTRA_SIMPLE_STOMP_FLAG_ABORT) {
        uip_abort();
        return;
    }
    if (uip_acked()) {
        __acked();
    }
#endif
    if (uip_newdata()) {
        stomp_net_received((char*) uip_appdata, uip_datalen());
    }
#ifndef WITH_UDP
    if (uip_rexmit() || uip_newdata() || uip_acked()) {
        __send();

    } else if (uip_poll()) {
        __send();
    }
#endif
    if (state.buf != NULL) {
        __send();
    }
}

void
stomp_connect(char *host, char* login, char* pass) {
    int off = 0, total_len = 0, host_len = 0, login_len = 0, pass_len = 0;
    char *buf = NULL;

    total_len = STOMP_COMMAND_CONNECT_LEN + 1;
    if (host == NULL) {
        PRINTA("No host. Abort CONNECT.\n");
        return;
    } else {
        host_len = strlen(host);
        total_len += STOMP_HEADER_HOST_LEN + 1 + host_len + 1;
    }
    if (login == NULL) {
        PRINTA("No login.\n");
    } else {
        login_len = strlen(login);
        total_len += STOMP_HEADER_LOGIN_LEN + 1 + login_len + 1;
    }
    if (pass == NULL) {
        PRINTA("No pass.\n");
    } else {
        pass_len = strlen(pass);
        total_len += STOMP_HEADER_PASSCODE_LEN + 1 + pass_len + 1;
    }
    total_len += STOMP_HEADER_ACCEPT_VERSION_LEN + 1 + 3 + 1 + 1;

    buf = NEW_ARRAY(char, total_len + 1);

    memcpy(buf + off, stomp_command_connect, STOMP_COMMAND_CONNECT_LEN);
    off += STOMP_COMMAND_CONNECT_LEN;
    *(buf + off) = STOMP_NEW_LINE;
    off += 1;

    memcpy(buf + off, stomp_header_host, STOMP_HEADER_HOST_LEN);
    off += STOMP_HEADER_HOST_LEN;
    *(buf + off) = STOMP_COLON;
    off += 1;
    memcpy(buf + off, host, host_len);
    off += host_len;
    *(buf + off) = STOMP_NEW_LINE;
    off += 1;

    if (login != NULL) {
        memcpy(buf + off, stomp_header_login, STOMP_HEADER_LOGIN_LEN);
        off += STOMP_HEADER_LOGIN_LEN;
        *(buf + off) = STOMP_COLON;
        off += 1;
        memcpy(buf + off, login, login_len);
        off += login_len;
        *(buf + off) = STOMP_NEW_LINE;
        off += 1;
    }

    if (pass != NULL) {
        memcpy(buf + off, stomp_header_passcode, STOMP_HEADER_PASSCODE_LEN);
        off += STOMP_HEADER_PASSCODE_LEN;
        *(buf + off) = STOMP_COLON;
        off += 1;
        memcpy(buf + off, pass, pass_len);
        off += pass_len;
        *(buf + off) = STOMP_NEW_LINE;
        off += 1;
    }

    memcpy(buf + off, stomp_header_accept_version, STOMP_HEADER_ACCEPT_VERSION_LEN);
    off += STOMP_HEADER_ACCEPT_VERSION_LEN;
    *(buf + off) = STOMP_COLON;
    off += 1;
    memcpy(buf + off, stomp_version_default, 3);
    off += 3;
    *(buf + off) = STOMP_NEW_LINE;
    off += 1;

    *(buf + off) = STOMP_NEW_LINE;
    off += 1;

    if (off != total_len) {
        PRINTA("CONNECT: off(%d) != total_len(%d).\n", off, total_len);
    }
    PRINTA("\n^%s@\n", buf);
    
    DELETE(buf);
}

void
stomp_subscribe(char *id, char *destination, char *ack) {
    int off = 0, total_len = 0, id_len = 0, destination_len = 0, ack_len = 0;
    char *buf = NULL;

    total_len = STOMP_COMMAND_SUBSCRIBE_LEN + 1;
    if (id == NULL) {
        PRINTA("No id. Abort SUBSCRIBE.\n");
        return;
    } else {
        id_len = strlen(id);
        total_len += STOMP_HEADER_ID_LEN + 1 + id_len + 1;
    }
    if (destination == NULL) {
        PRINTA("No destination. Abort SUBSCRIBE.\n");
        return;
    } else {
        destination_len = strlen(destination);
        total_len += STOMP_HEADER_DESTINATION_LEN + 1 + destination_len + 1;
    }
    if (ack == NULL) {
        PRINTA("No ack. Set to 'auto'.\n");
        ack = (char*)stomp_header_auto;
        ack_len = 4;
        total_len += STOMP_HEADER_ACK_LEN + 1 + ack_len + 1;
    } else {
        ack_len = strlen(ack);
        total_len += STOMP_HEADER_ACK_LEN + 1 + ack_len + 1;
    }
    total_len += 1;

    buf = NEW_ARRAY(char, total_len + 1);

    memcpy(buf + off, stomp_command_subscribe, STOMP_COMMAND_SUBSCRIBE_LEN);
    off += STOMP_COMMAND_SUBSCRIBE_LEN;
    *(buf + off) = STOMP_NEW_LINE;
    off += 1;

    if (id != NULL) {
        memcpy(buf + off, stomp_header_id, STOMP_HEADER_ID_LEN);
        off += STOMP_HEADER_ID_LEN;
        *(buf + off) = STOMP_COLON;
        off += 1;
        memcpy(buf + off, id, id_len);
        off += id_len;
        *(buf + off) = STOMP_NEW_LINE;
        off += 1;
    }

    if (destination != NULL) {
        memcpy(buf + off, stomp_header_destination, STOMP_HEADER_DESTINATION_LEN);
        off += STOMP_HEADER_DESTINATION_LEN;
        *(buf + off) = STOMP_COLON;
        off += 1;
        memcpy(buf + off, destination, destination_len);
        off += destination_len;
        *(buf + off) = STOMP_NEW_LINE;
        off += 1;
    }

    if (ack != NULL) {
        memcpy(buf + off, stomp_header_ack, STOMP_HEADER_ACK_LEN);
        off += STOMP_HEADER_ACK_LEN;
        *(buf + off) = STOMP_COLON;
        off += 1;
        memcpy(buf + off, ack, ack_len);
        off += ack_len;
        *(buf + off) = STOMP_NEW_LINE;
        off += 1;
    }

    *(buf + off) = STOMP_NEW_LINE;
    off += 1;

    if (off != total_len) {
        PRINTA("SUBSCRIBE: off(%d) != total_len(%d).\n", off, total_len);
    }
    PRINTA("\n^%s@\n", buf);
    
    DELETE(buf);
}

void
stomp_unsubscribe(char *id) {
    int off = 0, total_len = 0, id_len = 0;
    char *buf = NULL;

    total_len = STOMP_COMMAND_UNSUBSCRIBE_LEN + 1;
    if (id == NULL) {
        PRINTA("No id. Abort UNSUBSCRIBE.\n");
        return;
    } else {
        id_len = strlen(id);
        total_len += STOMP_HEADER_ID_LEN + 1 + id_len + 1;
    }
    total_len += 1;

    buf = NEW_ARRAY(char, total_len + 1);

    memcpy(buf + off, stomp_command_unsubscribe, STOMP_COMMAND_UNSUBSCRIBE_LEN);
    off += STOMP_COMMAND_UNSUBSCRIBE_LEN;
    *(buf + off) = STOMP_NEW_LINE;
    off += 1;

    if (id != NULL) {
        memcpy(buf + off, stomp_header_id, STOMP_HEADER_ID_LEN);
        off += STOMP_HEADER_ID_LEN;
        *(buf + off) = STOMP_COLON;
        off += 1;
        memcpy(buf + off, id, id_len);
        off += id_len;
        *(buf + off) = STOMP_NEW_LINE;
        off += 1;
    }

    *(buf + off) = STOMP_NEW_LINE;
    off += 1;

    if (off != total_len) {
        PRINTA("UNSUBSCRIBE: off(%d) != total_len(%d).\n", off, total_len);
    }
    PRINTA("\n^%s@\n", buf);
    
    DELETE(buf);
}

void
stomp_send(char *destination, char *type, char *length, char *receipt, char *tx, char *message) {
    int off = 0, total_len = 0, destination_len = 0, type_len = 0, length_len = 0, receipt_len = 0, tx_len = 0, message_len = 0;
    char *buf = NULL;

    total_len = STOMP_COMMAND_SEND_LEN + 1;
    if (destination == NULL) {
        PRINTA("No destination. Abort SEND.\n");
        return;
    } else {
        destination_len = strlen(destination);
        total_len += STOMP_HEADER_DESTINATION_LEN + 1 + destination_len + 1;
    }
    if (type == NULL) {
        PRINTA("No content type. Set to default 'plain/text'.\n");
        type = (char*)stomp_content_type_default;
        type_len = 10;
        total_len += STOMP_HEADER_CONTENT_TYPE_LEN + 1 + type_len + 1;
    } else {
        type_len = strlen(type);
        total_len += STOMP_HEADER_CONTENT_TYPE_LEN + 1 + type_len + 1;
    }

    if (length == NULL) {
        PRINTA("No length. Abort SEND.\n");
        return;
    } else {
        length_len = strlen(length);
        total_len += STOMP_HEADER_CONTENT_LENGTH_LEN + 1 + length_len + 1;
    }
    if (receipt == NULL) {
        PRINTA("No receipt.\n");
    } else {
        receipt_len = strlen(receipt);
        total_len += STOMP_HEADER_RECEIPT_LEN + 1 + receipt_len + 1;
    }
    if (tx == NULL) {
        PRINTA("No tx.\n");
    } else {
        tx_len = strlen(tx);
        total_len += STOMP_HEADER_TRANSACTION_LEN + 1 + tx_len + 1;
    }
    if (message == NULL) {
        PRINTA("No message. Abort SEND.\n");
        return;
    } else {
        message_len = strlen(length);
        total_len += message_len + 1;
    }
    total_len += 1;

    buf = NEW_ARRAY(char, total_len + 1);

    memcpy(buf + off, stomp_command_send, STOMP_COMMAND_SEND_LEN);
    off += STOMP_COMMAND_SEND_LEN;
    *(buf + off) = STOMP_NEW_LINE;
    off += 1;

    if (destination != NULL) {
        memcpy(buf + off, stomp_header_destination, STOMP_HEADER_DESTINATION_LEN);
        off += STOMP_HEADER_DESTINATION_LEN;
        *(buf + off) = STOMP_COLON;
        off += 1;
        memcpy(buf + off, destination, destination_len);
        off += destination_len;
        *(buf + off) = STOMP_NEW_LINE;
        off += 1;
    }

    if (type != NULL) {
        memcpy(buf + off, stomp_header_content_type, STOMP_HEADER_CONTENT_TYPE_LEN);
        off += STOMP_HEADER_CONTENT_TYPE_LEN;
        *(buf + off) = STOMP_COLON;
        off += 1;
        memcpy(buf + off, type, type_len);
        off += type_len;
        *(buf + off) = STOMP_NEW_LINE;
        off += 1;
    }

    if (length != NULL) {
        memcpy(buf + off, stomp_header_content_length, STOMP_HEADER_CONTENT_LENGTH_LEN);
        off += STOMP_HEADER_CONTENT_LENGTH_LEN;
        *(buf + off) = STOMP_COLON;
        off += 1;
        memcpy(buf + off, length, length_len);
        off += length_len;
        *(buf + off) = STOMP_NEW_LINE;
        off += 1;
    }

    if (receipt != NULL) {
        memcpy(buf + off, stomp_header_receipt, STOMP_HEADER_RECEIPT_LEN);
        off += STOMP_HEADER_RECEIPT_LEN;
        *(buf + off) = STOMP_COLON;
        off += 1;
        memcpy(buf + off, receipt, receipt_len);
        off += receipt_len;
        *(buf + off) = STOMP_NEW_LINE;
        off += 1;
    }

    if (tx != NULL) {
        memcpy(buf + off, stomp_header_transaction, STOMP_HEADER_TRANSACTION_LEN);
        off += STOMP_HEADER_TRANSACTION_LEN;
        *(buf + off) = STOMP_COLON;
        off += 1;
        memcpy(buf + off, tx, tx_len);
        off += tx_len;
        *(buf + off) = STOMP_NEW_LINE;
        off += 1;
    }

    *(buf + off) = STOMP_NEW_LINE;
    off += 1;

    if (message != NULL) {
        memcpy(buf + off, message, message_len);
        off += message_len + 1;
    }

    if (off != total_len) {
        PRINTA("SEND: off(%d) != total_len(%d).\n", off, total_len);
    }
    PRINTA("\n^%s@\n", buf);
    
    DELETE(buf);
}

void
stomp_begin(char *tx) {
    int off = 0, total_len = 0, tx_len = 0;
    char *buf = NULL;

    total_len = STOMP_COMMAND_BEGIN_LEN + 1;
    if (tx == NULL) {
        PRINTA("No tx. Abort BEGIN.\n");
        return;
    } else {
        tx_len = strlen(tx);
        total_len += STOMP_HEADER_TRANSACTION_LEN + 1 + tx_len + 1;
    }
    total_len += 1;

    buf = NEW_ARRAY(char, total_len + 1);

    memcpy(buf, stomp_command_begin, STOMP_COMMAND_BEGIN_LEN);
    off += STOMP_COMMAND_BEGIN_LEN;
    *(buf + off) = STOMP_NEW_LINE;
    off += 1;

    if (tx != NULL) {
        memcpy(buf + off, stomp_header_transaction, STOMP_HEADER_TRANSACTION_LEN);
        off += STOMP_HEADER_TRANSACTION_LEN;
        *(buf + off) = STOMP_COLON;
        off += 1;
        memcpy(buf + off, tx, tx_len);
        off += tx_len;
        *(buf + off) = STOMP_NEW_LINE;
        off += 1;
    }

    *(buf + off) = STOMP_NEW_LINE;
    off += 1;

    if (off != total_len) {
        PRINTA("BEGIN: off(%d) != total_len(%d).\n", off, total_len);
    }
    PRINTA("\n^%s@\n", buf);
    
    DELETE(buf);
}

void
stomp_commit(char *tx) {
    int off = 0, total_len = 0, tx_len = 0;
    char *buf = NULL;

    total_len = STOMP_COMMAND_COMMIT_LEN + 1;
    if (tx == NULL) {
        PRINTA("No tx. Abort ABORT.\n");
        return;
    } else {
        tx_len = strlen(tx);
        total_len += STOMP_HEADER_TRANSACTION_LEN + 1 + tx_len + 1;
    }
    total_len += 1;

    buf = NEW_ARRAY(char, total_len + 1);

    memcpy(buf, stomp_command_commit, STOMP_COMMAND_COMMIT_LEN);
    off += STOMP_COMMAND_COMMIT_LEN;
    *(buf + off) = STOMP_NEW_LINE;
    off += 1;

    if (tx != NULL) {
        memcpy(buf + off, stomp_header_transaction, STOMP_HEADER_TRANSACTION_LEN);
        off += STOMP_HEADER_TRANSACTION_LEN;
        *(buf + off) = STOMP_COLON;
        off += 1;
        memcpy(buf + off, tx, tx_len);
        off += tx_len;
        *(buf + off) = STOMP_NEW_LINE;
        off += 1;
    }

    *(buf + off) = STOMP_NEW_LINE;
    off += 1;

    if (off != total_len) {
        PRINTA("BEGIN: off(%d) != total_len(%d).\n", off, total_len);
    }
    PRINTA("\n^%s@\n", buf);
    
    DELETE(buf);
}

void
stomp_abort(char *tx) {
    int off = 0, total_len = 0, tx_len = 0;
    char *buf = NULL;

    total_len = STOMP_COMMAND_ABORT_LEN + 1;
    if (tx == NULL) {
        PRINTA("No tx. Abort ABORT.\n");
        return;
    } else {
        tx_len = strlen(tx);
        total_len += STOMP_HEADER_TRANSACTION_LEN + 1 + tx_len + 1;
    }
    total_len += 1;

    buf = NEW_ARRAY(char, total_len + 1);

    memcpy(buf, stomp_command_abort, STOMP_COMMAND_ABORT_LEN);
    off += STOMP_COMMAND_ABORT_LEN;
    *(buf + off) = STOMP_NEW_LINE;
    off += 1;

    if (tx != NULL) {
        memcpy(buf + off, stomp_header_transaction, STOMP_HEADER_TRANSACTION_LEN);
        off += STOMP_HEADER_TRANSACTION_LEN;
        *(buf + off) = STOMP_COLON;
        off += 1;
        memcpy(buf + off, tx, tx_len);
        off += tx_len;
        *(buf + off) = STOMP_NEW_LINE;
        off += 1;
    }

    *(buf + off) = STOMP_NEW_LINE;
    off += 1;

    if (off != total_len) {
        PRINTA("ABORT: off(%d) != total_len(%d).\n", off, total_len);
    }
    PRINTA("\n^%s@\n", buf);
    
    DELETE(buf);
}

void
stomp_disconnect(char *receipt) {
    int off = 0, total_len = 0, receipt_len = 0;
    char *buf = NULL;

    total_len = STOMP_COMMAND_DISCONNECT_LEN + 1;
    if (receipt == NULL) {
        PRINTA("No receipt. Abort DISCONNECT.\n");
        return;
    } else {
        receipt_len = strlen(receipt);
        total_len += STOMP_HEADER_RECEIPT_LEN + 1 + receipt_len + 1;
    }
    total_len += 1;

    buf = NEW_ARRAY(char, total_len + 1);

    memcpy(buf, stomp_command_disconnect, STOMP_COMMAND_DISCONNECT_LEN);
    off += STOMP_COMMAND_DISCONNECT_LEN;
    *(buf + off) = STOMP_NEW_LINE;
    off += 1;

    if (receipt != NULL) {
        memcpy(buf + off, stomp_header_receipt, STOMP_HEADER_RECEIPT_LEN);
        off += STOMP_HEADER_RECEIPT_LEN;
        *(buf + off) = STOMP_COLON;
        off += 1;
        memcpy(buf + off, receipt, receipt_len);
        off += receipt_len;
        *(buf + off) = STOMP_NEW_LINE;
        off += 1;
    }

    *(buf + off) = STOMP_NEW_LINE;
    off += 1;

    if (off != total_len) {
        PRINTA("DISCONNECT: off(%d) != total_len(%d).\n", off, total_len);
    }
    PRINTA("\n^%s@\n", buf);
    
    DELETE(buf);
}

/* Ultra-simple-stomp network section */

void
stomp_net_connect();

#ifndef WITH_UDP

void
stomp_net_connected() {
    PRINTA("Connected.\n");
}

void
stomp_net_timedout() {
    PRINTA("Timedout.\n");
}

void
stomp_net_abort() {
    state.flags = ULTRA_SIMPLE_STOMP_FLAG_ABORT;
}

void
stomp_net_aborted() {
    PRINTA("Aborted.\n");
}

void
stomp_net_close() {
    state.flags = ULTRA_SIMPLE_STOMP_FLAG_DISCONNECT;
}

void
stomp_net_closed() {
    PRINTA("Closed.\n");
}
#endif

void
stomp_net_send(char *buf, int len) {
    /* TODO KURDE */
}

void
stomp_net_sent() {
    PRINTA("Sent.\n");
}

void
stomp_net_received(char *buf, int len) {
    PRINTA("Received: {buf=\"%s\", len=%d}.\n", buf, len);
}