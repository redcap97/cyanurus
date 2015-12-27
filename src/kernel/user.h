/*
Copyright 2015 Akira Midorikawa

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

#ifndef _CYANURUS_USER_H_
#define _CYANURUS_USER_H_

#define USER_ADDRESS_START ((uint8_t*)0x00010000)
#define USER_ADDRESS_END   ((uint8_t*)0x60000000)
#define IS_USER_ADDRESSS(address) ((uint8_t*)(address) >= USER_ADDRESS_START && (uint8_t*)(address) < USER_ADDRESS_END)

#endif
