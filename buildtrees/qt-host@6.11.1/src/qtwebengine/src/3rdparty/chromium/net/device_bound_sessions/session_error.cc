// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/device_bound_sessions/session_error.h"

namespace net::device_bound_sessions {

SessionError::SessionError(SessionError::ErrorType type) : type(type) {}

SessionError::~SessionError() = default;

SessionError::SessionError(SessionError&&) noexcept = default;
SessionError& SessionError::operator=(SessionError&&) noexcept = default;

bool SessionError::IsFatal() const {
  // using enum ErrorType;

  switch (type) {
    case ErrorType::kSuccess:
      return false;
    case ErrorType::kKeyError:
    case ErrorType::kSigningError:
    case ErrorType::kServerRequestedTermination:
    case ErrorType::kInvalidConfigJson:
    case ErrorType::kInvalidSessionId:
    case ErrorType::kInvalidCredentials:
    case ErrorType::kInvalidChallenge:
    case ErrorType::kTooManyChallenges:
    case ErrorType::kInvalidFetcherUrl:
    case ErrorType::kInvalidRefreshUrl:
    case ErrorType::kPersistentHttpError:
    case ErrorType::kScopeOriginSameSiteMismatch:
    case ErrorType::kRefreshUrlSameSiteMismatch:
    case ErrorType::kInvalidScopeOrigin:
    case ErrorType::kMismatchedSessionId:
    case ErrorType::kInvalidRefreshInitiators:
    case ErrorType::kInvalidScopeRule:
    case ErrorType::kMissingScope:
    case ErrorType::kNoCredentials:
    case ErrorType::kInvalidScopeIncludeSite:
      return true;

    case ErrorType::kNetError:
    case ErrorType::kTransientHttpError:
      return false;
  }
}

bool SessionError::IsServerError() const {
  // using enum ErrorType;

  switch (type) {
    case ErrorType::kSuccess:
    case ErrorType::kKeyError:
    case ErrorType::kSigningError:
    case ErrorType::kNetError:
      return false;
    case ErrorType::kServerRequestedTermination:
    case ErrorType::kInvalidConfigJson:
    case ErrorType::kInvalidSessionId:
    case ErrorType::kInvalidCredentials:
    case ErrorType::kInvalidChallenge:
    case ErrorType::kTooManyChallenges:
    case ErrorType::kInvalidFetcherUrl:
    case ErrorType::kInvalidRefreshUrl:
    case ErrorType::kPersistentHttpError:
    case ErrorType::kScopeOriginSameSiteMismatch:
    case ErrorType::kRefreshUrlSameSiteMismatch:
    case ErrorType::kInvalidScopeOrigin:
    case ErrorType::kTransientHttpError:
    case ErrorType::kMismatchedSessionId:
    case ErrorType::kInvalidRefreshInitiators:
    case ErrorType::kInvalidScopeRule:
    case ErrorType::kMissingScope:
    case ErrorType::kNoCredentials:
    case ErrorType::kInvalidScopeIncludeSite:
      return true;
  }
}

}  // namespace net::device_bound_sessions
