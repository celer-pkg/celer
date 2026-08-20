-- Copyright 2025 The Chromium Authors
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.
DROP TABLE IF EXISTS scroll_distance_output;

CREATE PERFETTO TABLE scroll_distance_output
AS
SELECT
  (
    SELECT
      CAST(string_value as float)
    FROM
      slice
      JOIN args ON slice.arg_set_id = args.arg_set_id
    WHERE
      slice.name = 'scroll-end'
      AND key = 'debug.data.detail'
      AND value_type = 'string'
  ) AS distance_px;
