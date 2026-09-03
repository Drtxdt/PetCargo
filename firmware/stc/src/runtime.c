#include "runtime.h"
static uint8_t reached(uint32_t n,uint32_t t){return (int32_t)(n-t)>=0;}
uint8_t button_update(button_state_t *b,uint8_t raw,uint8_t valid,uint32_t now)
{
    uint8_t e=0;
    if(!valid){b->changed_at=now;return 0;}
    if(raw!=b->candidate){b->candidate=raw;b->changed_at=now;}
    if(b->candidate!=b->stable&&reached(now,b->changed_at+30)){
        b->stable=b->candidate;
        if(b->stable){b->pressed_at=now;b->long_sent=0;e=BUTTON_PRESS;}
        else if(!b->long_sent)e=BUTTON_SHORT;
    }
    if(b->stable&&!b->long_sent&&reached(now,b->pressed_at+1200)){b->long_sent=1;e|=BUTTON_LONG;}
    return e;
}
uint8_t nav_is_k3(uint8_t raw){return raw<=28;}
void csk_expire(csk_parser_t *p,uint32_t now)
{
    if(p->state&&reached(now,p->last_at+100)){p->state=0;p->errors++;}
}
uint8_t csk_feed(csk_parser_t *p,uint8_t v,uint32_t now)
{
    uint8_t result=0,i;
    csk_expire(p,now);p->last_at=now;p->bytes++;
    for(i=0;i<3;i++)p->last[i]=p->last[i+1];p->last[3]=v;
    switch(p->state){
    case 0:if(v==0xA5)p->state=1;break;
    case 1:if(v==0x5A)p->state=2;else {p->errors++;p->state=v==0xA5?1:0;}break;
    case 2:if(v>=1&&v<=12){p->command=v;p->state=3;}else{p->errors++;p->state=v==0xA5?1:0;}break;
    case 3:
        if(v==(uint8_t)(p->command^0xFF)){result=p->command;p->frames++;p->last_command=result;p->state=0;}
        else {p->errors++;p->state=v==0xA5?1:0;}break;
    default:p->state=0;break;
    }
    return result;
}
uint8_t tx_enqueue(tx_queue_t *q,const uint8_t *data,uint8_t n,uint8_t urgent)
{
    uint8_t i,next=(q->head+1)&(TX_SLOTS-1);
    if(!n||n>TX_FRAME_MAX){q->dropped++;return 0;}
    if(urgent){
        /* Finish the already-started frame. Cancel ALL queued stale work. */
        q->tail=q->head;
        for(i=0;i<n;i++)q->urgent[i]=data[i];q->urgent_size=n;return 1;
    }
    if(next==q->tail){q->dropped++;return 0;}
    for(i=0;i<n;i++)q->data[q->head][i]=data[i];
    q->length[q->head]=n;q->head=next;return 1;
}
uint8_t tx_next(tx_queue_t *q,uint8_t *value)
{
    uint8_t i;
    if(q->pos==q->size){
        if(q->urgent_size){q->size=q->urgent_size;for(i=0;i<q->size;i++)q->active[i]=q->urgent[i];q->urgent_size=0;}
        else if(q->tail!=q->head){q->size=q->length[q->tail];for(i=0;i<q->size;i++)q->active[i]=q->data[q->tail][i];q->tail=(q->tail+1)&(TX_SLOTS-1);}
        else return 0;
        q->pos=0;
    }
    *value=q->active[q->pos++];return 1;
}
uint16_t music_duration(uint8_t ticks,uint16_t *remainder)
{
    /* A tick is one sixteenth, 136 quarter notes/minute. Carry fractional ms. */
    uint32_t numerator=15000UL*ticks+*remainder;
    *remainder=(uint16_t)(numerator%136UL);return (uint16_t)(numerator/136UL);
}
