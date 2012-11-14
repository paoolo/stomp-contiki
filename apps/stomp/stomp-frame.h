#ifndef FRAME_H_
#define FRAME_H_

#define STOMP_NULL 0x00
#define STOMP_COLON 0x3a
#define STOMP_NEW_LINE 0x0a

#define STOMP_HEADER_NAME_LEN 21
#define STOMP_HEADER_VALUE_LEN 64

#define STOMP_FRAME_COMMAND_LEN 12
#define STOMP_FRAME_PAYLOAD_LEN 128

struct stomp_header {
    struct stomp_header *next;
    char name[STOMP_HEADER_NAME_LEN];
    char value[STOMP_HEADER_VALUE_LEN];
};

struct stomp_frame {
    struct stomp_header *headers;
    char command[STOMP_FRAME_COMMAND_LEN];
    char payload[STOMP_FRAME_PAYLOAD_LEN];
};

void
stomp_frame_delete_header(struct stomp_header *header);

void
stomp_frame_delete_frame(struct stomp_frame *frame);

struct stomp_header*
stomp_frame_new_header(const char *name, const char *value);

struct stomp_header*
stomp_frame_add_header(const char *name, const char *value, struct stomp_header *headers);

struct stomp_frame*
stomp_frame_new_frame(const char *command, struct stomp_header *headers, const char *payload);

struct stomp_header*
stomp_frame_find_header(const char *name, struct stomp_frame *frame);

struct stomp_frame*
stomp_frame_import(const char *stream, struct stomp_frame *frame);

void
stomp_frame_export(struct stomp_frame *frame, char *stream, int length);

void
stomp_frame_print(struct stomp_frame *frame);

void
stomp_frame_raw(struct stomp_frame *frame);

int
stomp_frame_length(struct stomp_frame *frame);

#endif /* FRAME_H_ */
