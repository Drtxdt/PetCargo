#!/bin/bash
if [ -f "build/bin/app.bin" ]; then
    if [ -f "tools/bin/csk5060-enc-pack" ]; then
        chip_id=CSK3021
        if [[ ! -z "$1" ]]; then
            chip_id=$1
            echo "chip_id is set to $chip_id"
		else
			echo "chip_id is set to $chip_id (default)"
        fi
        trace_baud=115200
        if [[ ! -z "$2" ]]; then
            trace_baud=$2
            echo "trace_baud is set to $trace_baud"
		else
			echo "trace_baud is set to $trace_baud (default)"
        fi

        if ./tools/bin/csk5060-enc-pack build/bin/app.bin build/bin/app.symb build/bin/app.img 0 $chip_id $trace_baud; then
			echo "IMG PACK SUCCESS!"
			exit 0
		fi
    fi
fi

echo "IMG PACK FAILED!"
exit 1