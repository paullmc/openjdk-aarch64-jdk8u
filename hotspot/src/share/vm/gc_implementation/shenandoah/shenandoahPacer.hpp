/*
 * Copyright (c) 2018, Red Hat, Inc. All rights reserved.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef SHARE_VM_GC_SHENANDOAH_SHENANDOAHPACER_HPP
#define SHARE_VM_GC_SHENANDOAH_SHENANDOAHPACER_HPP

#include "gc_implementation/shenandoah/shenandoahNumberSeq.hpp"
#include "memory/allocation.hpp"

class ShenandoahHeap;

#define PACING_PROGRESS_UNINIT (-1)
#define PACING_PROGRESS_ZERO   ( 0)

/**
 * ShenandoahPacer provides allocation pacing mechanism.
 *
 * Currently it implements simple tax-and-spend pacing policy: GC threads provide
 * credit, allocating thread spend the credit, or stall when credit is not available.
 */
class ShenandoahPacer : public CHeapObj<mtGC> {
private:
  ShenandoahHeap* _heap;
  BinaryMagnitudeSeq _delays;
  TruncatedSeq* _progress_history;

  volatile intptr_t _epoch;
  volatile jdouble _tax_rate;

  char _pad0[DEFAULT_CACHE_LINE_SIZE];
  volatile intptr_t _budget;
  char _pad1[DEFAULT_CACHE_LINE_SIZE];
  volatile intptr_t _progress;
  char _pad2[DEFAULT_CACHE_LINE_SIZE];

public:
  ShenandoahPacer(ShenandoahHeap* heap) :
          _heap(heap),
          _progress_history(new TruncatedSeq(5)),
          _epoch(0),
          _tax_rate(1),
          _budget(0),
          _progress(PACING_PROGRESS_UNINIT) {
  }

  void setup_for_idle();
  void setup_for_mark();
  void setup_for_evac();
  void setup_for_updaterefs();
  void setup_for_traversal();

  void setup_for_reset();
  void setup_for_preclean();

  inline void report_mark(size_t words);
  inline void report_evac(size_t words);
  inline void report_updaterefs(size_t words);

  inline void report_alloc(size_t words);

  bool claim_for_alloc(size_t words, bool force);
  void pace_for_alloc(size_t words);
  void unpace_for_alloc(intptr_t epoch, size_t words);

  intptr_t epoch();

  void print_on(outputStream* out) const;

private:
  inline void report_internal(size_t words);
  inline void report_progress_internal(size_t words);

  void restart_with(jlong non_taxable_bytes, jdouble tax_rate);

  size_t update_and_get_progress_history();
};

#endif //SHARE_VM_GC_SHENANDOAH_SHENANDOAHPACER_HPP
