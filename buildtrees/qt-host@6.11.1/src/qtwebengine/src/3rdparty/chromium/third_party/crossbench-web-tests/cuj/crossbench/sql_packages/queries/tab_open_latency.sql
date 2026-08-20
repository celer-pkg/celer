-- Copyright 2025 The Chromium Authors
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.
include PERFETTO MODULE sql_packages.page_load.page_load_start_by_iteration;

-- Calculate the difference between every page_load_start and the
-- previous page_load_start for each iteration.
-- Note that this query will output NULL for the first tab of each
-- iteration since there is no previous tab to compare against.
DROP TABLE IF EXISTS tab_open_latency_output;
CREATE PERFETTO TABLE tab_open_latency_output AS
select
  it_id,
  id,
  (
    page_load_start - lag (page_load_start, 1, NULL) over (
      partition by
        it_id
      order by
        page_load_start
    )
  ) / 1000000 as open_latency
from
  page_load_start_by_iteration
order by
  it_id,
  id;
