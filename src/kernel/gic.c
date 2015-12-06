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

#include "lib/type.h"
#include "gic.h"
#include "io.h"

#define GIC_DIST_BASE ((volatile uint8_t*)0x1e001000)
#define GIC_CPU_BASE  ((volatile uint8_t*)0x1e000100)

#define GICC_CTLR 0x0000
#define GICC_PMR  0x0004
#define GICC_IAR  0x000c
#define GICC_EOIR 0x0010

#define GICD_CTLR 0x0000
#define GICD_ISENABLER(n) (GIC_DIST_BASE + 0x100 + (n / 32) * 4)
#define GICD_ITARGETSR(n) (GIC_DIST_BASE + 0x800 + (n / 4)  * 4)
#define GICD_ICFGR(n)     (GIC_DIST_BASE + 0xc00 + (n / 16) * 4)

void gic_init(void) {
  // GICC_PMR:
  io_write32(GIC_CPU_BASE + GICC_PMR, 0xff);

  // GICC_CTLR:
  io_write32(GIC_CPU_BASE + GICC_CTLR, 0x1);

  // GICD_CTLR:
  io_write32(GIC_DIST_BASE + GICD_CTLR, 0x1);
}

void gic_enable_irq(int id) {
  // GICD_ISENABLERn:
  io_write32(GICD_ISENABLER(id), io_read32(GICD_ISENABLER(id)) | (0x1 << (id & 0x1f)));

  // GICD_ITARGETSRn:
  io_write32(GICD_ITARGETSR(id), io_read32(GICD_ITARGETSR(id)) | (0x1 << ((id & 0x3) * 8)));

  // GICD_ICFGRn:
  io_write32(GICD_ICFGR(id), io_read32(GICD_ICFGR(id)) & ~(0x3 << ((id & 0xf) * 2)));
}

uint32_t gic_interrupt_acknowledge(void) {
  // GICC_IAR
  return io_read32(GIC_CPU_BASE + GICC_IAR) & 0x3ff;
}

void gic_end_of_interrupt(uint32_t irq) {
  // GICC_EOIR
  io_write32(GIC_CPU_BASE + GICC_EOIR, irq & 0x3ff);
}
