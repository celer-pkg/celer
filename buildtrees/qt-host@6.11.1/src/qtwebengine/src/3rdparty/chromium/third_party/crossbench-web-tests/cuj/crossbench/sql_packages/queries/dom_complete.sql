-- Copyright 2025 The Chromium Authors
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.
include PERFETTO MODULE sql_packages.web_tests_common.iterations;

DROP TABLE IF EXISTS dom_complete_output;

CREATE PERFETTO TABLE dom_complete_output AS
SELECT
  iterations.id as it_id,
  ROW_NUMBER() OVER (ORDER BY ts) as id,
  CAST(json_extract(EXTRACT_ARG(arg_set_id, 'debug.data.detail'), '$.domComplete') AS FLOAT) dom_complete,
  CAST(json_extract(EXTRACT_ARG(arg_set_id, 'debug.data.detail'), '$.domInteractive') AS FLOAT) dom_interactive
FROM slice JOIN iterations
  ON slice.ts >= iterations.start AND slice.ts <= iterations.end
WHERE
  category = 'blink.user_timing'
  AND name = 'page-loaded';