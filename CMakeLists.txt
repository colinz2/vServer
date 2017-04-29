cmake_minimum_required (VERSION 2.8)
project (bsd)
set(BSD_SRC 
    core/dev_event.c
    core/dev_event_timer.c
    core/dev_heap.c
    bs_log.c
    bs_main.c
    bs_packet.c
    bs_snmp.c
    bs_utils.c
    bs_vserver.c)
add_executable(bsd ${BSD_SRC})
