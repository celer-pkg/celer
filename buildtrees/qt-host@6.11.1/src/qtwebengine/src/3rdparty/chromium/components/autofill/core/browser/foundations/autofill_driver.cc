// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/foundations/autofill_driver.h"

#include "base/check.h"

namespace autofill {

namespace {

#if DCHECK_IS_ON()
// Returns true iff the transition is a valid transition according to the
// diagram in AutofillDriver's class-level documentation.
// LINT.IfChange(LifecycleStateChanges)
bool IsValidTransition(AutofillDriver::LifecycleState previous_state,
                       AutofillDriver::LifecycleState old_state,
                       AutofillDriver::LifecycleState new_state) {
  switch (old_state) {
    case AutofillDriver::LifecycleState::kInactive:
      return new_state == AutofillDriver::LifecycleState::kActive || new_state == AutofillDriver::LifecycleState::kPendingReset ||
             new_state == AutofillDriver::LifecycleState::kPendingDeletion;
    case AutofillDriver::LifecycleState::kActive:
      return new_state == AutofillDriver::LifecycleState::kInactive || new_state == AutofillDriver::LifecycleState::kPendingReset ||
             new_state == AutofillDriver::LifecycleState::kPendingDeletion;
    case AutofillDriver::LifecycleState::kPendingReset:
      return new_state == previous_state &&
             (new_state == AutofillDriver::LifecycleState::kInactive || new_state == AutofillDriver::LifecycleState::kActive);
    case AutofillDriver::LifecycleState::kPendingDeletion:
      return false;
  }
  return false;
}
// LINT.ThenChange(autofill_driver.h:LifecycleStateChanges)
#endif

}  // namespace

AutofillDriver::~AutofillDriver() {
  CHECK_EQ(lifecycle_state_, LifecycleState::kPendingDeletion);
}

void AutofillDriver::SetLifecycleState(
    LifecycleState new_state,
    base::PassKey<AutofillDriverFactory> pass_key) {
  DCHECK_NE(lifecycle_state_, new_state);
#if DCHECK_IS_ON()
  DCHECK(
      IsValidTransition(previous_lifecycle_state_, lifecycle_state_, new_state))
      << "Invalid AutofillDriver::LifecycleState change "
      << base::to_underlying(previous_lifecycle_state_) << " -> "
      << base::to_underlying(lifecycle_state_) << " -> "
      << base::to_underlying(new_state);
  previous_lifecycle_state_ = lifecycle_state_;
#endif

  lifecycle_state_ = new_state;
}

}  // namespace autofill
