/*
Copyright 2014 Akira Midorikawa

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

        .macro push_process_context
                STMFD sp!, {lr}

                STMFD sp, {r0-lr}^
                SUB   sp, sp, #60

                MRS   r1, spsr
                STMFD sp!, {r1}
        .endm

        .macro switch_kernel_stack
                @ skip if current_process is NULL
                LDR   r0, __current_process
                LDR   r0, [r0]
                CMP   r0, #0
                BEQ   1f

                @ store context to process
                MOV   r0, sp
                BL    process_set_context

                @ set kernel stack
                BL    process_get_kernel_stack
                MOV   sp, r0
        1:
        .endm

        .text
        .code 32

        .global vectors_start
        .global vectors_end

vectors_start:
        LDR   pc, __reset_handler
        LDR   pc, __undef_handler
        LDR   pc, __svc_handler
        LDR   pc, __prefetch_abort_handler
        LDR   pc, __data_abort_handler
        B     .
        LDR   pc, __irq_handler
        LDR   pc, __fiq_handler

__reset_handler:          .long reset_handler
__undef_handler:          .long system_halt
__svc_handler:            .long svc_handler
__prefetch_abort_handler: .long system_halt
__data_abort_handler:     .long data_abort_handler
__irq_handler:            .long irq_handler
__fiq_handler:            .long system_halt
vectors_end:

__current_process: .long current_process
__svc_stack: .long svc_stack
__irq_stack: .long irq_stack
__data_abort_stack: .long data_abort_stack

reset_handler:
        LDR   sp, __svc_stack
        BL    kernel_main
        B     .

        .global irq_handler
irq_handler:
        LDR   sp, __irq_stack

        SUB   lr, lr, #4
        push_process_context
        switch_kernel_stack

        BL    system_irq_handler
        B     .

        .global svc_handler
svc_handler:
        LDR   sp, __svc_stack

        push_process_context
        switch_kernel_stack

        BL    system_svc_handler
        B     .

        .global data_abort_handler
data_abort_handler:
        LDR   sp, __data_abort_stack

        SUB   lr, lr, #8
        push_process_context
        switch_kernel_stack

        BL    system_data_abort_handler
        B     .
