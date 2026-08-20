-- Copyright 2025 The Chromium Authors
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.
include PERFETTO MODULE sql_packages.page_load.page_load_start_by_iteration;

include PERFETTO MODULE sql_packages.page_load.page_load_end_by_iteration;

-- Group the page load start and end together.
drop view if exists page_load_start_end_by_iteration;

create view
  page_load_start_end_by_iteration as
select
  page_load_start_by_iteration.id as id,
  page_load_start_by_iteration.it_id as it_id,
  page_load_start_by_iteration.page_load_start,
  page_load_end_by_iteration.page_load_end,
  page_load_start_by_iteration.detail
from
  page_load_start_by_iteration
  -- The start and end are joined using the detail and the iteration id columns because
  -- 'page-load' and 'page-loaded' for a specific tab are not on the
  -- same track (thread).
  join page_load_end_by_iteration using (detail, it_id)
order by
  id;