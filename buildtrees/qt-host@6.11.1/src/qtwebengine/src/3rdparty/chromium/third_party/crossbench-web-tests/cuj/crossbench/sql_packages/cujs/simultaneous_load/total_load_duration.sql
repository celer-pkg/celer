-- Copyright 2025 The Chromium Authors
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.
include PERFETTO MODULE sql_packages.page_load.page_load_start_end_by_iteration;

-- Get only the first page_load_start for each iteration
drop view if exists first_page_load_starts;

create view
  first_page_load_starts as
select
  it_id,
  page_load_start
from
  (
    select
      it_id,
      page_load_start,
      row_number() over (
        partition by
          it_id
        order by
          page_load_start
      ) as rn
    from
      page_load_start_end_by_iteration
  ) as ranked_rows
where
  rn = 1;

-- Get only the last page_load_end for each iteration
drop view if exists last_page_load_ends;

create view
  last_page_load_ends as
select
  it_id,
  page_load_end
from
  (
    select
      it_id,
      page_load_end,
      row_number() over (
        partition by
          it_id
        order by
          page_load_end desc
      ) as rn
    from
      page_load_start_end_by_iteration
  ) as ranked_rows
where
  rn = 1;

-- Get the time difference of the first page load starts
-- and last page load ends for each iteration
DROP TABLE IF EXISTS total_load_duration_output;
CREATE PERFETTO TABLE total_load_duration_output AS
select
  first_page_load_starts.it_id,
  (
    last_page_load_ends.page_load_end - first_page_load_starts.page_load_start
  ) / 1000000 as iteration_load_duration
from
  first_page_load_starts
  join last_page_load_ends on first_page_load_starts.it_id = last_page_load_ends.it_id
