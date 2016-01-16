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

        .text
        .code 32

        .global system_dispatch
system_dispatch:
        MOV   lr, r0

        LDMFD lr!, {r0}
        MSR   spsr, r0

        LDMFD lr, {r0-lr}^
        ADD   lr, lr, #60

        LDMFD lr!, {pc}^

        .global system_suspend
system_suspend:
        ADD   r0, r0, #68

        STMFD r0!, {lr}
        STMFD r0!, {r4-lr}
        SUB   r0, r0, #16

        MRS   r1, cpsr
        STMFD r0!, {r1}

        BL    process_switch
        B     .

        .global system_resume
system_resume:
        LDMFD r0!, {r1}
        MSR   cpsr, r1
        ADD   r0, r0, #16
        LDMFD r0!, {r4-pc}

        .global system_idle
system_idle:
        MRS   r0, cpsr
        ORR   r0, r0, #0x1f
        BIC   r0, r0, #0x80
        MSR   cpsr, r0
        WFI
