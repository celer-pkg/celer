-- Copyright 2025 The Chromium Authors
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.
include PERFETTO MODULE sql_packages.web_tests_common.iterations;

include PERFETTO MODULE sql_packages.page_load.page_load_start;

-- Add the iteration id to the page load start table.
drop view if exists page_load_start_by_iteration;

create view
  page_load_start_by_iteration as
select
  page_load_start.id,
  iterations.id as it_id,
  page_load_start.page_load_start,
  page_load_start.detail
from
  iterations
  -- Join for all page loads that started during this iteration
  join page_load_start on page_load_start.page_load_start >= iterations.start
  and page_load_start.page_load_start <= iterations.end;