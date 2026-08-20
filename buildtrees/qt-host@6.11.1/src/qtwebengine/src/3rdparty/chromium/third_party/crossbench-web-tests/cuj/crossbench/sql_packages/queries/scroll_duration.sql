-- Copyright 2025 The Chromium Authors
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.
DROP TABLE IF EXISTS scroll_duration_output;

CREATE PERFETTO TABLE scroll_duration_output
AS
SELECT
  (
    SELECT
      (dur / 1000000)
    FROM
      slice
    WHERE
      slice.name = 'scroll'
  ) AS 'duration_ms';
