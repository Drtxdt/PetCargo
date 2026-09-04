#include <assert.h>
#include <stdio.h>
#include "remote.h"
int main(void){
    uint16_t value;remote_keys_t state={0};uint8_t frame[5]={0xA5,0x5A,2,4,9};
    for(value=0;value<256;value++)
        assert(remote_decode((uint8_t)value)==(value<=28?5:value<=76?4:value<=116?2:value<=156?5:value<=188?3:value<=228?1:0));
    remote_sample(&state,200,1,0);assert(remote_next(&state,29)==0xFF);
    remote_sample(&state,200,1,30);assert(state.stable==1&&remote_next(&state,30)==1);
    assert(remote_next(&state,129)==0xFF&&remote_next(&state,130)==1);
    remote_sample(&state,255,1,140);remote_sample(&state,255,1,170);
    assert(state.stable==0&&remote_next(&state,170)==0);
    assert(remote_next(&state,199)==0xFF&&remote_next(&state,200)==0&&remote_next(&state,230)==0xFF);
    remote_sample(&state,90,1,240);remote_sample(&state,90,1,270);
    assert(state.stable==2&&remote_next(&state,270)==2);
    remote_sample(&state,90,0,280);assert(!state.valid&&state.stable==0);
    assert(remote_next(&state,280)==0&&remote_next(&state,310)==0);
    assert(remote_checksum(frame,5)==(uint8_t)(0xA5^0x5A^2^4^9));
    puts("PASS all ADC boundaries, 30ms debounce, 100ms repeat, two release frames, ADC fail-stop and XOR checksum");
    return 0;
}
