-- Copyright 2025 The Chromium Authors
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.
include PERFETTO MODULE sql_packages.web_tests_common.iterations;

drop view if exists meminfo;

create view
  meminfo as
select
  ts,
  EXTRACT_ARG (arg_set_id, 'debug.data.detail') AS json
from
  slice
where
  category = 'blink.user_timing'
  and name = 'crossbench-meminfo';

drop view if exists meminfo_with_iteration;

create view
  meminfo_with_iteration as
select
  iterations.id as it_id,
  ts,
  json
from
  iterations
  join meminfo on meminfo.ts >= iterations.start
  and meminfo.ts <= iterations.end;

DROP TABLE IF EXISTS meminfo_total_output;

CREATE PERFETTO TABLE meminfo_total_output AS
SELECT
  -- crossbench-meminfo event's detail field is a JSON blob of type:
  --  {
  --    title: 'title'
  --    meminfos: [
  --      {
  --        pid: number
  --        pss_total: number
  --        rss_total: number
  --        swap_total: number
  --      },
  --      ...
  --    ]
  --  }
  -- We have a row per process per meminfo event, sum up the meminfo counters
  -- for each meminfo event.
  it_id,
  ts,
  title,
  SUM(pss_total_kb) / 1024.0 AS pss_total_mb,
  SUM(rss_total_kb) / 1024.0 AS rss_total_mb,
  SUM(swap_total_kb) / 1024.0 AS swap_total_mb
FROM
  (
    -- Extract the per-process objects and join them to their meminfo rows, to get
    -- a row per process per meminfo event.
    SELECT
      meminfo_with_iteration.it_id AS it_id,
      meminfo_with_iteration.ts AS ts,
      json_extract (meminfo_with_iteration.json, '$.title') AS title,
      CAST(
        json_extract (per_process_meminfo_json.value, '$.pss_total') AS FLOAT
      ) AS pss_total_kb,
      CAST(
        json_extract (per_process_meminfo_json.value, '$.rss_total') AS FLOAT
      ) AS rss_total_kb,
      CAST(
        json_extract (per_process_meminfo_json.value, '$.swap_total') AS FLOAT
      ) AS swap_total_kb
    FROM
      meminfo_with_iteration,
      json_each (
        json_extract (meminfo_with_iteration.json, '$.meminfos')
      ) AS per_process_meminfo_json
  )
GROUP BY
  it_id,
  ts,
  title;
