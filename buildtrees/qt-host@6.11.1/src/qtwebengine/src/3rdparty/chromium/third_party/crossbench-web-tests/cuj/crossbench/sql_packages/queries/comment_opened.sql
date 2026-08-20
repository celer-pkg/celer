-- Copyright 2025 The Chromium Authors
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.
DROP TABLE IF EXISTS comment_opened_output;

CREATE PERFETTO TABLE comment_opened_output
AS
SELECT
  (
    SELECT
      (dur / 1000000)
    FROM
      slice
    WHERE
      slice.name = 'comment-opened'
  ) AS 'duration_ms';
