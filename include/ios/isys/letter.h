#ifndef _ISYS_LETTER_H
#define _ISYS_LETTER_H

#include "common.h"




typedef struct LetterType {
    s32 unk0; // 0x0
    s32 receiverSize; // 0x4
    s32 receiverInit; // 0x8
    s32 unkC; // 0xc
    s32 dataSize; // 0x10
    s32 dataInit; // 0x14
} LetterType;

typedef struct Letter {
    s32 refCount; // 0x0
    f32 unk4; // 0x4
    f32 unk8; // 0x8
    s32 unkC; // 0xc
    LetterType* type; // 0x10
    s32 receiverObjGroup; // 0x14
    s32 receiverData; // 0x18
    s32 contentsData; // 0x1C
    s32 is_initialized; // 0x20
    s32 unk24; // 0x24
    s32 unk28; // 0x28
    s32 unk2C; // 0x2c
} Letter;

typedef struct LetterLink {
    volatile Letter* letter; // 0x0
    struct LetterLink* next; // 0x4
    struct LetterLink* prev; // 0x8
} LetterLink;

typedef struct LetterPost {
    s32 letterCount; // 0x0
    LetterLink* letters_tail; // 0x4
    LetterLink* letters_head; // 0x8
    LetterLink* read; // 0xc
    LetterLink* reading; // 0x10
} LetterPost;

struct letter_unkstrc {
    s32 unk0, unk4;
    u8 unk8, unk9, unkA, unkB;
    s32 unkC, unk10, unk14, unk18;
    LetterPost* unk1C;
    s32 unk20;
};




extern s32* g_pLetterMem;



void isysInitISYSLetterObj(struct letter_unkstrc* arg0);
void isysFlushISYSLetterObj(struct letter_unkstrc* arg0);
struct letter_unkstrc* isysGetLetterObj(struct letter_unkstrc* arg0);
void isysLetterObjDiscardLetterAll(struct letter_unkstrc* arg0);
Letter* create_letter(s32* arg0, s32* arg1, s32 arg2, s32 arg3, s32 arg4);
void init_letter(Letter* arg0, s32* arg1, s32* arg2);
s32 is_hope_letter_type(Letter* arg0, s32 arg1);
void* get_letter_contents_data(Letter* arg0);
void increment_letter_refcount(Letter* arg0);
s32 decrement_letter_refcount(LetterLink* arg0);
s32 is_letter_reach_time(Letter* arg0, f32 arg1);
s32 is_init_letter_called(Letter* arg0);
LetterType* get_reference_letter_type(Letter* arg0);
s32 get_letter_receiver_obj_group(Letter* arg0);
s32 get_letter_receiver_data(Letter* arg0);
void doit_letter_manager(void);
void init_letter_manager(void);
void send_letter(LetterLink* arg0);
LetterPost* create_letter_post(void);
void check_free_letter_post(LetterPost** arg0);
void discard_all_letter_in_post(LetterPost* arg0);
s32 get_total_letter_in_post(LetterPost* arg0);
void open_letter_post(LetterPost* arg0);
Letter* get_next_letter_in_post(LetterPost* post);
void close_letter_post(LetterPost* arg0);
void push_back_letter_in_post(LetterPost* post, volatile Letter* letter);
void discard_selecting_letter_in_post(LetterPost* arg0);
void constructor_letter_type(LetterType* arg0);
s32 entry_new_letter_type(LetterType* arg0);
void init_letter_type_store(void);
LetterType* get_letter_type(u32 arg0);





#endif /* _ISYS_LETTER_H */
