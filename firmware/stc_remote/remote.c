#include "remote.h"
uint8_t remote_decode(uint8_t v){
    if(v<=28)return CMD_STOP;if(v<=76)return CMD_RIGHT;
    if(v<=116)return CMD_BACKWARD;if(v<=156)return CMD_STOP;
    if(v<=188)return CMD_LEFT;if(v<=228)return CMD_FORWARD;return CMD_IDLE;
}
void remote_sample(remote_keys_t *s,uint8_t adc,uint8_t valid,uint32_t now){
    uint8_t command=remote_decode(adc);
    if(!valid){
        if(s->valid||s->stable!=CMD_IDLE){s->release_frames=2;s->next_send=now;}
        s->valid=0;s->stable=s->candidate=CMD_IDLE;s->changed_at=now;return;
    }
    if(!s->valid){s->valid=1;s->candidate=command;s->changed_at=now;}
    if(command!=s->candidate){s->candidate=command;s->changed_at=now;}
    if(command!=s->stable&&(int32_t)(now-s->changed_at)>=30){
        s->stable=command;s->next_send=now;
        s->release_frames=(command==CMD_IDLE)?2:0;
    }
}
uint8_t remote_next(remote_keys_t *s,uint32_t now){
    if((int32_t)(now-s->next_send)<0)return 0xFF;
    if(s->release_frames){s->release_frames--;s->next_send=now+30;return CMD_IDLE;}
    if(s->valid&&s->stable!=CMD_IDLE){s->next_send=now+100;return s->stable;}
    return 0xFF;
}
uint8_t remote_checksum(const uint8_t *data,uint8_t length){
    uint8_t value=0;while(length--)value^=*data++;return value;
}
