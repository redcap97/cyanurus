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

        .global mmu_enable
mmu_enable:
        @ SCTLR
        MRC   p15, 0, r0, c1, c0, 0
        ORR   r0, r0, #0x1
        MCR   p15, 0, r0, c1, c0, 0
        BX    lr

        .global mmu_disable
mmu_disable:
        @ SCTLR
        MRC   p15, 0, r0, c1, c0, 0
        BIC   r0, r0, #0x1
        MCR   p15, 0, r0, c1, c0, 0
        BX    lr