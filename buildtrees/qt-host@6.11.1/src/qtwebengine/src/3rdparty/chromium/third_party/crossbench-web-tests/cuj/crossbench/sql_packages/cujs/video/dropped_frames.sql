-- Copyright 2025 The Chromium Authors
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.
INCLUDE PERFETTO MODULE sql_packages.web_tests_common.histograms;

DROP TABLE IF EXISTS dropped_frames_output;
CREATE PERFETTO TABLE dropped_frames_output AS
select
  AVG(value) as 'avg_percent_dropped'
from
  chrome_histograms
where
  name = 'Graphics.Smoothness.PercentDroppedFrames3.AllSequences'
