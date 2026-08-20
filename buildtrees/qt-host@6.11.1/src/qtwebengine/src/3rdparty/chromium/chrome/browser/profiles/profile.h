// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This a QtWebEngine specific stripped down replacement of Chromiums's Profile
// class, because it is used many places just for preference access.

#ifndef CHROME_BROWSER_PROFILES_PROFILE_H_
#define CHROME_BROWSER_PROFILES_PROFILE_H_

#include "base/memory/weak_ptr.h"
#include "content/public/browser/browser_context.h"

class PrefService;

namespace content {
class WebUI;
}

class Profile : public content::BrowserContext {
 public:
  // Returns the profile corresponding to the given browser context.
  static Profile* FromBrowserContext(content::BrowserContext* browser_context);

  // Returns the profile corresponding to the given WebUI.
  static Profile* FromWebUI(content::WebUI* web_ui);

  // Retrieves a pointer to the PrefService that manages the
  // preferences for this user profile.
  virtual PrefService* GetPrefs() = 0;
  virtual const PrefService* GetPrefs() const = 0;

  Profile *GetOriginalProfile();
  const Profile *GetOriginalProfile() const;

  // Returns whether the profile is new.  A profile is new if the browser has
  // not been shut down since the profile was created.
  virtual bool IsNewProfile() const = 0;

  // Returns whether it's a regular profile.
  bool IsRegularProfile() const;

  // Returns whether it is an Incognito profile. An Incognito profile is an
  // off-the-record profile that is used for incognito mode.
  bool IsIncognitoProfile() const;

  // Returns whether it is a system profile.
  bool IsSystemProfile() const;

  // Returns whether it is a Guest session. This covers both regular and
  // off-the-record profiles of a Guest session.
  virtual bool IsGuestSession() const;

  base::WeakPtr<Profile> GetWeakPtr();

  private:
  base::WeakPtrFactory<Profile> weak_ptr_factory_{this};

};

#endif  // CHROME_BROWSER_PROFILES_PROFILE_H_
