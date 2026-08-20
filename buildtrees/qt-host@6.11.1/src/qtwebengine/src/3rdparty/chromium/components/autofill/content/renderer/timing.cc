// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/content/renderer/timing.h"

#include "base/logging.h"
#include "base/metrics/histogram_functions.h"
#include "base/notreached.h"
#include "base/strings/strcat.h"

namespace autofill {

namespace {
std::string_view CallSiteToString(CallTimerState::CallSite call_site) {
  using CTS = CallTimerState::CallSite;
  switch (call_site) {
    case CTS::kApplyFieldsAction:
      return "ApplyFieldsAction";
    case CTS::kBatchSelectOptionChange:
      return "BatchSelectOptionChange";
    case CTS::kDidChangeScrollOffsetImpl:
      return "DidChangeScrollOffsetImpl";
    case CTS::kExtractForm:
      return "ExtractForm";
    case CTS::kFocusedElementChanged:
      return "FocusedElementChanged";
    case CTS::kGetFormDataFromUnownedInputElements:
      return "GetFormDataFromUnownedInputElements";
    case CTS::kGetFormDataFromWebForm:
      return "GetFormDataFromWebForm";
    case CTS::kGetSubmittedForm:
      return "GetSubmittedForm";
    case CTS::kHandleCaretMovedInFormField:
      return "HandleCaretMovedInFormField";
    case CTS::kJavaScriptChangedValue:
      return "JavaScriptChangedValue";
    case CTS::kNotifyPasswordManagerAboutClearedForm:
      return "NotifyPasswordManagerAboutClearedForm";
    case CTS::kOnProvisionallySaveForm:
      return "OnProvisionallySaveForm";
    case CTS::kOnTextFieldValueChanged:
      return "OnTextFieldValueChanged";
    case CTS::kQueryAutofillSuggestions:
      return "QueryAutofillSuggestions";
    case CTS::kShowSuggestionPopup:
      return "ShowSuggestionPopup";
    case CTS::kUpdateFormCache:
      return "UpdateFormCache";
    case CTS::kUpdateLastInteractedElement:
      return "UpdateLastInteractedElement";
    case CTS::kDidDispatchDomContentLoadedEvent:
      return "DidDispatchDomContentLoadedEvent";
    case CTS::kEmitFormIssuesToDevtools:
      return "EmitFormIssuesToDevtools";
    case CTS::kExtractForms:
      return "ExtractForms";
    case CTS::kExtractFormsAndNotifyPasswordAutofillAgent:
      return "ExtractFormsAndNotifyPasswordAutofillAgent";
  }
  NOTREACHED();
}
}  // namespace

ScopedCallTimer::ScopedCallTimer(const char* name, CallTimerState state)
    : state_(state), name_(name) {}

ScopedCallTimer::~ScopedCallTimer() {
  if (!base::TimeTicks::IsHighResolution()) {
    return;
  }
  base::TimeTicks after = base::TimeTicks::Now();
  std::string_view call_site = CallSiteToString(state_.call_site);

  {
    // Emit the duration of the timer's scope.
    auto record = [this](base::TimeDelta value, std::string_view suffix) {
      static constexpr std::string_view kPrefix = "Autofill.TimingPrecise.";
      base::UmaHistogramCustomMicrosecondsTimes(
          base::StrCat({kPrefix, name_, suffix.empty() ? "" : ".", suffix}),
          value, base::Microseconds(1), base::Seconds(1), 100);
    };
    record(after - before_, "");
    record(after - before_, call_site);
  }

  {
    // Emit the interval metrics from `state_.last_*` until the end of the
    // timer's scope.
    auto record = [this, call_site](base::TimeDelta value,
                                    std::string_view suffix) {
      static constexpr std::string_view kPrefix = "Autofill.TimingInterval.";
      DCHECK(!suffix.empty());
      base::UmaHistogramCustomMicrosecondsTimes(
          base::StrCat({kPrefix, name_, ".", call_site, ".", suffix}), value,
          base::Microseconds(1), base::Seconds(10), 100);
    };
    record(after - state_.last_autofill_agent_reset, "AutofillAgentReset");
    record(after - state_.last_dom_content_loaded, "DOMContentLoaded");
  }
}

}  // namespace autofill
