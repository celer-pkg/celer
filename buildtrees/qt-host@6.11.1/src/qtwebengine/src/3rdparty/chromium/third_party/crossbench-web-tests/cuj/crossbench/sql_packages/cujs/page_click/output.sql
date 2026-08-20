-- Copyright 2025 The Chromium Authors
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.
drop view if exists page_load_start_end;

create view
  page_load_start_end as
select
  *
from
  (
    select
      row_number() over (
        order by
          ts
      ) as id,
      ts as page_load_start
    from
      slice
    where
      category = 'blink.user_timing'
      and name = 'page-load'
  )
  join (
    select
      row_number() over (
        order by
          ts
      ) as id,
      ts as page_load_end
    from
      slice
    where
      category = 'blink.user_timing'
      and name = 'page-loaded'
  ) using (id);

drop view if exists page_load_dur;

create view
  page_load_dur as
select
  id,
  (
    select
      dur / 1000000
    from
      slice s
    where
      s.name = 'PageLoadMetrics.NavigationToLargestContentfulPaint'
      and s.ts > plse.page_load_start
      and s.ts + dur < plse.page_load_end
  ) as page_load_time
from
  page_load_start_end plse;

drop view if exists link_click_start_end;

create view
  link_click_start_end as
select
  *
from
  (
    select
      row_number() over (
        order by
          ts
      ) as id,
      ts as link_click_start
    from
      slice
    where
      category = 'blink.user_timing'
      and name = 'click-link-on-page'
  )
  join (
    select
      row_number() over (
        order by
          ts
      ) as id,
      ts as link_click_end
    from
      slice
    where
      category = 'blink.user_timing'
      and name = 'click-link-on-page-loaded'
  ) using (id);

drop view if exists link_click_time;

create view
  link_click_time as
select
  id,
  (
    select
      s.ts
    from
      slice s
    where
      s.name = 'EventLatency'
      and extract_arg (s.arg_set_id, 'event_latency.event_type') = 'GESTURE_TAP_DOWN'
      and s.ts > lcse.link_click_start
      and s.ts + s.dur < lcse.link_click_end
  ) as link_click_ts
from
  link_click_start_end lcse;

drop view if exists link_click_loaded;

create view
  link_click_loaded as
select
  id,
  (
    select
      s.ts + s.dur
    from
      slice s
    where
      s.name = 'PageLoadMetrics.NavigationToLargestContentfulPaint'
      and s.ts > lcse.link_click_start
      and s.ts + s.dur < lcse.link_click_end
  ) as link_click_loaded_ts
from
  link_click_start_end lcse;

drop view if exists link_click_dur;

create view
  link_click_dur as
select
  id,
  ((link_click_loaded_ts - link_click_ts) / 1000000) as link_load_time
from
  link_click_time
  join link_click_loaded using (id);

create perfetto table page_click_output as
select
  *
from
  page_load_dur
  join link_click_dur using (id);
