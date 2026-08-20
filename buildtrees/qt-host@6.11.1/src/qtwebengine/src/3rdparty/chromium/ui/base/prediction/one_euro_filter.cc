// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/prediction/one_euro_filter.h"
#include "ui/base/ui_base_features.h"

namespace ui {

constexpr double OneEuroFilter::kDefaultFrequency;
constexpr double OneEuroFilter::kDefaultMincutoff;
constexpr double OneEuroFilter::kDefaultBeta;
constexpr double OneEuroFilter::kDefaultDcutoff;

constexpr char OneEuroFilter::kParamBeta[];
constexpr char OneEuroFilter::kParamMincutoff[];

OneEuroFilter::OneEuroFilter(double mincutoff, double beta) {
  CHECK_GE(mincutoff, 0);
  CHECK_GE(beta, 0);
  x_filter_ = std::make_unique<::OneEuroFilter>(kDefaultFrequency, mincutoff,
                                                beta, kDefaultDcutoff);
  y_filter_ = std::make_unique<::OneEuroFilter>(kDefaultFrequency, mincutoff,
                                                beta, kDefaultDcutoff);
}

OneEuroFilter::~OneEuroFilter() {}

bool OneEuroFilter::Filter(const base::TimeTicks& timestamp,
                           gfx::PointF* position) const {
  if (!position)
    return false;
  double ts = (timestamp - base::TimeTicks()).InSecondsF();
  position->set_x(x_filter_->filter(position->x(), ts));
  position->set_y(y_filter_->filter(position->y(), ts));
  return true;
}

const char* OneEuroFilter::GetName() const {
  return features::kFilterNameOneEuro;
}

}  // namespace ui
