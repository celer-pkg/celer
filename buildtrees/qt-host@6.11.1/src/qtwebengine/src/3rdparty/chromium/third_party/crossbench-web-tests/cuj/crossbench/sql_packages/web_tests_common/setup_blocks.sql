-- Copyright 2025 The Chromium Authors
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.
DROP VIEW IF EXISTS setup_blocks_start;
CREATE view setup_blocks_start AS
SELECT ts, EXTRACT_ARG(slice.arg_set_id, 'debug.data.detail') as name
FROM slice
WHERE category = 'blink.user_timing'
  AND name = 'crossbench-setup-start';

DROP VIEW IF EXISTS setup_blocks_end;
CREATE view setup_blocks_end AS
SELECT ts, EXTRACT_ARG(slice.arg_set_id, 'debug.data.detail') as name
FROM slice
WHERE slice.category = 'blink.user_timing'
  AND slice.name = 'crossbench-setup-end';

DROP VIEW IF EXISTS setup_blocks;
CREATE view
  setup_blocks AS
SELECT
  concat(
    "setup_",
    -- performance.mark detail values are logged as JSON. We log a single string
    -- which gets quoted to convert to a JSON string in the event. Remove the
    --quotes.
    substr(setup_blocks_start.name, 2, length(setup_blocks_start.name) - 2)
  ) AS name,
  setup_blocks_start.ts AS start,
  setup_blocks_end.ts AS end
FROM setup_blocks_start JOIN setup_blocks_end
  ON setup_blocks_start.name = setup_blocks_end.name;