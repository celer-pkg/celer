-- Copyright 2025 The Chromium Authors
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.

include PERFETTO MODULE sql_packages.web_tests_common.setup_blocks;

-- The test may have been run multiple times in the same trace.
-- Grab the start and end ts for each iteration.
drop view if exists iterations_no_startup;

create view
  iterations_no_startup as
select
  *
from
  (
    select
      row_number() over (
        order by
          ts
      ) as id,
      ts as start
    from
      slice
    where
      category = 'blink.user_timing'
      and name = 'crossbench-iteration-start'
  )
  join (
    select
      row_number() over (
        order by
          ts
      ) as id,
      ts as end
    from
      slice
    where
      category = 'blink.user_timing'
      and name = 'crossbench-iteration-end'
  ) using (id)
order by
  id;

drop view if exists iterations;

create view
  iterations as
select
  name as id, start, end
from setup_blocks
union
select
  cast(id as TEXT) as id,
  start,
  end
from iterations_no_startup;