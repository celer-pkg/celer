-- Copyright 2025 The Chromium Authors
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.
include PERFETTO MODULE sql_packages.web_tests_common.iterations;

drop view if exists lmk_kill_occurred;

create view
  lmk_kill_occurred as
select
  ts,
  extract_arg (arg_set_id, 'lmk_kill_occurred.oom_adj_score') as oom_adj_score,
  extract_arg (arg_set_id, 'lmk_kill_occurred.min_oom_score') as min_oom_score,
  extract_arg (arg_set_id, 'lmk_kill_occurred.page_fault') as page_fault,
  extract_arg (arg_set_id, 'lmk_kill_occurred.page_major_fault') as page_major_fault,
  extract_arg (arg_set_id, 'lmk_kill_occurred.rss_in_bytes') as rss_in_bytes,
  extract_arg (arg_set_id, 'lmk_kill_occurred.cache_in_bytes') as cache_in_bytes,
  extract_arg (arg_set_id, 'lmk_kill_occurred.swap_in_bytes') as swap_in_bytes,
  extract_arg (arg_set_id, 'lmk_kill_occurred.free_mem_kb') as free_mem_kb,
  extract_arg (arg_set_id, 'lmk_kill_occurred.free_swap_kb') as free_swap_kb,
  extract_arg (arg_set_id, 'lmk_kill_occurred.reason') as reason,
  extract_arg (arg_set_id, 'lmk_kill_occurred.thrashing') as thrashing,
  extract_arg (arg_set_id, 'lmk_kill_occurred.max_thrashing') as max_thrashing,
  extract_arg (
    arg_set_id,
    'lmk_kill_occurred.total_foreground_services'
  ) as total_foreground_services,
  extract_arg (
    arg_set_id,
    'lmk_kill_occurred.procs_with_foreground_services'
  ) as procs_with_foreground_services,
  extract_arg (arg_set_id, 'lmk_kill_occurred.process_name') as process_name
from
  slice
where
  name = 'lmk_kill_occurred';

drop view if exists lmk_kill_list_output;
create view
  lmk_kill_list_output as
select
  iterations.id as it_id,
  lmk_kill_occurred.*
from
  iterations
  join lmk_kill_occurred on lmk_kill_occurred.ts >= iterations.start
  and lmk_kill_occurred.ts <= iterations.end
