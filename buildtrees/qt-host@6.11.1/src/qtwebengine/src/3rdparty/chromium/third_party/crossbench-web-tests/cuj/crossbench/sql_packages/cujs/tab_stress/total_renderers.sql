-- Copyright 2025 The Chromium Authors
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.
include PERFETTO MODULE sql_packages.web_tests_common.iterations;

drop view if exists renderer_created;

create view
  renderer_created as
select
  extract_arg (ftrace_event.arg_set_id, 'pid') as forked_pid,
  process.cmdline as process_cmdline,
  ts
from
  ftrace_event
  join process on forked_pid = process.pid
where
  ftrace_event.name = 'task_newtask'
  and (
    -- Android Chrome process cmd format
    process.cmdline glob '*org.chromium.content.app.SandboxedProcessService*'
    -- ChromeOS Chrome process cmd format
    or process.cmdline glob '/opt/google/chrome/chrome --type=renderer*'
  );

drop view if exists renderers_by_iteration;

create view
  renderers_by_iteration as
select
  iterations.id as it_id,
  process_cmdline
from
  iterations
  join renderer_created on renderer_created.ts >= iterations.start
  and renderer_created.ts <= iterations.end;

DROP TABLE IF EXISTS total_renderers_output;
CREATE PERFETTO TABLE total_renderers_output AS
select
  it_id,
  count(*) as total_renderers
from
  renderers_by_iteration
group by
  it_id
